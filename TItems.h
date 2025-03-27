#pragma once

#define ITEM_NOT_FOUND -1
//Spezielle Items
#define SITEM_GOLD 1046

//Settings
#define ITEM_DROPBOUND_TIME 30
#define ITEM_LIFE_TIME 60000

#ifndef _H_ITEMS
#define _H_ITEMS

//Typedefintionen

class TItems;
extern TItems Items;

//--------------------------

//Strukturen
struct structItemStats
{
	//Short Range
	int sr_damage_min;
	int sr_damage_max;
	int sr_hitrate;
	int sr_crit_chance;
	int sr_crit_damage;
	int sr_defence;
	//Longe Range
	int lr_damage_min;
	int lr_damage_max;
	int lr_hitrate;
	int lr_crit_chance;
	int lr_crit_damage;
	int lr_defence;
	//Magic
	int magic_defence;
	//Def
	int dodge;
};

struct structItems
{
	std::string name;
	int vnum;
	int inventorytype;
	int price;
	std::vector<int> effects;
	bool on_use_delete;
	int equipment_slot;
	structItemStats stats;
};

//------------------------

class TItems
{

public:
	TItems(void); //Konstruktor
	~TItems(void); //Destruktor

	std::vector<structItems> vItems; //Item Array

	int MakeItem(TSocket *Client, int itemid); //Item benutzen
	int GetItemIndex(int vnum); //Item Index returnen
	int GetEquipmentSlot( int vnum );
	void Bound_Check(int mapindex, int itemindex); //
	void Lifetime_Check(int mapindex, int itemindex); //
	int s_ItID(int mapindex);

	std::mutex lockthread;
	static DWORD WINAPI ItemsThread(LPVOID lpdwThreadParam);
};

#endif _H_ITEMS
