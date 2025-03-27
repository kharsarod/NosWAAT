#include "stdafx.h"
#include "TNPCShop.h"

TNPCShop NpcShop;

TNPCShop::TNPCShop(void)
{
}


TNPCShop::~TNPCShop(void)
{
}

//------------------------------------------------------------

void TNPCShop::OpenCustomShop( TSocket *Client, int mapindex, int npcindex )
{
	std::stringstream ss;
	int shoplistsize = Map.MapData[mapindex].Npc[npcindex].shoplist[0].list.size();
	int type;

	ss << "n_inv 2 " <<Map.MapData[mapindex].Npc[npcindex].npc_id << " 0 100";

	for(auto e = 0; e < shoplistsize; e++)
	{
		type = Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].inventorytype;

		switch (type)
		{
			case EQUIP_INVENTORY:
				ss << " " << type << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].slot << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemid <<
					"." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemrare << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemupgrade <<
					"." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemprice;
			break;
			default:
				ss << " " << type << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].slot << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemid <<
					"." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemcount << "." << Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[e].itemprice;
			break;
		}
	}

	Client->Send(ss.str());
}

//------------------------------------------------------------

void TNPCShop::BuyItem( TSocket *Client, int npc_id, int slot, int count )
{
	int MapIndex = Map.GetMapIndex(Client->Character->Map.id);
	int NpcIndex = Map.GetMapNpcIndex(MapIndex, MapIndex, npc_id);

	if(NpcIndex == NPC_NOT_FOUND || count < 1 ||count > 99)
	{
		return;
	}

	int ShopItemIndex = this->GetItemSlotIndex(Client, MapIndex, NpcIndex, slot);

	if(ShopItemIndex == INVALID_SLOT_INDEX)
	{
		return;
	}

	int price = Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].itemprice * count;

	if(price > 0 && price <= Client->Character->Inventory.gold_count)
	{
		this->ShopMemoMessage(Client, SUCCESS_MESSAGE, "You have successfully bought an item");

		Client->Character->Inventory.gold_count -= price;
		CharacterInfo.Send_CharacterGold(Client);

		Inventory.AddItem(
			Client, 
			Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].inventorytype,
			AUTODETECT_SLOT,
			Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].itemid,
			count,
			Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].itemrare,
			Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].itemupgrade,
			false,
			Map.MapData[MapIndex].Npc[NpcIndex].shoplist[0].list[ShopItemIndex].stats
		);

	} else
	{
		this->ShopMemoMessage(Client, FAILED_MESSAGE, "Sorry, not enough gold!");
	}
}

//------------------------------------------------------------

int TNPCShop::GetItemSlotIndex( TSocket *Client, int mapindex, int npcindex, int slot )
{
	int ShopListSize = Map.MapData[mapindex].Npc[npcindex].shoplist[0].list.size();

	for(auto i = 0; i < ShopListSize; i++)
	{
		if(Map.MapData[mapindex].Npc[npcindex].shoplist[0].list[i].slot == slot)
		{
			return i;
		}
	}

	return INVALID_SLOT_INDEX;
}

//------------------------------------------------------------

void TNPCShop::ShopMemoMessage( TSocket *Client, int type, std::string message )
{
	std::stringstream ss;
	ss << "s_memo " << type << " " << message;
	Client->Send(ss.str());
}

//------------------------------------------------------------

void TNPCShop::SellItem( TSocket *Client, int npc_id, int inventorytype, int slot, int count )
{
	if(count > 0 && Client->Character->Inventory.Index[inventorytype][slot].inUse == true && count <= Client->Character->Inventory.Index[inventorytype][slot].count)
	{
		int ItemIndex = Items.GetItemIndex(Client->Character->Inventory.Index[inventorytype][slot].id);
		int Price = count * Items.vItems[ItemIndex].price;

		Inventory.RemoveItem(Client, inventorytype, slot, count);
	
		this->ShopMemoMessage(Client, SUCCESS_MESSAGE, "You have successfully sold an item");
		Client->Character->Inventory.gold_count += Price;
		CharacterInfo.Send_CharacterGold(Client);
	}
}

//------------------------------------------------------------