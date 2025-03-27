#pragma once

#define OBJ_PLAYERS 1
#define OBJ_NPCS 2
#define OBJ_ENEMIES 3
#define OBJ_ITEMS 9
#define NPC_NOT_FOUND -1
#define ITEM_NOT_FOUND -1
#define PLAYER_NOT_FOUND -1
#ifndef _H_MAP
#define _H_MAP

//Typedefinitionen

class TSocket;
class TMap;
extern TMap Map;

//Strukturen-------------------------------------------------------------

struct structCharacterMap
{
	int id;
	int x;
	int y;
	int speed;
};

struct structMapZones
{
	short x;
	short y;
	std::vector<std::vector<char>>  grid;
};

struct structMapPortal
{
	int mapid;
	int portal_id;
	int portal_type;
	int x;
	int y;
	int target_id;
	int target_x;
	int target_y;
	bool isDisabled;
	int maptitle_id;
};

struct structMapNpc
{
	int level;
	int mapid;
	int vnum;
	int npc_id;
	int hp_cur;
	int hp_max;
	int mp_cur;
	int mp_max;
	int x;
	int y;
	int direction;
	bool shop; 
	int shoptype;
	int menutype;
	std::string shopname;
	std::vector<structShopList> shoplist;
	int dialogid;
};

struct structWaypoints
{
	int x;
	int y;
};

struct structPathfinding
{
	std::vector<structWaypoints> waypoints;
	bool chasing, started;
	int target_x, target_y;
	float nextposition_timer;
};

struct structMapEnemy
{
	int id;
	int vnum;
	int start_x, start_y;
	int x;
	int y;
	int level;
	int cur_hp, cur_mp;
	int max_hp, max_mp;
	int random_move_next;
	bool walking;
	int target_x;
	int target_y;
	std::vector<structDroptable> droptable;
	structDropGold dropgold;
	int xp;
	int jxp;
	structEnemyAttack attack;
	structPathfinding pathfinding;
	std::vector<structEnemySkillsUse> skills;
};

struct structMapEnemySpawn
{
	int mapid;
	int level_min;
	int level_max;
	int vnum;
	int x;
	int y;
	int active_enemy_id;
	int next_respawn_time;
};

struct structMapItemBound
{
	bool drop_bound_is;
	int drop_bound_charid;
	int drop_bound_fadetime;
	int lifetime;
};

struct structMapItem
{
	int id;
	int vnum;
	int amount;
	int x;
	int y;
	bool quest;
	int bound_id;
	int rare;
	int upgrade;
	bool enemy_drop;
	structMapItemBound dropbound;
	structItemStats stats;
};

struct structMapData
{
	int id;
	structMapZones zones;
	std::vector<TSocket*> Players;
	std::vector<structMapPortal> Portal;
	std::vector<structMapNpc> Npc;
	std::vector<structMapItem> Item;
	std::vector<structMapEnemySpawn> EnemySpawns;
	std::vector<structMapEnemy> Enemy;
};

//-------------------------------------------------------------------------

class TMap
{
public:
	TMap(void); //Konstruktor
	~TMap(void); //Destruktor

	std::vector<structMapData> MapData; //MapArray 
	
	void AddPlayerToMap(TSocket *Client); //Add Player to Map
	void RemovePlayerFromMap(TSocket *Client); //Remove Player from Map
	void GetPortalsOnMap(TSocket *Client); // Load Portals on Map
	void ChangeMapRequest(TSocket *Client); // Request Map change
	bool isBlockedZone(int mapindex, int x, int y); // Check if zone is a object
	void GetNpcsOnMap( TSocket *Client ); //Load Npcs on map
	void GetEnemiesOnMap( TSocket *Client ); //Load Enemies on map
	void GetItemsOnMap( TSocket *Client ); //Empfange alle items der Map
	void SendMap( int MapIndex, int id_myself, bool send_myself, std::string packet ); //An allen Spielern auf der Map packet senden
	int GetMapIndex(int mapid); //MapID Position returnen
	int GetMapNpcIndex(int mapid, int mapindex, int npc_id); //Map NPC Position returnen
	int GetMapItemIndex(int mapid, int mapindex, int itemid); //Map Item ID bekommen
	int GetMapEnemyIndex( int mapid, int mapindex, int enemy_id ); //Map Enemy Position returnen
	TSocket GetPlayerSocketByID( int playerid ); //Nach PlayerID suchen
	int GetPlayerAllocatedByID( int mapindex,  int playerid );
};

#endif _H_MAP