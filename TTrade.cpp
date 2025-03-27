#include "stdafx.h"
#include "TTrade.h"

//Typedefinitionen
TTrade Trade;
//------------------------------------------------------------

TTrade::TTrade(void)
{
}

//------------------------------------------------------------

TTrade::~TTrade(void)
{
}
//------------------------------------------------------------
void TTrade::Execute_Handle(TSocket *Client, std::vector<std::string> packet)
{
	//<gold> <inv> <slot> <amount> <inv> <slot> <amount>
	//sned: exc_list 0 2 0 10 2 1 1
	//<action> <id> <gold> <slot>.<kp>.<id>.<amount>.<kp>
	//recv: exc_list 1 825728 0 0.2.2024.10.0 1.2.2081.1.0
	//Schauen ob trade aktiv

	if(Client->Character->trade_active)
	{
		std::stringstream message;
		structMapData currentMap;
		int mapsize = Map.MapData.size();

		for(auto i = 0; i < mapsize; i++)
		{
			//Wenn Map gefunden zuweisen
			if(Client->Character->Map.id == Map.MapData[i].id)
			{
				currentMap = Map.MapData[i];
				break;
			}
		}

		int trade_id = Client->Character->active_trade_id;

		//Alle Spieler auf der Map ablaufen
		int size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == trade_id)
			{
				//Schauen ob Request vorhanden
				bool request_allready = this->Request_Check(Client, currentMap.Players[e]->Character->CharacterInfo.ID);

				if(request_allready)
				{
					int size_item = (packet.size() - 3);

					if(size_item %3 == 0)
					{
						int itemcount = (packet.size() - 3) / 3;
						int gold = General.s_atoi(packet[2]);

						//Schauen ob der Char soviel Gold hat
						if(gold <= Client->Character->Inventory.gold_count)
						{
							//exc_list 1 825728 0 0.2.2024.10.0 1.2.2081.1.0
							message << "exc_list 1 " << Client->Character->CharacterInfo.ID << " " << gold;

							Client->Character->Inventory.TradeInventory.clear();

							int nextitem = 3;
							std::vector<std::string> TradeItem;

							for(int i = 0; i < itemcount; i++)
							{
								//Checken ob Item in "Ordnung" ist
								//Item in ein Array schreiben(?)
								//message << " " << count << "." << ? << "." << ItemID << "." << amount << "." << ?;
								//TradeItem = General.split(packet[nextitem], '.');
								int inv = General.s_atoi(packet[nextitem]);
								int slot = General.s_atoi(packet[nextitem + 1]);
								int amount = General.s_atoi(packet[nextitem + 2]);

								//ItemCheck
								if(!Client->Character->Inventory.Index[inv][slot].inUse || Client->Character->Inventory.Index[inv][slot].count < amount)
								{
									//Cheat Versuch
									this->Trade_Cancel(Client);
									Client->Disconnect();
								}

								structTradeInventory TradeItems = {inv, slot, amount};
								nextitem = nextitem + 3;

								Client->Character->Inventory.TradeInventory.push_back(TradeItems);

								int ItemID = Client->Character->Inventory.Index[inv][slot].id;
								message << " " << (i) << "." << "2" << "." << ItemID << "." << amount << "." << "0";
							}

							Client->Character->Inventory.gold_trade = gold;

							//Liste schicken
							currentMap.Players[e]->Send(message.str());
							Client->Character->Inventory.trade_ready = true;
						}else
						{
							//Cheat Versuch
							this->Trade_Cancel(Client);
							Client->Disconnect();
						}
					}else
					{
						//Cheat Versuch
						this->Trade_Cancel(Client);
						Client->Disconnect();
					}
				}
				break;
			}
		}
	}
}
//------------------------------------------------------------
void TTrade::Trade_Finish(TSocket *Client)
{
	if(Client->Character->trade_active)
	{
		std::stringstream message;
		structMapData currentMap;
		int mapsize = Map.MapData.size();

		for(auto i = 0; i < mapsize; i++)
		{
			//Wenn Map gefunden zuweisen
			if(Client->Character->Map.id == Map.MapData[i].id)
			{
				currentMap = Map.MapData[i];
				break;
			}
		}

		int trade_id = Client->Character->active_trade_id;

		//Alle Spieler auf der Map ablaufen
		int size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == trade_id)
			{
				//Schauen ob Request vorhanden
				bool request_allready = this->Request_Check(Client, currentMap.Players[e]->Character->CharacterInfo.ID);

				if(request_allready)
				{
					if(Client->Character->Inventory.trade_ready && currentMap.Players[e]->Character->Inventory.trade_ready)
					{
						Client->Character->Inventory.trade_finish = true;
						if(currentMap.Players[e]->Character->Inventory.trade_finish)
						{
							//----------------------- GOLD
							//Delete Client Gold
							Client->Character->Inventory.gold_count = Client->Character->Inventory.gold_count - Client->Character->Inventory.gold_trade;
							//Delete Partner Gold
							currentMap.Players[e]->Character->Inventory.gold_count = currentMap.Players[e]->Character->Inventory.gold_count - currentMap.Players[e]->Character->Inventory.gold_trade;

							//Gold buchen
							Client->Character->Inventory.gold_count = Client->Character->Inventory.gold_count + currentMap.Players[e]->Character->Inventory.gold_trade;
							currentMap.Players[e]->Character->Inventory.gold_count = currentMap.Players[e]->Character->Inventory.gold_count + Client->Character->Inventory.gold_trade;

							//Neuer Gold betrag senden
							std::stringstream message;
							message << "gold " << Client->Character->Inventory.gold_count;
							Client->Send(message.str());

							message.str("");
							message << "gold " << currentMap.Players[e]->Character->Inventory.gold_count;
							currentMap.Players[e]->Send(message.str());
							//----------------------- ITEMS
							//Char #1
							int TradeInvSize = Client->Character->Inventory.TradeInventory.size();
							for(int t = 0; t < TradeInvSize; t++)
							{
								int ItemSlot = Client->Character->Inventory.TradeInventory[t].slot;
								int ItemInv = Client->Character->Inventory.TradeInventory[t].inv;
								int ItemAmount = Client->Character->Inventory.TradeInventory[t].amount;
								int ItemID = Client->Character->Inventory.Index[ItemInv][ItemSlot].id;
								structItemStats stats = Client->Character->Inventory.Index[ItemInv][ItemSlot].stats;
								Inventory.RemoveItem(Client, ItemInv, ItemSlot, ItemAmount);
								//Inventory.AddItem(currentMap.Players[e], -1, -1, ItemID, ItemAmount, 0,0);
								Inventory.AddItem(currentMap.Players[e], ItemInv, -1, ItemID, ItemAmount, 0,0, false, stats);
							}
							//Char #2
							TradeInvSize = currentMap.Players[e]->Character->Inventory.TradeInventory.size();
							for(int x = 0; x < TradeInvSize; x++)
							{
								int ItemSlot = currentMap.Players[e]->Character->Inventory.TradeInventory[x].slot;
								int ItemInv = currentMap.Players[e]->Character->Inventory.TradeInventory[x].inv;
								int ItemAmount = currentMap.Players[e]->Character->Inventory.TradeInventory[x].amount;
								int ItemID = currentMap.Players[e]->Character->Inventory.Index[ItemInv][ItemSlot].id;
								structItemStats stats = currentMap.Players[e]->Character->Inventory.Index[ItemInv][ItemSlot].stats;

								Inventory.RemoveItem(currentMap.Players[e], ItemInv, ItemSlot, ItemAmount);
								//Inventory.AddItem(Client, -1, -1, ItemID, ItemAmount, 0,0);	
								Inventory.AddItem(Client, ItemInv, -1, ItemID, ItemAmount, 0,0, false, stats);
							}
							//-------------------------------

							//Trade beenden
							Client->Send("exc_close 1");
							currentMap.Players[e]->Send("exc_close 1");

							//Trade löschen
							Client->Character->trade_active = false;
							currentMap.Players[e]->Character->trade_active = false;

							//Request löschen
							this->Requests_Delete(Client, currentMap.Players[e]);	
						}else
						{
							Message.Message_Info(Client, "Waiting for accept.");
						}
					}
				}

				break;
			}
		}
	}
}
//------------------------------------------------------------
void TTrade::Trade_Handle(TSocket *Client, int action, int partnerid)
{
	switch(action)
	{
	case ACTION_REQUEST:
		this->Request_Handle(Client, partnerid);
		break;
	case ACTION_ACCEPT:
		this->Request_Accept(Client, partnerid);
		break;
	case ACTION_DECLINE:
		this->Request_Decline(Client, partnerid);
		break;
	case ACTION_CANCEL:
		this->Trade_Cancel(Client);
		break;
	case ACTION_FINISH:
		this->Trade_Finish(Client);
		break;
	}
}
//------------------------------------------------------------
void TTrade::Trade_Cancel(TSocket *Client)
{
	//exc_close 0
	//Schauen ob trade aktiv
	if(Client->Character->trade_active)
	{

		std::stringstream message;
		structMapData currentMap;
		int mapsize = Map.MapData.size();

		for(auto i = 0; i < mapsize; i++)
		{
			//Wenn Map gefunden zuweisen
			if(Client->Character->Map.id == Map.MapData[i].id)
			{
				currentMap = Map.MapData[i];
				break;
			}
		}

		int size;
		int trade_id = Client->Character->active_trade_id;

		//Alle Spieler auf der Map ablaufen
		size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == trade_id)
			{
				//Schauen ob Request vorhanden
				bool request_allready = this->Request_Check(Client, currentMap.Players[e]->Character->CharacterInfo.ID);

				if(request_allready)
				{
					//Abbruch schicken
					Client->Send("exc_close 0");
					currentMap.Players[e]->Send("exc_close 0");

					//Trade löschen
					Client->Character->trade_active = false;
					currentMap.Players[e]->Character->trade_active = false;

					Client->Character->Inventory.trade_ready = false;
					currentMap.Players[e]->Character->Inventory.trade_ready = false;

					//Request löschen
					this->Requests_Delete(Client, currentMap.Players[e]);
				}
			}
		}
	}

}
//------------------------------------------------------------
void TTrade::Request_Accept(TSocket *Client, int partnerid)
{
	std::stringstream message;
	structMapData currentMap;
	int mapsize = Map.MapData.size();

	for(auto i = 0; i < mapsize; i++)
	{
		//Wenn Map gefunden zuweisen
		if(Client->Character->Map.id == Map.MapData[i].id)
		{
			currentMap = Map.MapData[i];
			break;
		}
	}

	int size;

	//Check ob er mit sich selber Handeln will
	if(partnerid != Client->Character->CharacterInfo.ID)
	{
		//Alle Spieler auf der Map ablaufen
		size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == partnerid)
			{
				//Schauen ob Request vorhanden
				bool request_allready = this->Request_Check(Client, partnerid);

				if(request_allready)
				{
					//Schauen ob kein anderer Trade aktiv ist)
					if(!Client->Character->trade_active && !currentMap.Players[e]->Character->trade_active)
					{
						//Trade Annahme schicken (Trade Fenster)
						//exc_list 1 825479 -1
						message << "exc_list 1 " << currentMap.Players[e]->Character->CharacterInfo.ID << " -1";
						Client->Send(message.str());

						message.str("");
						message << "exc_list 1 " << Client->Character->CharacterInfo.ID << " -1";
						currentMap.Players[e]->Send(message.str());

						//Trade aktiv setzen
						Client->Character->active_trade_id = currentMap.Players[e]->Character->CharacterInfo.ID;
						currentMap.Players[e]->Character->active_trade_id = Client->Character->CharacterInfo.ID;
						Client->Character->trade_active = true;
						currentMap.Players[e]->Character->trade_active = true;
						Client->Character->Inventory.trade_ready = false;
						currentMap.Players[e]->Character->Inventory.trade_ready = false;
						Client->Character->Inventory.trade_finish = false;
						currentMap.Players[e]->Character->Inventory.trade_finish = false;

					}else
					{
						//Einer von beiden ist gerade am traden
						message << "info Someone is allready trading.";
						Client->Send(message.str());

						message.str("");
						message << "info Someone is allready trading.";
						currentMap.Players[e]->Send(message.str());
						//Request entfernen
						this->Requests_Delete(Client, currentMap.Players[e]);
					}
				}
				break;
			}
		}
	}
}
//------------------------------------------------------------
void TTrade::Request_Decline(TSocket *Client, int partnerid)
{
	std::stringstream message;
	structMapData currentMap;
	int mapsize = Map.MapData.size();

	for(auto i = 0; i < mapsize; i++)
	{
		//Wenn Map gefunden zuweisen
		if(Client->Character->Map.id == Map.MapData[i].id)
		{
			currentMap = Map.MapData[i];
			break;
		}
	}

	int size;

	//Check ob er mit sich selber Handeln will
	if(partnerid != Client->Character->CharacterInfo.ID)
	{
		//Alle Spieler auf der Map ablaufen
		size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == partnerid)
			{
				//Schauen ob Request vorhanden
				bool request_done = this->Requests_Delete(Client, currentMap.Players[e]);

				if(request_done)
				{
					//Trade Ablehnung schicken
					message << "say 1 " << currentMap.Players[e]->Character->CharacterInfo.ID << " 10 " << Client->Character->CharacterInfo.Name << " declined the traderequest";
					currentMap.Players[e]->Send(message.str());
				}
				break;
			}
		}
	}
}
//------------------------------------------------------------
bool TTrade::Request_Check(TSocket *Client, int id)
{
	int size = Client->Character->TradeRequests.size();

	for(int i = 0; i < size; i++)
	{
		if(Client->Character->TradeRequests[i] == id)
		{
			return true;
		}
	}

	return false;
}
//------------------------------------------------------------
bool TTrade::Requests_Delete(TSocket *Client, TSocket *Partner)
{
	int ClientID = Client->Character->CharacterInfo.ID;
	int PartnerID = Partner->Character->CharacterInfo.ID;

	int size = Client->Character->TradeRequests.size();

	bool ClientDel = false;

	for(int i = 0; i < size; i++)
	{
		if(Client->Character->TradeRequests[i] == PartnerID)
		{
			//Requests löschen
			Client->Character->TradeRequests.erase(Client->Character->TradeRequests.begin() + i);
			ClientDel = true;
		}
	}

	bool PartnerDel = false;
	size = Partner->Character->TradeRequests.size();

	for(int i = 0; i < size; i++)
	{
		if(Partner->Character->TradeRequests[i] == ClientID)
		{
			//Requests löschen
			Partner->Character->TradeRequests.erase(Partner->Character->TradeRequests.begin() + i);
			PartnerDel = true;
		}
	}

	if(ClientDel && PartnerDel)
	{
		return true;
	}else
	{
		return false;
	}
}
//------------------------------------------------------------

void TTrade::Request_Handle(TSocket *Client, int partnerid)
{
	std::stringstream message;
	structMapData currentMap;
	int mapsize = Map.MapData.size();

	for(auto i = 0; i < mapsize; i++)
	{
		//Wenn Map gefunden zuweisen
		if(Client->Character->Map.id == Map.MapData[i].id)
		{
			currentMap = Map.MapData[i];
			break;
		}
	}

	int size;

	//Check ob er mit sich selber Handeln will
	if(partnerid != Client->Character->CharacterInfo.ID)
	{
		//Alle Spieler auf der Map ablaufen
		size = currentMap.Players.size();

		for(auto e = 0; e < size; e++)
		{
			//Wenn Spieler gefunden
			if(currentMap.Players[e]->Character->CharacterInfo.ID == partnerid)
			{
				//Schauen ob ein Request schon besteht
				bool request_allready = this->Request_Check(Client, partnerid);

				if(!request_allready)
				{
					//TODO: Blacklist check
					//Requests erstellen
					Client->Character->TradeRequests.push_back(partnerid);
					currentMap.Players[e]->Character->TradeRequests.push_back(Client->Character->CharacterInfo.ID);

					//Trade Anfrage schicken
					//modal 0 Du hast ,knox nach einem Tausch gefragt.
					message << "modal 0 You've asked " << currentMap.Players[e]->Character->CharacterInfo.Name << " for a trade.";
					Client->Send(message.str());
					message.str("");
					//dlg #req_exc^2^825479 #req_exc^5^825479 Tausch von KnoxLiebtsAnal akzeptieren?
					message << "dlg #req_exc^" << ACTION_ACCEPT << "^" << Client->Character->CharacterInfo.ID << " #req_exc^" << ACTION_DECLINE << "^" << Client->Character->CharacterInfo.ID << " Accept " << Client->Character->CharacterInfo.Name << "'s trade?";
					currentMap.Players[e]->Send(message.str());
				}else
				{
					message << "info You're allready trading.";
					Client->Send(message.str());
				}
				break;
			}
		}
	}

	return;
}

//------------------------------------------------------------