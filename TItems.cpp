#include "stdafx.h"
#include "TItems.h"

//Typedefinitionen
TItems Items;

//------------------------------------------------------------

TItems::TItems(void)
{
}

//------------------------------------------------------------

TItems::~TItems(void)
{
}

//------------------------------------------------------------

int TItems::MakeItem( TSocket *Client, int itemid )
{
	int ItemIndex = this->GetItemIndex(itemid);
	int effectssize = this->vItems[ItemIndex].effects.size();

	if(ItemIndex == ITEM_NOT_FOUND)
	{
		return ITEM_NOT_FOUND;
	}

	for(auto e = 0; e < effectssize; e++)
	{
		Effects.ShowEffect(Client, this->vItems[ItemIndex].effects[e]);
	}

	return this->vItems[ItemIndex].on_use_delete;
}

//------------------------------------------------------------

int TItems::s_ItID(int mapindex)
{
	bool found = true;
	int id = 0;
	int itemsize = 0;
	itemsize = Map.MapData[mapindex].Item.size();

	do 
	{
		id = General.CreateRandomNumber(100, 100000);
		found = false;

		for(int i = 0; i < itemsize; i++)
		{
			if(Map.MapData[mapindex].Item[i].id == id)
			{
				found = true;
				break;
			}
		}

	} while (found);

	if(!found)
	{
		return id;
	}

	return ERROR_ENEMYID;
}

//------------------------------------------------------------

int TItems::GetItemIndex( int vnum )
{
	int ItemSize = this->vItems.size();

	for(auto i = 0; i < ItemSize; i++)
	{
		if(this->vItems[i].vnum == vnum)
		{
			return i;
		}
	}

	return ITEM_NOT_FOUND;
}

//------------------------------------------------------------

int TItems::GetEquipmentSlot( int vnum )
{
	int ItemSize = this->vItems.size();

	for(auto i = 0; i < ItemSize; i++)
	{
		if(this->vItems[i].vnum == vnum)
		{
			return this->vItems[i].equipment_slot;
		}
	}

	return ITEM_NOT_FOUND;
}

//------------------------------------------------------------

void TItems::Bound_Check(int mapindex, int itemindex)
{
	int time = GetTickCount() / 1000;

	//Check bound time;
	if(Map.MapData[mapindex].Item[itemindex].dropbound.drop_bound_is)
	{
		if(time >= Map.MapData[mapindex].Item[itemindex].dropbound.drop_bound_fadetime)
		{
			Map.MapData[mapindex].Item[itemindex].dropbound.drop_bound_is = false;
		}
	}
}

//------------------------------------------------------------

void TItems::Lifetime_Check(int mapindex, int itemindex)
{
	int time = GetTickCount() / 1000;

	if(time >= Map.MapData[mapindex].Item[itemindex].dropbound.lifetime)
	{
		//Seit X Minuten auf dem Boden - entfernen.
		std::stringstream ss;
		ss << "out " << OBJ_ITEMS << " " << Map.MapData[mapindex].Item[itemindex].id;
		Map.SendMap(mapindex, NULL, true, ss.str());

		Map.MapData[mapindex].Item.erase(Map.MapData[mapindex].Item.begin()+itemindex);
	}
}

//------------------------------------------------------------

DWORD WINAPI TItems::ItemsThread(LPVOID lpdwThreadParam)
{
	Sleep(100);

	std::cout << "TItems::ItemsThread() running" << std::endl;
	std::cout << "-----------------------------------" << std::endl;

	do 
	{
		int mapsize = Map.MapData.size();
		int itemsize;

		for(int i = 0; i < mapsize; i++)
		{
			Items.lockthread.lock();
			//Enemies.Check_Respawn(i);

			itemsize = Map.MapData[i].Item.size();

			for(int t = 0; t < itemsize; t++)
			{
				//Bound check
				Items.Bound_Check(i,t);

				//Check Lifetime
				Items.Lifetime_Check(i,t);
			}

			Items.lockthread.unlock();
		}
		Sleep(100);
	} while (1);
}
