#include "stdafx.h"
#include "TCharacterInventory.h"

TCharacterInventory Inventory;
//------------------------------------------------------------

TCharacterInventory::TCharacterInventory(void)
{
}

//------------------------------------------------------------

TCharacterInventory::~TCharacterInventory(void)
{
}

//------------------------------------------------------------

void TCharacterInventory::ParseInventory( TSocket *Client, int characterslot, int inventorytype, std::string inventoryarray )
{
	std::vector<std::string> Items = General.split(inventoryarray, ' ');
	std::vector<std::string> Index;
	int itemssize = Items.size();

	//Inventar Items auf 0 setzen
	if(Client->CharacterList[characterslot].Inventory.Initialized != true)
	{
		//Size Allozieren
		Client->CharacterList[characterslot].Inventory.Index.resize(INVENTORY_SIZE);
		Client->CharacterList[characterslot].Inventory.Index[EQUIP_INVENTORY].resize(MAX_SLOTS_BACKPACK);
		Client->CharacterList[characterslot].Inventory.Index[MAIN_INVENTORY].resize(MAX_SLOTS_BACKPACK);
		Client->CharacterList[characterslot].Inventory.Index[OTHER_INVENTORY].resize(MAX_SLOTS_BACKPACK);

		for(auto i = 0; i < INVENTORY_SIZE; i++)
		{
			for(auto y = 0; y < MAX_SLOTS_BACKPACK; y++)
			{
				Client->CharacterList[characterslot].Inventory.Index[i][y].inUse = false;
				Client->CharacterList[characterslot].Inventory.Index[i][y].inUse = false;
				Client->CharacterList[characterslot].Inventory.Index[i][y].count = -1;
				Client->CharacterList[characterslot].Inventory.Index[i][y].id = -1;
				Client->CharacterList[characterslot].Inventory.Index[i][y].rare = -1;
				Client->CharacterList[characterslot].Inventory.Index[i][y].upgrade = -1;
			}
		}

		Client->CharacterList[characterslot].Inventory.Initialized = true;
	}

	//Alle Items durchlaufen & die Informationen splitten (Slot etc)
	for(auto i = 0; i < itemssize; i++)
	{
		Index = General.split(Items[i], '.');

		//Items im Inventar hinzufügen
		switch (inventorytype)
		{
		case EQUIP_INVENTORY:
			this->SetItemFromDatabase(Client, characterslot, inventorytype, General.s_atoi(Index[0]), General.s_atoi(Index[1]), 1, General.s_atoi(Index[2]), General.s_atoi(Index[3]));
			break;
		case MAIN_INVENTORY:
			this->SetItemFromDatabase(Client, characterslot, inventorytype, General.s_atoi(Index[0]), General.s_atoi(Index[1]), General.s_atoi(Index[2]), NULL, NULL);
			break;
		case OTHER_INVENTORY:
			this->SetItemFromDatabase(Client, characterslot, inventorytype, General.s_atoi(Index[0]), General.s_atoi(Index[1]), General.s_atoi(Index[2]), NULL, NULL);
			break;
		};
	}
}
//------------------------------------------------------------

int TCharacterInventory::DetectStackSlot(TSocket *Client, int inventorytype, int item_vnum, int amount)
{
	//Slot suchen und returnen
	for(auto i = 0; i < MAX_SLOTS_BACKPACK; i++)
	{
		if(Client->Character->Inventory.Index[inventorytype][i].inUse == true)
		{
			if(Client->Character->Inventory.Index[inventorytype][i].id == item_vnum)
			{
				if((Client->Character->Inventory.Index[inventorytype][i].count + amount) <= 99)
				{
					return i;
				}
			}
		}
	}	

	return NO_ITEM_STACK;
}

//------------------------------------------------------------

bool TCharacterInventory::AddItem( TSocket *Client, int inventorytype, int slot, int id, int count, int rare, int upgrade, bool ismoving, structItemStats stats )
{
	std::stringstream message;
	int tempslot;

	if(count <= 0)
	{
		return false;
	}
	
	inventorytype = this->DetectInventory(Client, inventorytype, id);

	if(inventorytype == UNKNOWN_INVENTORY)
	{
		return false;
	}


	if(inventorytype == GOLD_INVENTORY)
	{
		Client->Character->Inventory.gold_count += count;
	}else
	{
		if(inventorytype == MAIN_INVENTORY)
		{
			tempslot = this->DetectStackSlot(Client, inventorytype, id, count);
		}

		if(inventorytype == MAIN_INVENTORY && tempslot != NO_ITEM_STACK && !ismoving)
		{
			//Item zu stack adden
			slot = tempslot;
			Client->Character->Inventory.Index[inventorytype][slot].count += count;

		}else
		{
			//Neuer Slot
			slot = this->DetectSlot(Client, inventorytype, slot);

			if(slot == SLOTS_FULL)
			{
				return false;
			}

			Client->Character->Inventory.Index[inventorytype][slot].inUse = true;
			Client->Character->Inventory.Index[inventorytype][slot].count = count;
			Client->Character->Inventory.Index[inventorytype][slot].id = id;
			Client->Character->Inventory.Index[inventorytype][slot].rare = rare;
			Client->Character->Inventory.Index[inventorytype][slot].upgrade = upgrade;
			Client->Character->Inventory.Index[inventorytype][slot].stats = stats;
		}
	}

	switch (inventorytype)
	{
	case EQUIP_INVENTORY:	
		message << "ivn " << inventorytype << " " << slot << "." << id << "." << rare << "." << upgrade;
		break;
	case MAIN_INVENTORY:
		message << "ivn " << inventorytype << " " << slot << "." << id << "." << Client->Character->Inventory.Index[inventorytype][slot].count;
		break;
	case OTHER_INVENTORY:
		message << "ivn " << inventorytype << " " << slot << "." << id << "." << count;
		break;
	case GOLD_INVENTORY:
		message << "gold " << Client->Character->Inventory.gold_count << " 0";
		break;
	};

	//Packet senden zum Item hinzufügen
	Client->Send(message.str());

	return true;
}

//------------------------------------------------------------

void TCharacterInventory::SetItemFromDatabase( TSocket *Client, int characterslot, int inventorytype, int slot, int id, int count, int rare, int upgrade )
{
	//Inventar Item übernehmen
	Client->CharacterList[characterslot].Inventory.Index[inventorytype][slot].inUse = true;
	Client->CharacterList[characterslot].Inventory.Index[inventorytype][slot].count = count;
	Client->CharacterList[characterslot].Inventory.Index[inventorytype][slot].id = id;
	Client->CharacterList[characterslot].Inventory.Index[inventorytype][slot].rare = rare;
	Client->CharacterList[characterslot].Inventory.Index[inventorytype][slot].upgrade = upgrade;
}

//------------------------------------------------------------

void TCharacterInventory::RemoveItem( TSocket *Client, int inventorytype, int slot, int amount)
{
	std::stringstream message;
	int allowed_slot;

	if(Client->Character->Inventory.BackpackActive)
	{
		allowed_slot = MAX_SLOTS_BACKPACK;
	} else
	{
		allowed_slot = MAX_SLOTS_SINGLE;
	}

	//Prüfen ob Slots & Inventartype im richtigen Bereich
	if(slot < 0 || slot > allowed_slot || inventorytype < 0 || inventorytype > 2 || Client->Character->Inventory.Index[inventorytype][slot].inUse == false)
	{
		return;
	}

	bool removed = true;
	//Item in der Inventar Struktur vom Clienten entfernen
	if(amount == -1) //Default - Item komplett löschen
	{
		Client->Character->Inventory.Index[inventorytype][slot].inUse = false;
		Client->Character->Inventory.Index[inventorytype][slot].count = -1;
		Client->Character->Inventory.Index[inventorytype][slot].id = -1;
		Client->Character->Inventory.Index[inventorytype][slot].rare = -1;
		Client->Character->Inventory.Index[inventorytype][slot].upgrade = -1;
	}else
	{
		if((Client->Character->Inventory.Index[inventorytype][slot].count - amount) > 0) //Anzahl reduzieren
		{
			Client->Character->Inventory.Index[inventorytype][slot].count -= amount;
			removed = false;
		}else //Anzahl = 0 -> komplett löschen
		{
			Client->Character->Inventory.Index[inventorytype][slot].inUse = false;
			Client->Character->Inventory.Index[inventorytype][slot].count = -1;
			Client->Character->Inventory.Index[inventorytype][slot].id = -1;
			Client->Character->Inventory.Index[inventorytype][slot].rare = -1;
			Client->Character->Inventory.Index[inventorytype][slot].upgrade = -1;
		}
	}

	//Item entfernen Packet senden
	if(removed)
	{
		message << "ivn " << inventorytype << " " << slot << ".-1.0";
		Client->Send(message.str());
	}else //Nur geändert
	{
		switch (inventorytype)
		{
		case MAIN_INVENTORY:
			message << "ivn " << inventorytype << " " << slot << "." << Client->Character->Inventory.Index[inventorytype][slot].id<< "." << Client->Character->Inventory.Index[inventorytype][slot].count;
			break;
		case OTHER_INVENTORY:
			message << "ivn " << inventorytype << " " << slot << "." << Client->Character->Inventory.Index[inventorytype][slot].id << "." << Client->Character->Inventory.Index[inventorytype][slot].count;
			break;
		};

		Client->Send(message.str());
	}
}

//------------------------------------------------------------

void TCharacterInventory::RemoveItemRequest( TSocket *Client, int inventorytype, int slot )
{
	std::stringstream accept;
	accept << "#b_i^" << inventorytype << "^" << slot << "^1";
	Message.Dialog(Client, accept.str(), "#b_i^Cancel", "Do you really want to delete this Item?");
}

//------------------------------------------------------------

void TCharacterInventory::MoveItem( TSocket *Client, int inventorytype, int slot, int count, int newslot )
{
	int allowed_slot;
	int difference;
	structInventory TempItem = {Client->Character->Inventory.Index[inventorytype][slot].inUse, Client->Character->Inventory.Index[inventorytype][slot].id, Client->Character->Inventory.Index[inventorytype][slot].count, Client->Character->Inventory.Index[inventorytype][slot].rare, Client->Character->Inventory.Index[inventorytype][slot].upgrade};
	structInventory TempSecondItem = {Client->Character->Inventory.Index[inventorytype][newslot].inUse, Client->Character->Inventory.Index[inventorytype][newslot].id, Client->Character->Inventory.Index[inventorytype][newslot].count, Client->Character->Inventory.Index[inventorytype][newslot].rare, Client->Character->Inventory.Index[inventorytype][newslot].upgrade};

	//Überprüfen welche Slots erlaubt sind
	if(Client->Character->Inventory.BackpackActive)
	{
		allowed_slot = MAX_SLOTS_BACKPACK;
	} else
	{
		allowed_slot = MAX_SLOTS_SINGLE;
	}

	//Prüfen ob Gültiger Slot, Inventartype etc.
	if(inventorytype < 0 || inventorytype > 2 || slot < 0 || slot > allowed_slot || 
		count > Client->Character->Inventory.Index[inventorytype][slot].count || count < 1 || 
		count > 99 || newslot < 0 || newslot > allowed_slot || 
		Client->Character->Inventory.Index[inventorytype][slot].inUse == false)
	{
		return;
	}

	//Item Moven auf leeren Slot
	if(TempSecondItem.inUse == false)
	{
		//Wenn weniger Items gemoved werden, als vorhanden, nur bestimmte Anzahl moven
		if(TempItem.count > count)
		{
			Client->Character->Inventory.Index[inventorytype][slot].count -= count;
			this->AddItem(Client, inventorytype, slot, TempItem.id, Client->Character->Inventory.Index[inventorytype][slot].count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
			this->AddItem(Client, inventorytype, newslot, TempItem.id, count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
		} else
		{
			this->RemoveItem(Client, inventorytype, slot);
			this->AddItem(Client, inventorytype, newslot, TempItem.id, TempItem.count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
		}
	} else
	{
		//Wenn  auf dem anderen Slot, das gleiche Item, Item addieren
		if(TempItem.id == TempSecondItem.id)
		{
			difference = (TempItem.count + TempSecondItem.count);

			//Wenn die Differenz größer, Items abziehen und Moven, ansonsten direkt addieren
			if(difference > 99)
			{
				difference = TempSecondItem.count - 99;
				Client->Character->Inventory.Index[inventorytype][newslot].count += difference;
				Client->Character->Inventory.Index[inventorytype][slot].count -= difference;
				this->AddItem(Client, inventorytype, slot, TempItem.id, Client->Character->Inventory.Index[inventorytype][slot].count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
				this->AddItem(Client, inventorytype, newslot, TempSecondItem.id, Client->Character->Inventory.Index[inventorytype][newslot].count, TempSecondItem.rare, TempSecondItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
			} else
			{
				Client->Character->Inventory.Index[inventorytype][newslot].count += count;
				Client->Character->Inventory.Index[inventorytype][slot].count -= count;

				//Wenn beim Moven keine Items verfügbar entfernen, ansonsten rest dalassen
				if(Client->Character->Inventory.Index[inventorytype][slot].count <= 0)
				{
					RemoveItem(Client, inventorytype, slot);
				} else
				{
					this->AddItem(Client, inventorytype, slot, TempItem.id, Client->Character->Inventory.Index[inventorytype][slot].count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
				}

				this->AddItem(Client, inventorytype, newslot, TempSecondItem.id, Client->Character->Inventory.Index[inventorytype][newslot].count, TempSecondItem.rare, TempSecondItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
			}

		} else //Ansonsten Item einfach tauschen
		{
			this->RemoveItem(Client, inventorytype, slot);
			this->RemoveItem(Client, inventorytype, newslot);
			this->AddItem(Client, inventorytype, newslot, TempItem.id, TempItem.count, TempItem.rare, TempItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
			this->AddItem(Client, inventorytype, slot, TempSecondItem.id, TempSecondItem.count, TempSecondItem.rare, TempSecondItem.upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
		}
	}
}

//------------------------------------------------------------

int TCharacterInventory::DetectSlot( TSocket *Client, int inventorytype, int slot )
{
	//Wenn nicht auf AUTODETECT_SLOT gesetzt, dann manuell slot besetzen
	if(slot != AUTODETECT_SLOT)
	{
		return slot;
	} else
	{
		//Überprüfen welche Slots erlaubt sind
		int allowed_slot;
		if(Client->Character->Inventory.BackpackActive)
		{
			allowed_slot = MAX_SLOTS_BACKPACK;
		} else
		{
			allowed_slot = MAX_SLOTS_SINGLE;
		}

		//Freien Slot suchen und returnen
		for(auto i = 0; i < allowed_slot; i++)
		{
			if(Client->Character->Inventory.Index[inventorytype][i].inUse == false)
			{
				return i;
			}
		}

		return SLOTS_FULL;
	}
}

//------------------------------------------------------------

int TCharacterInventory::DetectInventory( TSocket *Client, int inventorytype, int itemid )
{
	if(inventorytype != AUTODETECT_INVENTORY)
	{
		return inventorytype;
	} else
	{
		int itemssize = Items.vItems.size();

		//Alle Items ablaufen
		for(auto i = 0; i < itemssize; i++)
		{
			//Wenn Item gefunden, Inventartype returnen
			if(Items.vItems[i].vnum == itemid)
			{
				return Items.vItems[i].inventorytype;
			}
		}

		return UNKNOWN_INVENTORY;
	}
}

//------------------------------------------------------------

void TCharacterInventory::UseItem( TSocket *Client, int inventorytype, int slot )
{
	int used;

	//Prüfen ob Slot überhaupt belegt
	if(Client->Character->Inventory.Index[inventorytype][slot].inUse == true)
	{
		//Item nutzen
		used = Items.MakeItem(Client, Client->Character->Inventory.Index[inventorytype][slot].id);

		//Wenn das Item in der Liste nicht gefunden, returnen
		if(used == ITEM_NOT_FOUND)
		{
			return;
		}

		//Wenn Item gelöscht wird, nachdem benutzen
		if(used)
		{
			//Anzahl um 1 reduzieren
			Client->Character->Inventory.Index[inventorytype][slot].count--;

			//Wenn keine Items mehr vorhanden (Count <= 0) Item löschen, ansonsten reduzierung anzeigen
			if(Client->Character->Inventory.Index[inventorytype][slot].count <= 0)
			{
				this->RemoveItem(Client, inventorytype, slot);
			} else
			{
				this->AddItem(Client, inventorytype, slot, Client->Character->Inventory.Index[inventorytype][slot].id, Client->Character->Inventory.Index[inventorytype][slot].count, Client->Character->Inventory.Index[inventorytype][slot].rare, Client->Character->Inventory.Index[inventorytype][slot].upgrade, true, Client->Character->Inventory.Index[inventorytype][slot].stats);
			}
		}  
	}
}

//------------------------------------------------------------
void TCharacterInventory::Send_Equipment( TSocket *Client )
{
	std::stringstream message;
	//eq 558097 0 <gender> <hairstyle> <haircolor> <class> -1.12.1.8.-1.-1.-1.-1 0 0
	message << "eq " << Client->Character->CharacterInfo.ID << " "
		<< "0" << " "
		<< Client->Character->CharacterInfo.Gender << " "
		<< Client->Character->CharacterInfo.Hairstyle << " "
		<< Client->Character->CharacterInfo.Haircolor << " "
		<< "0" << " "  //Class
		<< Client->Character->Equipment[EQ_SLOT_HEAD].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_CHEST].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_WEAPON1].vnum << "."
		<< "-1" << "."
		<< Client->Character->Equipment[EQ_SLOT_SPECS].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_KOSTUEM_CHEST].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_KOSTUEM_HEAD].vnum << "."
		<< "-1" << " "
		<< "0 0";
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	Map.SendMap(MapIndex, Client->Character->CharacterInfo.ID, false, message.str());
}

//------------------------------------------------------------

void TCharacterInventory::WearItem( TSocket *Client, int slot, int inventorytype )
{
	int used;

	if(inventorytype == EQUIP_INVENTORY && slot < 15 && slot >= 0)
	{
		//Prüfen ob Inventar Slot überhaupt belegt
		if(Client->Character->Inventory.Index[inventorytype][slot].inUse == true)
		{
			int eq_slot = Items.GetEquipmentSlot(Client->Character->Inventory.Index[inventorytype][slot].id);

			if(eq_slot != ITEM_NOT_FOUND)
			{
				if(Client->Character->Equipment[eq_slot].vnum == EQUIP_NOITEM)
				{
					//EQ Slot ist leer
					Client->Character->Equipment[eq_slot].vnum = Client->Character->Inventory.Index[inventorytype][slot].id;
					Client->Character->Equipment[eq_slot].rare = Client->Character->Inventory.Index[inventorytype][slot].rare;
					Client->Character->Equipment[eq_slot].upgrade = Client->Character->Inventory.Index[inventorytype][slot].upgrade;
					Client->Character->Equipment[eq_slot].stats = Client->Character->Inventory.Index[inventorytype][slot].stats;
					this->RemoveItem(Client, inventorytype, slot);

				}else
				{
					//EQ Slot beinhaltet Item
					//Altes in den Inventar packen
					structInventory tempItem;
					tempItem.id = Client->Character->Equipment[eq_slot].vnum;
					tempItem.rare = Client->Character->Equipment[eq_slot].rare;
					tempItem.upgrade = Client->Character->Equipment[eq_slot].upgrade;
					tempItem.stats = Client->Character->Equipment[eq_slot].stats;

					//Neues anlegen
					Client->Character->Equipment[eq_slot].vnum = Client->Character->Inventory.Index[inventorytype][slot].id;
					Client->Character->Equipment[eq_slot].rare = Client->Character->Inventory.Index[inventorytype][slot].rare;
					Client->Character->Equipment[eq_slot].upgrade = Client->Character->Inventory.Index[inventorytype][slot].upgrade;
					Client->Character->Equipment[eq_slot].stats = Client->Character->Inventory.Index[inventorytype][slot].stats;
					this->RemoveItem(Client, inventorytype, slot);

					//ablegen
					this->AddItem(Client, 0, -1, tempItem.id, 1, tempItem.rare, tempItem.upgrade, true, tempItem.stats);
				}

				CharacterInfo.Stats_CalcReal(Client);
				CharacterInfo.Send_CharacterStatPoints(Client);
				CharacterInfo.Send_Equipment(Client);
				this->Send_Equipment(Client);
			}
		}
	}
}

//------------------------------------------------------------

void TCharacterInventory::UnWearItem( TSocket *Client, int slot, int unkown )
{
	int used;

	if(slot < 15 && slot >= 0)
	{
		//Prüfen ob Inventar Slot überhaupt belegt
		if(Client->Character->Equipment[slot].vnum != EQUIP_NOITEM)
		{
			//EQ Slot beinhaltet Item
			//Altes in den Inventar packen
			structInventory tempItem;
			tempItem.id = Client->Character->Equipment[slot].vnum;
			tempItem.rare = Client->Character->Equipment[slot].rare;
			tempItem.upgrade = Client->Character->Equipment[slot].upgrade;

			//ablegen
			this->AddItem(Client, -1, -1, tempItem.id, 1, tempItem.rare, tempItem.upgrade, true, Client->Character->Equipment[slot].stats);

			//Neues anlegen
			Client->Character->Equipment[slot].vnum = EQUIP_NOITEM;
			Client->Character->Equipment[slot].rare = 0;
			Client->Character->Equipment[slot].upgrade = 0;

			CharacterInfo.Stats_CalcReal(Client);
			CharacterInfo.Send_CharacterStatPoints(Client);
			CharacterInfo.Send_Equipment(Client);
			this->Send_Equipment(Client);
		}
	}
}

//------------------------------------------------------------

void TCharacterInventory::DropItem(TSocket *Client, int inventory, int slot, int amount)
{
	if(Client->Character->Inventory.Index[inventory][slot].inUse)
	{
		if(Client->Character->Inventory.Index[inventory][slot].count >= amount)
		{
			//drop 2024 441127 8 7 10 0 0
			//drop <modalid> <objectid> <x> <y> <count> <quest> <bound>
			int vnum = Client->Character->Inventory.Index[inventory][slot].id;
			int x = General.CreateRandomNumber(Client->Character->Map.x - 1, Client->Character->Map.x + 1);
			int y = General.CreateRandomNumber(Client->Character->Map.y - 1, Client->Character->Map.y + 1);
			int count = amount;
			int quest = 0;
			int bound = 0;
			int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
			int itemid = Items.s_ItID(MapIndex);
			int rare = Client->Character->Inventory.Index[inventory][slot].rare;
			int upgrade = Client->Character->Inventory.Index[inventory][slot].upgrade;

			structMapItem MapItem = {itemid, vnum, count, x, y, quest, bound, rare, upgrade};
			MapItem.enemy_drop = false;
			MapItem.dropbound.drop_bound_is = false;
			MapItem.dropbound.drop_bound_fadetime = (GetTickCount() / 1000) + ITEM_DROPBOUND_TIME;
			MapItem.dropbound.lifetime = (GetTickCount() / 1000) + ITEM_LIFE_TIME;
			MapItem.stats = Client->Character->Inventory.Index[inventory][slot].stats;

			Items.lockthread.lock();
			Map.MapData[MapIndex].Item.push_back(MapItem);
			Items.lockthread.unlock();

			//Item entfernen
			Inventory.RemoveItem(Client, inventory, slot, amount);

			std::stringstream message;

			message << "drop " << vnum << " " << itemid << " " << x << " " << y << " " << count << " " << quest << " " << bound;
			
			Map.SendMap(MapIndex, NULL, true, message.str());
		}
	}
}

//------------------------------------------------------------

void TCharacterInventory::GetItem(TSocket *Client, int action, int charid, int itemid)
{
	if(action == 1) // Andere Actions?
	{
		if(Client->Character->CharacterInfo.ID == charid)
		{
			std::stringstream message;
			Items.lockthread.lock();
			int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
			int MapItemIndex = Map.GetMapItemIndex(Client->Character->Map.id, MapIndex, itemid);
			
			if(MapItemIndex != ITEM_NOT_FOUND)
			{
				//If(Item gehört nicht noch einem anderen Char (Drop)) {}
				int distance = Math.Distance(Client->Character->Map.x, Map.MapData[MapIndex].Item[MapItemIndex].x, Client->Character->Map.y, Map.MapData[MapIndex].Item[MapItemIndex].y);
				if(distance <= 3)
				{
					if(!Map.MapData[MapIndex].Item[MapItemIndex].dropbound.drop_bound_is || (Map.MapData[MapIndex].Item[MapItemIndex].dropbound.drop_bound_is && Map.MapData[MapIndex].Item[MapItemIndex].dropbound.drop_bound_charid == Client->Character->CharacterInfo.ID))
					{
						bool ispickedup = Inventory.AddItem(Client, -1, -1, Map.MapData[MapIndex].Item[MapItemIndex].vnum, Map.MapData[MapIndex].Item[MapItemIndex].amount, Map.MapData[MapIndex].Item[MapItemIndex].rare, Map.MapData[MapIndex].Item[MapItemIndex].upgrade, false, Map.MapData[MapIndex].Item[MapItemIndex].stats);

						/*
						say 1 825479 12 Du hast ein Item erhalten:Kekse x 10
						icon 1 825479 1 2024
						get 1 825479 441259 0				
						*/
						if(ispickedup)
						{
							int itemindex = Items.GetItemIndex(Map.MapData[MapIndex].Item[MapItemIndex].vnum);

							message << "say 1 " << Client->Character->CharacterInfo.ID << " " << ITEM_CHAT << " You got an item: " << Items.vItems[itemindex].name << " x " << Map.MapData[MapIndex].Item[MapItemIndex].amount;
							Client->Send(message.str());
							message.str("");

							message << "icon 1 " << Client->Character->CharacterInfo.ID << " 1 " << Map.MapData[MapIndex].Item[MapItemIndex].vnum;
							Map.SendMap(MapIndex, NULL, true, message.str());
							message.str("");

							message << "get 1 " << Client->Character->CharacterInfo.ID << " " << itemid << " 0";

							Map.SendMap(MapIndex, NULL, true, message.str());
							Map.MapData[MapIndex].Item.erase(Map.MapData[MapIndex].Item.begin()+MapItemIndex);
						}else //Inventar voll
						{
							std::stringstream message;
							message << "say 1 " << Client->Character->CharacterInfo.ID << " " << SYSTEM_CHAT << " Your inventory is full.";
							Client->Send(message.str());
						}
					}else
					{
						std::stringstream message;
						message << "say 1 " << Client->Character->CharacterInfo.ID << " " << SYSTEM_CHAT << " Item is bound to another player.";
						Client->Send(message.str());
					}
				}
			}
			Items.lockthread.unlock();

		}
	}
}

//------------------------------------------------------------