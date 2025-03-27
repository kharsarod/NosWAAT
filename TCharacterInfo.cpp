#include "stdafx.h"
#include "TCharacterInfo.h"

TCharacterInfo CharacterInfo;
//------------------------------------------------------------

TCharacterInfo::TCharacterInfo(void)
{
}

//------------------------------------------------------------

TCharacterInfo::~TCharacterInfo(void)
{
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterInfo( TSocket *Client )
{
	//Packet erstellen und senden
	std::stringstream ss;
	ss << "c_info " << Client->Character->CharacterInfo.Name << " - -1 -1 - " << Client->Character->CharacterInfo.ID << " 1 "
		<< Client->Character->CharacterInfo.Gender << " " << Client->Character->CharacterInfo.Hairstyle << " "
		<< Client->Character->CharacterInfo.Haircolor << " 0 1 0 0 0";
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterSkills( TSocket *Client )
{
	//Packet erstellen und senden
	std::stringstream ss;
	int SkillsSize = Client->Character->CharacterSkills.size();
	ss << "ski 200 201"; //TODO: Primärer und Sekundärer Skill

	for(int i = 0; i < SkillsSize; i++)
	{
		ss << " " << Client->Character->CharacterSkills[i].vnum;
	}

	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterMap( TSocket *Client )
{
	//Packets erstellen und senden
	std::stringstream ss;

	ss << "at " << Client->Character->CharacterInfo.ID << " " << Client->Character->Map.id << " "
		<< Client->Character->Map.x << " " << Client->Character->Map.y << " 0 0 1 0";
	Client->Send(ss.str());

	ss.str("");
	ss << "c_map 0 " << Client->Character->Map.id << " 0";
	Client->Send(ss.str());

	this->Send_CharacterSpeed(Client);

	//Send Info Stuff
	Map.GetPortalsOnMap(Client);
	Map.GetNpcsOnMap(Client);
	Map.GetItemsOnMap(Client);
	Map.GetEnemiesOnMap(Client);
}

//------------------------------------------------------------

void TCharacterInfo::Send_PlayerOut( TSocket *Player, TSocket *Client )
{
	std::stringstream ss;
	ss << "out 1 " << Player->Character->CharacterInfo.ID;
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_PlayerIn( TSocket *Player, TSocket *Client )
{
	//Packets erstellen und senden
	//<kopf>.<körper>.<waffe 1>.<?>.<brille>.<kostüm>.<kostümkopf>.<?>
	std::stringstream ss;
	ss << "in " << OBJ_PLAYERS << " " << Player->Character->CharacterInfo.Name << " - " << Player->Character->CharacterInfo.ID << " "
		<< Player->Character->Map.x << " " << Player->Character->Map.y << " 1 0 " << Player->Character->CharacterInfo.Gender << " "
		<< Player->Character->CharacterInfo.Hairstyle << " " << Player->Character->CharacterInfo.Haircolor << " "
		<< "0" << " " 
		<< Player->Character->Equipment[EQ_SLOT_HEAD].vnum << "."
		<< Player->Character->Equipment[EQ_SLOT_CHEST].vnum << "."
		<< Player->Character->Equipment[EQ_SLOT_WEAPON1].vnum << "."
		<< "-1" << "."
		<< Player->Character->Equipment[EQ_SLOT_SPECS].vnum << "."
		<< Player->Character->Equipment[EQ_SLOT_KOSTUEM_CHEST].vnum << "."
		<< Player->Character->Equipment[EQ_SLOT_KOSTUEM_HEAD].vnum << "."
		<< "-1" << " "
		<< "100 100" << " "
		<< Player->Character->Status.isrest << " "
		<< "-1 0 0 0 0 0 0 0 0 -1 - 1 0 0 0 0 1 0";
	Client->Send(ss.str());

	ss.str("");
	ss << "in " << OBJ_PLAYERS << " " << Client->Character->CharacterInfo.Name << " - " << Client->Character->CharacterInfo.ID << " "
		<< Client->Character->Map.x << " " << Client->Character->Map.y << " 1 0 " << Client->Character->CharacterInfo.Gender << " "
		<< Client->Character->CharacterInfo.Hairstyle << " " << Client->Character->CharacterInfo.Haircolor << " "
		<< "0" << " " 
		<< Client->Character->Equipment[EQ_SLOT_HEAD].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_CHEST].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_WEAPON1].vnum << "."
		<< "-1" << "."
		<< Client->Character->Equipment[EQ_SLOT_SPECS].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_KOSTUEM_CHEST].vnum << "."
		<< Client->Character->Equipment[EQ_SLOT_KOSTUEM_HEAD].vnum << "."
		<< "-1" << " "
		<< "100 100" << " "
		<< Client->Character->Status.isrest << " "
		<< "-1 0 0 0 0 0 0 0 0 -1 - 1 0 0 0 0 1 0";
	Player->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Save_Character(TSocket *Client)
{
	//TServer -> Playerdisconnect -> Save Character
	//TServer -> Gewisse Zeit -> Auto Save Character
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterMapout( TSocket *Client )
{
	//Packet erstellen und senden
	std::stringstream ss;
	ss << "mapout";
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterSpeed( TSocket *Client )
{
	std::stringstream ss;
	ss << "cond 1 " << Client->Character->CharacterInfo.ID << " 0 0 " << Client->Character->Map.speed;
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterInventory( TSocket *Client )
{
	std::stringstream equip, main, other;
	equip << "inv 0";
	main << "inv 1";
	other << "inv 2";

	//Anzahl Freier Slots im Inventar laden
	if(Client->Character->Inventory.BackpackActive)
	{
		Client->Send("exts 45 45 45");
	} else
	{
		Client->Send("exts 32 32 32");
	}

	//Alle Inventar Items ins Packet laden

	for(auto i = 0; i < MAX_SLOTS_BACKPACK; i++)
	{
		// 0 = EQUIP, 1 = MAIN, 2 = OTHER
		if(Client->Character->Inventory.Index[0][i].inUse == true)
		{
			equip << " " << i << "." << Client->Character->Inventory.Index[0][i].id << "." << Client->Character->Inventory.Index[0][i].rare << "." << Client->Character->Inventory.Index[0][i].upgrade;
		}

		if(Client->Character->Inventory.Index[1][i].inUse == true)
		{
			main << " " << i << "." << Client->Character->Inventory.Index[1][i].id << "." << Client->Character->Inventory.Index[1][i].count;
		}

		if(Client->Character->Inventory.Index[2][i].inUse == true)
		{
			other << " " << i << "." << Client->Character->Inventory.Index[2][i].id << "." << Client->Character->Inventory.Index[2][i].count;
		}
	}
	

	//Inventar Packets senden
	Client->Send(equip.str());
	Client->Send(main.str());
	Client->Send(other.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterStats( TSocket *Client )
{
	std::stringstream ss;
	ss << "stat " << Client->Character->Status.currenthp << " " << Client->Character->Status.maxhp << 
		" " << Client->Character->Status.currentmp << " " << Client->Character->Status.maxmp << " 0 1024";
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterGold( TSocket *Client )
{
	std::stringstream ss;
	ss << "gold " << Client->Character->Inventory.gold_count << " 0";
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::GetAllMapPackets( TSocket *Client )
{
	CharacterInfo.lockthread.lock();
	CharacterInfo.Send_CharacterInfo(Client);
	CharacterInfo.Send_CharacterStats(Client);
	CharacterInfo.Send_CharacterLevel(Client);
	CharacterInfo.Send_CharacterMap(Client);
	CharacterInfo.Send_CharacterSkills(Client);
	CharacterInfo.Send_CharacterGold(Client);
	CharacterInfo.Send_CharacterInventory(Client);
	CharacterInfo.Send_Quicklist(Client);
	CharacterInfo.Send_Equipment(Client);

	//Stats
	CharacterInfo.Stats_CalcReal(Client);
	CharacterInfo.Send_CharacterStatPoints(Client);
	Chat.LoadSystemMessages(Client);

	Map.AddPlayerToMap(Client);
	CharacterInfo.lockthread.unlock();
}

//------------------------------------------------------------

void TCharacterInfo::Quicklist_Change( TSocket *Client, int type, int list, int slot, int maintype, int id)
{
	//qset <type> <qlist> <slot> <maintype> <id>
	if(list < 3 && list >= 0)
	{
		if(slot < 10 && slot >= 0)
		{
			Client->Character->Quicklist[list].slot[slot].id = id;
			Client->Character->Quicklist[list].slot[slot].type = type;
			Client->Character->Quicklist[list].slot[slot].maintype = maintype;
		}
	}

	this->Send_Quicklist(Client);
}

//------------------------------------------------------------

void TCharacterInfo::Quicklist_Remove( TSocket *Client, int type, int list, int slot)
{
	//qset <type> <qlist> <slot> <maintype> <id>
	if(list < 3 && list >= 0)
	{
		if(slot < 10 && slot >= 0)
		{
			if(type == 3)
			{
				Client->Character->Quicklist[list].slot[slot].id = QL_NONE;
				Client->Character->Quicklist[list].slot[slot].type = QL_TYPE_NONE;
				Client->Character->Quicklist[list].slot[slot].maintype = QL_MAINTYPE_NONE;
			}
		}
	}

	this->Send_Quicklist(Client);
}

//------------------------------------------------------------
void TCharacterInfo::Send_Quicklist( TSocket *Client )
{
	std::stringstream message;

	for(int i = 0; i < 3; i++)
	{
		message << "qslot " << i << " ";

		for(int t = 0; t < 10; t++)
		{
			message << Client->Character->Quicklist[i].slot[t].type << "." << Client->Character->Quicklist[i].slot[t].maintype << "." << Client->Character->Quicklist[i].slot[t].id << " ";
		}

		Client->Send(message.str());
		message.str("");
	}
}

//------------------------------------------------------------

void TCharacterInfo::Send_Equipment( TSocket *Client)
{
	std::stringstream message;
	message << "equip 0 0 ";

	for(int i = 0; i < 15; i++)
	{
		if(Client->Character->Equipment[i].vnum != EQUIP_NOITEM)
		{
			message << i << "." << Client->Character->Equipment[i].vnum << "." << Client->Character->Equipment[i].rare << "." << Client->Character->Equipment[i].upgrade << " ";
		}
	}

	Client->Send(message.str());
}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterLevel( TSocket *Client )
{
	std::stringstream ss;
	
	ss << "lev " << Client->Character->Level.mainlevel << " " << Client->Character->Level.mainexp
		<< " " << Client->Character->Level.secondlevel << " " << Client->Character->Level.secondexp
		<< " " << Client->Character->Level.mainexp_levelup << " " << Client->Character->Level.secondexp_levelup
		<< " 0 0";
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TCharacterInfo::XP_Add(TSocket *Client, int mainxp, int secondxp, int spxp)
{
	Client->Character->Level.mainexp += mainxp * Config.Rate_XP;
	Client->Character->Level.secondexp += secondxp * Config.Rate_JXP;

	//level Up
	if(Client->Character->Level.mainexp >= Client->Character->Level.mainexp_levelup)
	{
		Client->Character->Level.mainexp = Client->Character->Level.mainexp - Client->Character->Level.mainexp_levelup;
		Client->Character->Level.mainlevel += 1;
		Client->Character->Level.mainexp_levelup = 300 * Client->Character->Level.mainlevel;

		int MapIndex = Map.GetMapIndex(Client->Character->Map.id);

		std::stringstream message;
		message << "msg 0 Your combat level has been increased.";
		Client->Send(message.str());
		message.str("");
		message << "eff 1 " << Client->Character->CharacterInfo.ID << " 6";
		Map.SendMap(MapIndex, NULL, true, message.str());
		message.str("");
		message << "eff 1 " << Client->Character->CharacterInfo.ID << " 198";
		Map.SendMap(MapIndex, NULL, true, message.str());

		CharacterInfo.Stats_CalcReal(Client);
		CharacterInfo.Send_CharacterStatPoints(Client);
		Client->Character->Status.currenthp = Client->Character->Status.maxhp;
		Client->Character->Status.currentmp = Client->Character->Status.maxmp;
		CharacterInfo.Send_CharacterStats(Client);
	}
	//level Up
	if(Client->Character->Level.secondexp >= Client->Character->Level.secondexp_levelup)
	{
		Client->Character->Level.secondexp = Client->Character->Level.secondexp - Client->Character->Level.secondexp_levelup;
		Client->Character->Level.secondlevel += 1;
		Client->Character->Level.secondexp_levelup = 2200 * Client->Character->Level.secondlevel;

		std::stringstream message;
		message << "msg 0 Your job level has been increased.";
		Client->Send(message.str());

		Client->Character->Status.currenthp = Client->Character->Status.maxhp;
		Client->Character->Status.currentmp = Client->Character->Status.maxmp;
		CharacterInfo.Send_CharacterStats(Client);
	}
	this->Send_CharacterLevel(Client);
}

//------------------------------------------------------------

int TCharacterInfo::Classes_GetbyID(int id)
{
	int classes_size = CharacterClasses.size();

	for(int i = 0; i < classes_size; i++)
	{
		if(CharacterClasses[i].id == id);
		{
			return i;
		}
	}

}

//------------------------------------------------------------

void TCharacterInfo::Send_CharacterStatPoints(TSocket *Client)
{
	//sc <FIRE> <GET DA WHATA NIGGA> <LIGHT> <Darkness>
	std::stringstream message;
	message << "sc ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.type << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.attackgrade << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.dmg_min << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.dmg_max << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.hitrate << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.critchance << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Primary.critdamage << " ";

	message << Client->Character->Stats.Stats_Real.Attack_Secondary.type << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.attackgrade << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_min << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_max << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.hitrate << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.critchance << " ";
	message << Client->Character->Stats.Stats_Real.Attack_Secondary.critdamage << " ";

	message << Client->Character->Stats.Stats_Real.Defence.DefenceGrade << " ";
	message << Client->Character->Stats.Stats_Real.Defence.Short_Defence << " ";
	message << Client->Character->Stats.Stats_Real.Defence.Short_Dodge << " ";
	message << Client->Character->Stats.Stats_Real.Defence.Long_Defence << " ";
	message << Client->Character->Stats.Stats_Real.Defence.Long_Dodge << " ";
	message << Client->Character->Stats.Stats_Real.Defence.Magic_Decrease << " ";
	
	message << Client->Character->Stats.Stats_Real.Resistances.Fire << " ";
	message << Client->Character->Stats.Stats_Real.Resistances.Water << " ";
	message << Client->Character->Stats.Stats_Real.Resistances.Light << " ";
	message << Client->Character->Stats.Stats_Real.Resistances.Darkness;

	Client->Send(message.str());
}

//------------------------------------------------------------

void TCharacterInfo::Stats_CalcReal(TSocket *Client)
{
	int Level = Client->Character->Level.mainlevel;

	Client->Character->Stats.Stats_Real = Client->Character->Stats.Stats_Base;
	Client->Character->Stats.Stats_Real.Basic.stamina += (2 *Level);
	Client->Character->Stats.Stats_Real.Basic.intelligence += (2 *Level);
	//Client->CharacterList[currentslot].Stats.Stats_Base.Basic.walkspeed += (2 *Level);

	//PA
	//Client->Character->Stats.Stats_Real.Attack_Primary.attackgrade += (2 *Level);
	//Client->Character->Stats.Stats_Real.Attack_Primary.critchance += (2 *Level);
	//Client->Character->Stats.Stats_Real.Attack_Primary.critdamage += (2 *Level);
	Client->Character->Stats.Stats_Real.Attack_Primary.dmg_max += Level;
	Client->Character->Stats.Stats_Real.Attack_Primary.dmg_min += Level;
	Client->Character->Stats.Stats_Real.Attack_Primary.hitrate += Level;
	//Client->CharacterList[currentslot].Stats.Stats_Base.Attack_Primary.type += (2 *Level);

	//SA
	//Client->Character->Stats.Stats_Real.Attack_Secondary.attackgrade += (2 *Level);
	//Client->Character->Stats.Stats_Real.Attack_Secondary.critchance += (2 *Level);
	//Client->Character->Stats.Stats_Real.Attack_Secondary.critdamage += (2 *Level);
	Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_max += Level;
	Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_min += Level;
	Client->Character->Stats.Stats_Real.Attack_Secondary.hitrate += (2 *Level);
	//Client->CharacterList[currentslot].Stats.Stats_Base.Attack_Secondary.type += (2 *Level);

	//Defence
	//Client->Character->Stats.Stats_Real.Defence.DefenceGrade += (2 *Level);
	Client->Character->Stats.Stats_Real.Defence.Long_Dodge += Level;
	Client->Character->Stats.Stats_Real.Defence.Short_Dodge += Level;

	int defplus = 0;
	int stamplus = 0;
	int intplus = 0;

	for(int i = 1; i <= Level; i++)
	{
		if(i%2 == 1){defplus++;}
		stamplus += (Client->Character->Stats.Stats_Real.Basic.stam_plus + i);
		intplus += (Client->Character->Stats.Stats_Real.Basic.int_plus + i);
	}

	Client->Character->Stats.Stats_Real.Defence.Long_Defence += defplus;
	Client->Character->Stats.Stats_Real.Defence.Magic_Decrease += defplus;
	Client->Character->Stats.Stats_Real.Defence.Short_Defence += defplus;

	Client->Character->Status.maxhp = Client->Character->Stats.Stats_Real.Basic.stamina + stamplus;
	Client->Character->Status.maxmp = Client->Character->Stats.Stats_Real.Basic.intelligence + intplus;
	Client->Character->Status.currenthp = Client->Character->Stats.Stats_Real.Basic.stamina + stamplus;
	Client->Character->Status.currentmp = Client->Character->Stats.Stats_Real.Basic.intelligence + intplus;

	//Resis
	//Client->Character->Stats.Stats_Real.Resistances.Darkness += (2 *Level);
	//Client->Character->Stats.Stats_Real.Resistances.Fire += (2 *Level);
	//Client->Character->Stats.Stats_Real.Resistances.Light += (2 *Level);
	//Client->Character->Stats.Stats_Real.Resistances.Water += (2 *Level);

	//EQ STATS
	for(int i = 0; i < 15; i++)
	{
		if(Client->Character->Equipment[i].vnum != EQUIP_NOITEM)
		{
			Client->Character->Stats.Stats_Real.Attack_Primary.dmg_min += Client->Character->Equipment[i].stats.sr_damage_min;
			Client->Character->Stats.Stats_Real.Attack_Primary.dmg_max += Client->Character->Equipment[i].stats.sr_damage_max;
			Client->Character->Stats.Stats_Real.Attack_Primary.critchance += Client->Character->Equipment[i].stats.sr_crit_chance;
			Client->Character->Stats.Stats_Real.Attack_Primary.critdamage += Client->Character->Equipment[i].stats.sr_crit_damage;
			Client->Character->Stats.Stats_Real.Attack_Primary.hitrate += Client->Character->Equipment[i].stats.sr_hitrate;

			Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_min += Client->Character->Equipment[i].stats.lr_damage_min;
			Client->Character->Stats.Stats_Real.Attack_Secondary.dmg_max += Client->Character->Equipment[i].stats.lr_damage_max;
			Client->Character->Stats.Stats_Real.Attack_Secondary.critchance += Client->Character->Equipment[i].stats.lr_crit_chance;
			Client->Character->Stats.Stats_Real.Attack_Secondary.critdamage += Client->Character->Equipment[i].stats.lr_crit_damage;
			Client->Character->Stats.Stats_Real.Attack_Secondary.hitrate += Client->Character->Equipment[i].stats.lr_hitrate;

			Client->Character->Stats.Stats_Real.Defence.Short_Defence += Client->Character->Equipment[i].stats.sr_defence;
			Client->Character->Stats.Stats_Real.Defence.Long_Defence += Client->Character->Equipment[i].stats.lr_defence;

			Client->Character->Stats.Stats_Real.Defence.Magic_Decrease += Client->Character->Equipment[i].stats.magic_defence;
			Client->Character->Stats.Stats_Real.Defence.Long_Dodge += Client->Character->Equipment[i].stats.dodge;
			Client->Character->Stats.Stats_Real.Defence.Short_Dodge += Client->Character->Equipment[i].stats.dodge;
		}
	}
}

//------------------------------------------------------------

void TCharacterInfo::Stats_GetBase(TSocket *Client, int currentslot)
{
	int CCA = this->Classes_GetbyID(Client->CharacterList[currentslot].CharacterInfo.ClassType);
	int Level = Client->CharacterList[currentslot].Level.mainlevel;

	//Basic Stats
	Client->CharacterList[currentslot].Stats.Stats_Base.Basic = CharacterClasses[CCA].Basic;

	//PA
	Client->CharacterList[currentslot].Stats.Stats_Base.Attack_Primary = CharacterClasses[CCA].Attack_Primary;

	//SA
	Client->CharacterList[currentslot].Stats.Stats_Base.Attack_Secondary = CharacterClasses[CCA].Attack_Secondary;

	//Defence
	Client->CharacterList[currentslot].Stats.Stats_Base.Defence = CharacterClasses[CCA].Defence;

	//Resis
	Client->CharacterList[currentslot].Stats.Stats_Base.Resistances = CharacterClasses[CCA].Resistances;
}

//------------------------------------------------------------

void TCharacterInfo::Player_Rest(TSocket *Client, int param1, int param2, int id)
{
	Client->Character->Status.isrest = !Client->Character->Status.isrest;
	int mapindex = Map.GetMapIndex(Client->Character->Map.id);
	std::stringstream message;
	//rest 1 557826 0
	message << "rest 1 " << Client->Character->CharacterInfo.ID << " " << Client->Character->Status.isrest;
	Map.SendMap(mapindex, NULL, true, message.str());
	if(Client->Character->Status.isrest)
	{
		Client->Character->Status.rest_next = (GetTickCount() / 1000) + REST_TIME;
	}
}

//------------------------------------------------------------

void TCharacterInfo::Rest_Check(int mapindex, int playerindex)
{
	if(Map.MapData[mapindex].Players[playerindex]->Character->Status.isrest)
	{
		int time = (GetTickCount() / 1000);
		if(time >= Map.MapData[mapindex].Players[playerindex]->Character->Status.rest_next)
		{
			int hp_perc = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxhp / REST_PERC;
			int mp_perc = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxmp / REST_PERC;

			Map.MapData[mapindex].Players[playerindex]->Character->Status.currenthp += hp_perc;
			Map.MapData[mapindex].Players[playerindex]->Character->Status.currentmp += mp_perc;

			if(Map.MapData[mapindex].Players[playerindex]->Character->Status.currenthp > Map.MapData[mapindex].Players[playerindex]->Character->Status.maxhp)
				Map.MapData[mapindex].Players[playerindex]->Character->Status.currenthp = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxhp;

			if(Map.MapData[mapindex].Players[playerindex]->Character->Status.currentmp > Map.MapData[mapindex].Players[playerindex]->Character->Status.maxmp)
				Map.MapData[mapindex].Players[playerindex]->Character->Status.currentmp = Map.MapData[mapindex].Players[playerindex]->Character->Status.maxmp;

			CharacterInfo.Send_CharacterStats(Map.MapData[mapindex].Players[playerindex]);

			Map.MapData[mapindex].Players[playerindex]->Character->Status.rest_next = time + REST_TIME;
		}
	}
}

//------------------------------------------------------------

DWORD WINAPI TCharacterInfo::PlayerThread(LPVOID lpdwThreadParam)
{
	Sleep(100);

	std::cout << "TCharacterInfo::PlayerThread() running" << std::endl;
	do 
	{
		int mapsize = Map.MapData.size();
		int playersize;

		for(int i = 0; i < mapsize; i++)
		{
			CharacterInfo.lockthread.lock();

			playersize = Map.MapData[i].Players.size();

			for(int t = 0; t < playersize; t++)
			{
				Skills.Player_CheckCast(i,t);
				CharacterInfo.Rest_Check(i,t);
				Group.Player_SendStats(i,t);
			}

			CharacterInfo.lockthread.unlock();
		}

		Sleep(7);
	} while (1);
}