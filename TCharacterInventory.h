#pragma once
#define AUTODETECT_SLOT -1
#define AUTODETECT_INVENTORY -1
#define SLOTS_FULL -2
#define UNKNOWN_INVENTORY -2
#define NO_ITEM_STACK -3
#define MAX_SLOTS_BACKPACK 45
#define MAX_SLOTS_SINGLE 32
#define INVENTORY_SIZE 3
#define EQUIP_INVENTORY 0
#define MAIN_INVENTORY 1
#define OTHER_INVENTORY 2
#define GOLD_INVENTORY 99
#define EQUIP_NOITEM -1
#define EQ_SLOT_WEAPON1 0
#define EQ_SLOT_CHEST 1
#define EQ_SLOT_HEAD 2
#define EQ_SLOT_HAND 3
#define EQ_SLOT_SHOE 4
#define EQ_SLOT_WEAPON2 5
#define EQ_SLOT_NECK 6
#define EQ_SLOT_RING1 7
#define EQ_SLOT_RING2 8
#define EQ_SLOT_SPECS 9
#define EQ_SLOT_FERRY 10
#define EQ_SLOT_AMU 11
#define EQ_SLOT_SP 12
#define EQ_SLOT_KOSTUEM_CHEST 13
#define EQ_SLOT_KOSTUEM_HEAD 14
#ifndef _H_CHARACTERINVENTORY
#define _H_CHARACTERINVENTORY

//Typedefinitionen

class TGeneral;
class TCharacterInventory;
extern TCharacterInventory Inventory;

//Strukturen, Klassen
struct structEquipmentSlot
{
	int vnum;
	int rare;
	int upgrade;
	structItemStats stats;
};

struct structInventory
{
	bool inUse;
	int id;
	int count;
	int rare;
	int upgrade;
	structItemStats stats;
};

struct structTradeInventory
{
	int inv;
	int slot;
	int amount;
};

struct structCharacterInventory
{
	bool Initialized;
	bool BackpackActive;
	int gold_count;
	std::vector<std::vector<structInventory>> Index;
	std::vector<structTradeInventory> TradeInventory;
	int gold_trade;
	bool trade_ready;
	bool trade_finish;
};

//-------------------------------------------

class TCharacterInventory
{
private:
	void SetItemFromDatabase(TSocket *Client, int characterslot, int inventorytype, int slot, int id, int count, int rare, int upgrade); //Mini Wrapper
	int DetectSlot(TSocket *Client, int inventorytype, int slot); //Slot ermitteln oder manuell setzen
	int DetectInventory(TSocket *Client, int inventorytype, int itemid); //Item Inventar Type ermitteln oder manuell setzen
	int DetectStackSlot(TSocket *Client, int inventorytype, int item_vnum, int amount); //Item Stack ermitteln
public:
	TCharacterInventory(void); //Konstruktor
	~TCharacterInventory(void); //Destruktor

	bool AddItem(TSocket *Client, int inventorytype, int slot, int id, int count, int rare, int upgrade, bool ismoving, structItemStats stats); //Item im Inventar hinzufügen
	void RemoveItem(TSocket *Client, int inventorytype, int slot, int amount = -1); //Item im Inventar entfernen
	void ParseInventory(TSocket *Client, int characterslot, int inventorytype, std::string inventoryarray); // Datenbank Infos parsen
	void RemoveItemRequest(TSocket *Client, int inventorytype, int slot); //Anfrage um Item zu löschen
	void MoveItem(TSocket *Client, int inventorytype, int slot, int count, int newslot); //Item im Inventar verschieben
	void UseItem(TSocket *Client, int inventorytype, int slot); //Item im Inventar nutzen
	void Send_Equipment( TSocket *Client );
	void WearItem( TSocket *Client, int slot, int inventorytype ); //EQ Item anziehen
	void UnWearItem( TSocket *Client, int slot, int unkown ); //EQ Item ausziehen
	void DropItem(TSocket *Client, int inventory, int slot, int amount); //Item droppen
	void GetItem(TSocket *Client, int action, int charid, int itemid); //item aufheben
};

#endif _H_CHARACTERINVENTORY
