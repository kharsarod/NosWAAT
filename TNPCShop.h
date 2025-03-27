#pragma once
#define INVALID_SLOT_INDEX -1
#define FAILED_MESSAGE 2
#define SUCCESS_MESSAGE 1
#define NORMAL_MESSAGE 5

class TNPCShop;
extern TNPCShop NpcShop;

#ifndef _H_NPCSHOP
#define _H_NPCSHOP

struct structShopListData
{
	int slot;
	int inventorytype;
	int itemid;
	int itemcount;
	int itemrare;
	int itemupgrade;
	int itemprice;
	structItemStats stats;
};

struct structShopList
{
	int list_id;
	std::vector<structShopListData> list;
};

class TNPCShop
{
private:
	
public:
	std::vector<structShopList> ShopList;

	TNPCShop(void);
	~TNPCShop(void);

	void OpenCustomShop(TSocket *Client, int mapindex, int npcindex); //Shop öffnen
	void BuyItem(TSocket *Client, int npc_id, int slot, int count); //Item kaufen
	void SellItem(TSocket *Client, int npc_id, int inventorytype, int slot, int count); //Item verkaufen
	int GetItemSlotIndex(TSocket *Client, int mapindex, int npcindex, int slot); //Shop Item Index returnen
	void ShopMemoMessage(TSocket *Client, int type, std::string message);
};

#endif _H_NPCSHOP