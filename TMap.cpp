#include "stdafx.h"
#include "TMap.h"

//Typedefinitionen

TMap Map;

//------------------------------------------------------------

TMap::TMap(void)
{
}

//------------------------------------------------------------

TMap::~TMap(void)
{
}

//------------------------------------------------------------

void TMap::AddPlayerToMap( TSocket *Client )
{
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int MapPlayersSize = this->MapData[MapIndex].Players.size();

	//Alle Spieler ablaufen und IN Packet senden
	for(auto y = 0; y < MapPlayersSize; y++)
	{
		CharacterInfo.Send_PlayerIn(Client, this->MapData[MapIndex].Players[y]);
	}

	//Eigenen Spieler zu der Map hinzufügen
	this->MapData[MapIndex].Players.push_back(Client);
	return;
}

//------------------------------------------------------------

void TMap::RemovePlayerFromMap( TSocket *Client )
{
	CharacterInfo.lockthread.lock();
	int playerarraypos = -1;
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int MapPlayersSize = this->MapData[MapIndex].Players.size();

	for(auto y = 0; y < MapPlayersSize; y++)
	{	
		//Wenn Charakter ungleich der eigene Charakter, Map verlassen senden
		if(this->MapData[MapIndex].Players[y]->Character->CharacterInfo.ID != Client->Character->CharacterInfo.ID)
		{
			CharacterInfo.Send_PlayerOut(Client, this->MapData[MapIndex].Players[y]);
		}else
		{
			//Ansonsten Position speichern
			playerarraypos = y;
		}
	}

	//Wenn Spieler nicht vorhanden, von der Map entfernen
	if(Client->Character->CharacterInfo.ID == this->MapData[MapIndex].Players[playerarraypos]->Character->CharacterInfo.ID)
	{
		this->MapData[MapIndex].Players.erase(this->MapData[MapIndex].Players.begin() + playerarraypos);
	}
	CharacterInfo.lockthread.unlock();
	return;
}

//------------------------------------------------------------

bool TMap::isBlockedZone( int mapindex, int x, int y )
{
	//Prüfen ob Coordinaten außerhalb der Map
	if(x > this->MapData[mapindex].zones.x || x < 1 || y > this->MapData[mapindex].zones.y || y < 1 || this->MapData[mapindex].zones.grid[y][x] == 1)
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------

void TMap::GetPortalsOnMap( TSocket *Client )
{
	std::stringstream ss;
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int portalsize = this->MapData[MapIndex].Portal.size();

	//Alle Portale auf der Map an den Spieler senden
	for(auto i = 0; i < portalsize; i++)
	{
		ss << "gp " << this->MapData[MapIndex].Portal[i].x << " " << this->MapData[MapIndex].Portal[i].y << " " << this->MapData[MapIndex].Portal[i].maptitle_id << " " 
			<< this->MapData[MapIndex].Portal[i].portal_type << " " << this->MapData[MapIndex].Portal[i].isDisabled;

		Client->Send(ss.str());
		ss.str("");
	}

	return;
}

//------------------------------------------------------------
void TMap::GetItemsOnMap( TSocket *Client )
{
	std::stringstream ss;
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int itemsize = this->MapData[MapIndex].Item.size();

	//Alle Items auf der Map an den Spieler senden

	for(auto i = 0; i < itemsize; i++)
	{
		//in 9 2024 4051959 3 5 10 0 0 0

		ss << "in " << OBJ_ITEMS << " " << this->MapData[MapIndex].Item[i].vnum << " " << this->MapData[MapIndex].Item[i].id << " " << this->MapData[MapIndex].Item[i].x << " " 
			<< this->MapData[MapIndex].Item[i].y << " " << this->MapData[MapIndex].Item[i].amount << " " << this->MapData[MapIndex].Item[i].quest << " " << this->MapData[MapIndex].Item[i].bound_id
			<< " 0";

		Client->Send(ss.str());

		ss.str("");
	}

	return;
}
//------------------------------------------------------------
void TMap::GetNpcsOnMap( TSocket *Client )
{
	std::stringstream ss;
	std::stringstream shop;
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int npcsize = this->MapData[MapIndex].Npc.size();

	//Alle Npcs auf der Map an den Spieler senden

	for(auto i = 0; i < npcsize; i++)
	{
		ss << "in " << OBJ_NPCS << " " << this->MapData[MapIndex].Npc[i].vnum << " " << this->MapData[MapIndex].Npc[i].npc_id << " " << this->MapData[MapIndex].Npc[i].x << " " 
			<< this->MapData[MapIndex].Npc[i].y << " " << this->MapData[MapIndex].Npc[i].direction << " " 
			<< ((Map.MapData[MapIndex].Npc[i].hp_cur *100) / Map.MapData[MapIndex].Npc[i].hp_max) << " " <<
			((Map.MapData[MapIndex].Npc[i].mp_cur *100) / Map.MapData[MapIndex].Npc[i].mp_max) << " " <<
			this->MapData[MapIndex].Npc[i].dialogid << " 0 0 -1 0 0 -1 -";

		Client->Send(ss.str());

		//Prüfen ob NPC ein Shop
		if(this->MapData[MapIndex].Npc[i].shop)
		{
			shop.str("");
			shop << "shop 2 " << this->MapData[MapIndex].Npc[i].npc_id << " " << this->MapData[MapIndex].Npc[i].shop << " " << this->MapData[MapIndex].Npc[i].menutype << " " << this->MapData[MapIndex].Npc[i].shoptype << " " << this->MapData[MapIndex].Npc[i].shopname;
			Client->Send(shop.str());
		}

		ss.str("");
	}

	return;

}
//------------------------------------------------------------
void TMap::GetEnemiesOnMap( TSocket *Client )
{
	Enemies.lockthread.lock();
	std::stringstream ss;
	std::stringstream shop;
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int enemysize = this->MapData[MapIndex].Enemy.size();

	//Alle Npcs auf der Map an den Spieler senden

	for(auto i = 0; i < enemysize; i++)
	{
		ss << "in " << OBJ_ENEMIES << " " << this->MapData[MapIndex].Enemy[i].vnum << " " << this->MapData[MapIndex].Enemy[i].id << " " << this->MapData[MapIndex].Enemy[i].x << " " << this->MapData[MapIndex].Enemy[i].y << " 0 100 100 0 0 0 -1 0 0 -1 -";

		Client->Send(ss.str());

		ss.str("");
	}

	Enemies.lockthread.unlock();
	return;

}
//------------------------------------------------------------
void TMap::ChangeMapRequest( TSocket *Client )
{
	int MapIndex = this->GetMapIndex(Client->Character->Map.id);
	int portalsize = this->MapData[MapIndex].Portal.size();

	//Alle Portale durchlaufen
	for(auto i = 0; i < portalsize; i++)
	{
		//Wenn Spieler auf dem richtigen Portal, Map wechseln
		if(Client->Character->Map.x < (this->MapData[MapIndex].Portal[i].x + 2) && Client->Character->Map.x  > (this->MapData[MapIndex].Portal[i].x -2)
			&& Client->Character->Map.y < (this->MapData[MapIndex].Portal[i].y + 2) && Client->Character->Map.y > (this->MapData[MapIndex].Portal[i].y - 2))
		{
			this->RemovePlayerFromMap(Client);

			Client->Character->Map.id = this->MapData[MapIndex].Portal[i].target_id;
			Client->Character->Map.x = this->MapData[MapIndex].Portal[i].target_x;
			Client->Character->Map.y = this->MapData[MapIndex].Portal[i].target_y;

			CharacterInfo.Send_CharacterMapout(Client);
			CharacterInfo.GetAllMapPackets(Client);
			return;
		}
	}
}

//------------------------------------------------------------

void TMap::SendMap( int MapIndex, int id_myself, bool send_myself, std::string packet )
{
	int MapPlayersSize = this->MapData[MapIndex].Players.size();

	for(auto i = 0; i < MapPlayersSize; i++)
	{
		if(send_myself || (!send_myself && this->MapData[MapIndex].Players[i]->Character->CharacterInfo.ID != id_myself ))
		{
			this->MapData[MapIndex].Players[i]->Send(packet);
		}
	}
}

//------------------------------------------------------------

int TMap::GetMapIndex( int mapid )
{
	int mapsize = this->MapData.size(); 
	for(auto i = 0; i < mapsize; i++)
	{
		if(mapid == this->MapData[i].id)
		{
			return i;
		}
	}
}
//------------------------------------------------------------

TSocket TMap::GetPlayerSocketByID( int playerid )
{
	int mapsize = this->MapData.size(); 

	for(auto i = 0; i < mapsize; i++)
	{
		int playersize = this->MapData[i].Players.size();

		for(int t = 0; t < playersize; t++)
		{
			if(this->MapData[i].Players[t]->Character->CharacterInfo.ID == playerid)
			{
				return *this->MapData[i].Players[t];
			}
		}
	}

}
//------------------------------------------------------------

int TMap::GetPlayerAllocatedByID( int mapindex, int playerid )
{

	int playersize = this->MapData[mapindex].Players.size();

	for(int t = 0; t < playersize; t++)
	{
		if(this->MapData[mapindex].Players[t]->Character->CharacterInfo.ID == playerid)
		{
			return t;
		}
	}

	return PLAYER_NOT_FOUND;
}
//------------------------------------------------------------

int TMap::GetMapNpcIndex( int mapid, int mapindex, int npc_id )
{
	int MapNpcSize = this->MapData[mapindex].Npc.size();

	for(auto i = 0; i < MapNpcSize; i++)
	{
		if(this->MapData[mapindex].Npc[i].npc_id == npc_id)
		{
			return i;
		}
	}

	return NPC_NOT_FOUND;
}
//------------------------------------------------------------

int TMap::GetMapEnemyIndex( int mapid, int mapindex, int enemy_id )
{
	int MapEnemySize = this->MapData[mapindex].Enemy.size();

	for(auto i = 0; i < MapEnemySize; i++)
	{
		if(this->MapData[mapindex].Enemy[i].id == enemy_id)
		{
			return i;
		}
	}

	return NPC_NOT_FOUND;
}
//------------------------------------------------------------

int TMap::GetMapItemIndex( int mapid, int mapindex, int itemid )
{
	int MapItemSize = this->MapData[mapindex].Item.size();

	for(auto i = 0; i < MapItemSize; i++)
	{
		if(this->MapData[mapindex].Item[i].id == itemid)
		{
			return i;
		}
	}

	return ITEM_NOT_FOUND;
}
