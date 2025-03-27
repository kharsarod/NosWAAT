#include "stdafx.h"
#include "TCharacter.h"

TCharacter Character;
//------------------------------------------------------------

TCharacter::TCharacter(void)
{
}

//------------------------------------------------------------

TCharacter::~TCharacter(void)
{
}

//------------------------------------------------------------

void TCharacter::LoadCharacter( TSocket *Client )
{
	//Wenn Account noch nicht überprüft, dann überprüfen
	if(Client->Account->Checked != true)
	{
		bool result = Account.Check(Client);
		delete Client->Character;
		delete Database.stmt;
		delete Database.result;

		if(result == true)
		{
			return;
		}

		Client->Account->Checked = true;
	}

	//Charakterliste laden
	this->ExecuteCharacterList(Client);
}

//------------------------------------------------------------

void TCharacter::ExecuteCharacterList( TSocket *Client )
{
	int current_slot = 0;
	std::stringstream ss;
	structCharacterSkills CharacterSkills;

	Client->CharacterList = new structCharacterList[3];

	//Datenbank abfragen
	try
	{

		Database.stmt = Database.connection->prepareStatement("SELECT * FROM tbl_characters WHERE AccountID = ? ORDER BY Slot");
		Database.stmt->setInt(1, Client->Account->accountid);
		Database.result = Database.stmt->executeQuery();

	} catch(sql::SQLException &e)
	{
		std::cout << "ERROR SQL Exception: " << e.getSQLStateCStr() << std::endl;
	}

	Client->Send("clist_start 0");

	while(Database.result->next())
	{
		current_slot = Database.result->getInt("Slot");
		Client->CharacterList[current_slot].isReceived = true;
		Client->CharacterList[current_slot].CharacterInfo.ID = Database.result->getInt("CharacterID");
		Client->CharacterList[current_slot].CharacterInfo.Slot = Database.result->getInt("Slot");
		Client->CharacterList[current_slot].CharacterInfo.Name = Database.result->getString("Name");
		Client->CharacterList[current_slot].CharacterInfo.Gender = Database.result->getBoolean("Gender");
		Client->CharacterList[current_slot].CharacterInfo.Hairstyle = Database.result->getInt("Hairstyle");
		Client->CharacterList[current_slot].CharacterInfo.Haircolor = Database.result->getInt("Haircolor");
		Client->CharacterList[current_slot].CharacterInfo.ClassType = Database.result->getInt("Class");
		Client->CharacterList[current_slot].Map.id = Database.result->getInt("Map");
		Client->CharacterList[current_slot].Map.x = Database.result->getInt("X");
		Client->CharacterList[current_slot].Map.y = Database.result->getInt("Y");
		Client->CharacterList[current_slot].Map.speed = Database.result->getInt("Speed");
		Client->CharacterList[current_slot].Status.currenthp = Database.result->getInt("CurrentHP");
		Client->CharacterList[current_slot].Status.currentmp = Database.result->getInt("CurrentMP");
		Client->CharacterList[current_slot].Status.maxhp = Database.result->getInt("MaxHP");
		Client->CharacterList[current_slot].Status.maxmp = Database.result->getInt("MaxMP");
		Client->CharacterList[current_slot].Inventory.gold_count = Database.result->getInt("Gold");
		Client->CharacterList[current_slot].Level.mainlevel = Database.result->getInt("Level");
		Client->CharacterList[current_slot].Level.secondlevel = Database.result->getInt("JobLevel");
		Client->CharacterList[current_slot].Level.mainexp = Database.result->getInt("LevelEXP");
		Client->CharacterList[current_slot].Level.secondexp = Database.result->getInt("JobLevelEXP");
		Client->CharacterList[current_slot].Inventory.BackpackActive = Database.result->getBoolean("BackpackActive");
		Inventory.ParseInventory(Client, current_slot, EQUIP_INVENTORY, Database.result->getString("Inventory 0"));
		Inventory.ParseInventory(Client, current_slot, MAIN_INVENTORY, Database.result->getString("Inventory 1"));
		Inventory.ParseInventory(Client, current_slot, OTHER_INVENTORY, Database.result->getString("Inventory 2"));
		Friend.ParseFriends(Client, current_slot, Database.result->getString("Friends"));

		CharacterInfo.Stats_GetBase(Client, current_slot);

		//Skills Temp
		CharacterSkills.vnum = 200;
		CharacterSkills.castid = 0;
		CharacterSkills.cooldown = 1;
		Client->CharacterList[current_slot].CharacterSkills.push_back(CharacterSkills);
		CharacterSkills.vnum = 201;
		CharacterSkills.castid = 1;
		CharacterSkills.cooldown = 1;
		Client->CharacterList[current_slot].CharacterSkills.push_back(CharacterSkills);

		//Temp solange keine Tabelle
		Client->CharacterList[current_slot].Level.mainexp_levelup = 300;
		Client->CharacterList[current_slot].Level.mainexp = 0;
		Client->CharacterList[current_slot].Level.secondexp_levelup = 2200;
		Client->CharacterList[current_slot].Level.secondexp = 0;

		//equipment
		Client->CharacterList[current_slot].Equipment.resize(15);

		for(int i = 0; i < 15; i++)
		{
			Client->CharacterList[current_slot].Equipment[i].vnum = EQUIP_NOITEM;
		}

		//Quicklist Temp
		Client->CharacterList[current_slot].Quicklist.resize(3);
		Client->CharacterList[current_slot].Quicklist[0].slot.resize(10);
		Client->CharacterList[current_slot].Quicklist[1].slot.resize(10);
		Client->CharacterList[current_slot].Quicklist[2].slot.resize(10);


		//Quicklist 1
		Client->CharacterList[current_slot].Quicklist[0].slot[0].type = QL_TYPE_SKILL;
		Client->CharacterList[current_slot].Quicklist[0].slot[0].maintype = QL_MAINTYPE_NSKILL;
		Client->CharacterList[current_slot].Quicklist[0].slot[0].id = 1;

		for(int i = 1; i < 10; i++)
		{
			Client->CharacterList[current_slot].Quicklist[0].slot[i].type = QL_TYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[0].slot[i].maintype = QL_MAINTYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[0].slot[i].id = QL_NONE;
		}

		for(int i = 0; i < 10; i++)
		{
			Client->CharacterList[current_slot].Quicklist[1].slot[i].type = QL_TYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[1].slot[i].maintype = QL_MAINTYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[1].slot[i].id = QL_NONE;
		}

		for(int i = 0; i < 10; i++)
		{
			Client->CharacterList[current_slot].Quicklist[2].slot[i].type = QL_TYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[2].slot[i].maintype = QL_MAINTYPE_NONE;
			Client->CharacterList[current_slot].Quicklist[2].slot[i].id = QL_NONE;
		}

		//Temp Settings
		Client->CharacterList[current_slot].Status.alive = true;
		Client->CharacterList[current_slot].Status.isrest = false;
		Client->CharacterList[current_slot].Group.ingroup = false;

		ss.str("");
		ss << "clist " << Client->CharacterList[current_slot].CharacterInfo.Slot << " " << Client->CharacterList[current_slot].CharacterInfo.Name << " 0 " << Client->CharacterList[current_slot].CharacterInfo.Gender << " " << Client->CharacterList[current_slot].CharacterInfo.Hairstyle << " " << Client->CharacterList[current_slot].CharacterInfo.Haircolor << " 0 " << Client->CharacterList[current_slot].CharacterInfo.ClassType << " 1 0.0.0.0.0.0.0.0 1 0 0 -1.-1 0 0";
		Client->Send(ss.str());
	}

	Client->Send("clist_end");

	delete Database.stmt;
	delete Database.result;
}

//------------------------------------------------------------

void TCharacter::NewCharacter( TSocket *Client, std::string name, int slot, bool gender, int hairstyle, int haircolor )
{
	//Neuen Charakter erstellen
	
	//Wenn alle Daten ok, Charakter in der Datenbank eintragen
	if(Client->CharacterList[slot].isReceived != true && gender >= 0 && gender <= 2 && slot >= 0 && slot <= 2 && hairstyle >= 0 && hairstyle <= 2 && haircolor >= 0 && haircolor <= 9)
	{
		for(auto i = 0; i < name.length(); i++)
		{
			if(name[i] < 0x30 || name[i] > 0x7E)
			{
				Message.Message_Info(Client, "You have an illegal char in your Name");
				return;
			}
		}
		Database.stmt = Database.connection->prepareStatement("SELECT * FROM tbl_characters WHERE Name = ?");
		Database.stmt->setString(1, name);
		Database.result = Database.stmt->executeQuery();

		if(Database.result->rowsCount() != 0)
		{
			Message.Message_Info(Client, "Charactername is already in use");
			return;
		}

		Database.stmt = Database.connection->prepareStatement("INSERT INTO tbl_characters (AccountID, Slot, Name, Gender, Hairstyle, Haircolor, X, Y) VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
		Database.stmt->setInt(1, Client->Account->accountid);
		Database.stmt->setInt(2, slot);
		Database.stmt->setString(3, name);
		Database.stmt->setBoolean(4, gender);
		Database.stmt->setInt(5, hairstyle);
		Database.stmt->setInt(6, haircolor);
		Database.stmt->setInt(7, General.CreateRandomNumber(75, 84));
		Database.stmt->setInt(8, General.CreateRandomNumber(111, 120));
		Database.stmt->executeQuery();
		delete Database.stmt;

		//Characterliste laden
		this->ExecuteCharacterList(Client);

	} else
	{
		Client->Disconnect();
	}
}

//------------------------------------------------------------

void TCharacter::DeleteCharacter( TSocket *Client, int slot, std::string password )
{
	//Wenn Daten ok, Charakter löschen
	if(slot >= 0 && slot <= 2 && Client->CharacterList[slot].isReceived)
	{
		if(password.compare(Client->Account->password) != 0)
		{
			Message.Message_Info(Client, "Wrong Password");
			return;
		}

		Database.stmt = Database.connection->prepareStatement("DELETE FROM tbl_characters WHERE AccountID = ? AND Slot = ?");
		Database.stmt->setInt(1, Client->Account->accountid);
		Database.stmt->setInt(2, slot);
		Database.stmt->executeQuery();
		delete Database.stmt;

		//Charakterliste laden
		delete[] Client->CharacterList;
		this->ExecuteCharacterList(Client);
	} else
	{
		Client->Disconnect();
	}
}

//------------------------------------------------------------


void TCharacter::SelectCharacter( TSocket *Client, int slot )
{
	//Prüfen ob Daten ok sind
	if(slot >= 0 && slot <= 2 && Client->CharacterList[slot].isReceived == true)
	{
		//Charakter Daten setzen
		Client->Character = &Client->CharacterList[slot];
	
		//Packet informationen senden
		Client->Account->isIngame = true;
		Client->Send("OK");
		this->Pulse_Activate(Client);

		CharacterInfo.GetAllMapPackets(Client);
		Friend.Send_Hello(Client);

		//Trade temp
		Client->Character->trade_active = false;
		//Group temp
		Client->Character->Group.ingroup = false;
		//Cast temp
		Client->Character->Cast.iscasting = false;

	} else
	{
		Client->Disconnect();
	}
}

//------------------------------------------------------------

void TCharacter::Pulse_Activate( TSocket *Client )
{
	Client->Character->Ping.lastpulsecount = 0;
	Client->Character->Ping.lastpulsetime = GetTickCount() / 1000;
	Client->Character->Ping.lastautopulse = GetTickCount() / 1000;
}

//------------------------------------------------------------

void TCharacter::Pulse_Check( TSocket *Client, int pulse)
{
	int time_should = Client->Character->Ping.lastpulsetime + 60;
	int count_should = Client->Character->Ping.lastpulsecount + 60;

	//Aktuelle pulse zeit über die soll zeit + 2 sekunden oder unter sollzeit - 2 sekunden

	if((GetTickCount() / 1000) > time_should + PING_BUFFER || (GetTickCount() / 1000) < time_should - PING_BUFFER)
	{
		Client->Disconnect();
		std::cout << "Time Error Disconnect" << std::endl;

	}else
	{
		if(pulse != count_should)
		{
			//kick
			Client->Disconnect();
			std::cout << "Count Error Disconnect" << std::endl;
		}else
		{
			//Alles okay
			Client->Character->Ping.lastpulsecount = pulse;
			Client->Character->Ping.lastpulsetime = GetTickCount() / 1000;
		}
	}
}

//------------------------------------------------------------

void TCharacter::Pulse_Auto_Check(TSocket *Client)
{
	int time_should = Client->Character->Ping.lastpulsetime;

	//Aktuelle pulse zeit über die soll zeit + 2 Minuten
	if((GetTickCount() / 1000) > time_should + PING_AUTO_BUFFER)
	{
		Client->Disconnect();	
	}

	Client->Character->Ping.lastautopulse = GetTickCount() / 1000;
}

//------------------------------------------------------------