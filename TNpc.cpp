#include "stdafx.h"
#include "TNpc.h"

//Typedefinitionen
TNpc Npcs;

//------------------------------------------------------------

TNpc::TNpc(void)
{
}

//------------------------------------------------------------

TNpc::~TNpc(void)
{
}

//------------------------------------------------------------

void TNpc::OpenNpcDialog( TSocket *Client, int npc_id )
{
	std::stringstream dialog;
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	int NpcIndex = Map.GetMapNpcIndex(MapIndex, MapIndex, npc_id);

	if(NpcIndex == NPC_NOT_FOUND)
	{
		return;
	}


	//Wenn NPC kein richtiges Dialog hat, aber nen shop, direkt den shop öffnen
	if(Map.MapData[MapIndex].Npc[NpcIndex].dialogid == 9999 && Map.MapData[MapIndex].Npc[NpcIndex].shop == true)
	{
		NpcShop.OpenCustomShop(Client, MapIndex, NpcIndex);
	} else //Wenn NPC nen Dialog hat, Dialog öffnen
	{	
		dialog << "npc_req 2 " << npc_id << " " << Map.MapData[MapIndex].Npc[NpcIndex].dialogid;
		Client->Send(dialog.str());
	}
}

//------------------------------------------------------------

void TNpc::GetTargetStatus( TSocket *Client, int type, int id )
{
	std::stringstream ss;
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	int PlayerIndex, NpcIndex, EnemyIndex;

	switch (type)
	{
	case OBJ_PLAYERS:
		{
		TSocket *TargetClient = &Map.GetPlayerSocketByID(id);
		
		ss << "st " << type << " " << id << " "
			<< TargetClient->Character->Level.mainlevel << " " 
			<< ((TargetClient->Character->Status.currenthp *100) / TargetClient->Character->Status.maxhp) << " "
			<< ((TargetClient->Character->Status.currentmp *100) / TargetClient->Character->Status.maxmp) << " "
			<< TargetClient->Character->Status.maxhp << " " << TargetClient->Character->Status.maxmp;

		Client->Send(ss.str());
		}
		break;
	case OBJ_NPCS:
		NpcIndex = Map.GetMapNpcIndex(Client->Character->Map.id, MapIndex, id);

		if(NpcIndex == NPC_NOT_FOUND)
		{
			return;
		}

		ss << "st " << type << " " << id << " "
			<< Map.MapData[MapIndex].Npc[NpcIndex].level << " " 
			<< ((Map.MapData[MapIndex].Npc[NpcIndex].hp_cur *100) / Map.MapData[MapIndex].Npc[NpcIndex].hp_max) << " "
			<< ((Map.MapData[MapIndex].Npc[NpcIndex].mp_cur *100) / Map.MapData[MapIndex].Npc[NpcIndex].mp_max) << " "
			<< Map.MapData[MapIndex].Npc[NpcIndex].hp_max << " " << Map.MapData[MapIndex].Npc[NpcIndex].mp_max;

		Client->Send(ss.str());

		break;
	case OBJ_ENEMIES:
		EnemyIndex = Map.GetMapEnemyIndex(Client->Character->Map.id, MapIndex, id);

		if(EnemyIndex == NPC_NOT_FOUND)
		{
			return;
		}

			ss << "st " << type << " " << id << " "
				<< Map.MapData[MapIndex].Enemy[EnemyIndex].level << " " 
				<< ((Map.MapData[MapIndex].Enemy[EnemyIndex].cur_hp *100) / Map.MapData[MapIndex].Enemy[EnemyIndex].max_hp) << " "
				<< ((Map.MapData[MapIndex].Enemy[EnemyIndex].cur_mp *100) / Map.MapData[MapIndex].Enemy[EnemyIndex].max_mp) << " "
				<< Map.MapData[MapIndex].Enemy[EnemyIndex].max_hp << " " << Map.MapData[MapIndex].Enemy[EnemyIndex].max_mp;

			Client->Send(ss.str());
		break;
	}
}

//------------------------------------------------------------
