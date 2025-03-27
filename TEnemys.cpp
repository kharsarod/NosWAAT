#include "stdafx.h"
#include "TEnemys.h"

TEnemies Enemies;

TEnemies::TEnemies(void)
{
}


TEnemies::~TEnemies(void)
{
}

int TEnemies::Calc_BaseHP(int BaseHP, int Level)
{
	return (BaseHP * Level);
}

int TEnemies::Calc_BaseMP(int BaseMP, int Level)
{
	return (BaseMP * Level);
}

int TEnemies::Get_Spawnpoint(int mapindex, int enemyid)
{
	int spawnsize = Map.MapData[mapindex].EnemySpawns.size();

	for(int i = 0; i < spawnsize; i++)
	{
		if(Map.MapData[mapindex].EnemySpawns[i].active_enemy_id == enemyid)
		{
			return i;
		}
	}

	return -1;
}

void TEnemies::Enemy_Kill(TSocket *Client, int mapindex, int all_en)
{
	int enemyid = Map.MapData[mapindex].Enemy[all_en].id;
	int allocated_spawn = this->Get_Spawnpoint(mapindex, enemyid);

	if(allocated_spawn >= 0)
	{
		Map.MapData[mapindex].EnemySpawns[allocated_spawn].next_respawn_time = (GetTickCount() / 1000) + (ENEMY_RESPAWN_TIME);
	}

	//Drop Item
	int droptablesize = Map.MapData[mapindex].Enemy[all_en].droptable.size();
	int vnum, itemchance, generated, goldcount;

	for(int i = 0; i < droptablesize; i++)
	{
		vnum = Map.MapData[mapindex].Enemy[all_en].droptable[i].vnum;
		itemchance =  Map.MapData[mapindex].Enemy[all_en].droptable[i].chance * Config.Rate_Drop;
		generated = General.CreateRandomNumber(0,100);

		if(generated <= itemchance)
		{
			//drop
			int x = General.CreateRandomNumber(Map.MapData[mapindex].Enemy[all_en].x - 1, Map.MapData[mapindex].Enemy[all_en].x + 1);
			int y = General.CreateRandomNumber(Map.MapData[mapindex].Enemy[all_en].y - 1, Map.MapData[mapindex].Enemy[all_en].y + 1);
			int count = 1;
			int quest = 0;
			int bound = 0;
			int itemid = Items.s_ItID(mapindex);
			int rare = 0;
			int upgrade = 0;

			structMapItem MapItem;
			MapItem.enemy_drop = true;
			MapItem.id = itemid;
			MapItem.vnum = vnum;
			MapItem.amount = count;
			MapItem.x = x;
			MapItem.y = y;
			MapItem.quest = quest;
			MapItem.bound_id = bound;
			MapItem.rare = rare;
			MapItem.upgrade = upgrade;
			MapItem.dropbound.drop_bound_charid = Map.MapData[mapindex].Enemy[all_en].attack.charid_firstattack;
			MapItem.dropbound.drop_bound_is = true;
			MapItem.dropbound.drop_bound_fadetime = (GetTickCount() / 1000) + ITEM_DROPBOUND_TIME;
			MapItem.dropbound.lifetime = (GetTickCount() / 1000) + ITEM_LIFE_TIME;

			//Stats
			int itemindex = Items.GetItemIndex(MapItem.vnum);
			MapItem.stats = Items.vItems[itemindex].stats;

			Items.lockthread.lock();
			Map.MapData[mapindex].Item.push_back(MapItem);
			Items.lockthread.unlock();

			std::stringstream message;
			message << "drop " << vnum << " " << itemid << " " << x << " " << y << " " << count << " " << quest << " " << bound;

			Map.SendMap(mapindex, NULL, true, message.str());
		}
	}

	//Drop Gold
	vnum = SITEM_GOLD;
	itemchance =  Map.MapData[mapindex].Enemy[all_en].dropgold.chance * Config.Rate_Gold;
	goldcount = Map.MapData[mapindex].Enemy[all_en].dropgold.count * Config.Rate_Gold;
	int goldpercent = Map.MapData[mapindex].Enemy[all_en].dropgold.count / 100 * 10;
	goldcount = General.CreateRandomNumber(goldcount - goldpercent,goldcount + goldpercent);
	generated = General.CreateRandomNumber(0,100);

	if(generated <= itemchance)
	{
		//drop
		int x = General.CreateRandomNumber(Map.MapData[mapindex].Enemy[all_en].x - 1, Map.MapData[mapindex].Enemy[all_en].x + 1);
		int y = General.CreateRandomNumber(Map.MapData[mapindex].Enemy[all_en].y - 1, Map.MapData[mapindex].Enemy[all_en].y + 1);
		int count = goldcount;
		int quest = 0;
		int bound = 0;
		int itemid = Items.s_ItID(mapindex);
		int rare = 0;
		int upgrade = 0;

		structMapItem MapItem;
		MapItem.id = itemid;
		MapItem.vnum = vnum;
		MapItem.amount = count;
		MapItem.x = x;
		MapItem.y = y;
		MapItem.quest = quest;
		MapItem.bound_id = bound;
		MapItem.rare = rare;
		MapItem.upgrade = upgrade;
		MapItem.dropbound.drop_bound_charid = Map.MapData[mapindex].Enemy[all_en].attack.charid_firstattack;
		MapItem.dropbound.drop_bound_is = true;
		MapItem.dropbound.drop_bound_fadetime = (GetTickCount() / 1000) + ITEM_DROPBOUND_TIME;
		MapItem.dropbound.lifetime = (GetTickCount() / 1000) + ITEM_LIFE_TIME;

		Items.lockthread.lock();
		Map.MapData[mapindex].Item.push_back(MapItem);
		Items.lockthread.unlock();

		std::stringstream message;
		message << "drop " << vnum << " " << itemid << " " << x << " " << y << " " << count << " " << quest << " " << bound;

		Map.SendMap(mapindex, NULL, true, message.str());
	}

	//Give EXP
	if(!Client->Character->Group.ingroup)
	{
		CharacterInfo.XP_Add(Client, Map.MapData[mapindex].Enemy[all_en].xp, Map.MapData[mapindex].Enemy[all_en].jxp, 0);
	}else
	{
		//IN Gruppe
		int groupindex = Group.Group_GetByID(Client->Character->Group.groupid);
		int membersize = Group.groups[groupindex].members.size();
		int mapid = Client->Character->Map.id;
		int xpsplitcount = 0;
		for(int i = 0; i < membersize; i++)
		{
			if(Group.groups[groupindex].members[i].client->Character->Map.id == mapid)
			{
				xpsplitcount++;
			}
		}

		int xppchar = Map.MapData[mapindex].Enemy[all_en].xp / xpsplitcount;
		xppchar = xppchar + (xppchar * GRP_XP_BONUS); //Gruppenbonus

		int jxppchar = Map.MapData[mapindex].Enemy[all_en].jxp / xpsplitcount;
		jxppchar = jxppchar + (jxppchar * GRP_XP_BONUS); //Gruppenbonus

		for(int t = 0; t < membersize; t++)
		{
			if(Group.groups[groupindex].members[t].client->Character->Map.id == mapid)
			{
				CharacterInfo.XP_Add(Group.groups[groupindex].members[t].client, xppchar, jxppchar, 0);
			}
		}
	}

	Enemies.lockthread.lock();
	Map.MapData[mapindex].Enemy.erase(Map.MapData[mapindex].Enemy.begin() + all_en);
	Enemies.lockthread.unlock();
}

int TEnemies::Calculate_PathH(int var1, int var2)
{
	if(var1 > var2)
	{
		return ( var1 - var2 );
	}else
	{
		return ( var2 - var1 );
	}
}

void TEnemies::Walk_To(int mapindex, int all_en, int target_x, int target_y)
{
	Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.clear();

	//std::cout << "Generating Path for " << Map.MapData[mapindex].Enemy[all_en].id << ": ";
	int start_x = Map.MapData[mapindex].Enemy[all_en].x;
	int start_y = Map.MapData[mapindex].Enemy[all_en].y;
	//std::cout << "Cur X: " << start_x << " Cur Y: " << start_y << " ";

	int shortest_distance = 99999;
	int temp_x, temp_y, next_x, next_y;
	bool found;

	struct pathfindpoint
	{
		int x;
		int y;
		int F;
		int G;
		int H;
		int parent_x;
		int parent_y;
	};

	std::vector<pathfindpoint> openList;
	std::vector<pathfindpoint> closedList;
	pathfindpoint waypoint;
	pathfindpoint current;

	//Start Punkt
	waypoint.x = start_x;
	waypoint.y = start_y;
	waypoint.F = 0;
	waypoint.G = 0;
	waypoint.H = 0;
	openList.push_back(waypoint);
	//---------------------------
	int openListsize, closedListSize;
	int lowestF = 99999, lowestFall;
	bool target_found = false;

	do 
	{
		//niedrigste F Wert holen und auf closed list;
		openListsize = openList.size();
		lowestF = 99999;
		for(int i = 0; i < openListsize; i++)
		{
			if(openList[i].F < lowestF)
			{
				lowestF = openList[i].F;
				lowestFall = i;
			}
		}

		current = openList[lowestFall];
		closedList.push_back(openList[lowestFall]);
		openList.erase(openList.begin() + lowestFall);
		if(current.x == target_x && current.y == target_y)
		{
			target_found = true;
			//std::cout << "TARGET FOUND " << closedList.size() << std::endl;
			break;
		}
		//--------------------------------------------
		//8 Kästchen ermitteln
		//--------------------------------------------
		bool isblocked, isOnClosedList, isOnOpenList;
		int temp_x, temp_y, openListAllocated;
		//Oben ---------------------------------------
		temp_x = current.x; temp_y = current.y + 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 10;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Oben Rechts ---------------------------------------
		temp_x = current.x + 1; temp_y = current.y + 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 14;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Oben Links ---------------------------------------
		temp_x = current.x - 1; temp_y = current.y + 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 14;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Links ---------------------------------------
		temp_x = current.x - 1; temp_y = current.y;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 10;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Rechts ---------------------------------------
		temp_x = current.x + 1; temp_y = current.y;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 10;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Unten ---------------------------------------
		temp_x = current.x; temp_y = current.y - 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 10;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Unten Links ---------------------------------------
		temp_x = current.x - 1; temp_y = current.y - 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 14;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
		//Unten Rechts ---------------------------------------
		temp_x = current.x + 1; temp_y = current.y - 1;

		isblocked = Map.isBlockedZone(mapindex, temp_x, temp_y);
		if(!isblocked)
		{
			closedListSize = closedList.size();
			isOnClosedList = false;
			for(int t = 0; t < closedListSize; t++)
			{
				if(closedList[t].x == temp_x && closedList[t].y == temp_y)
				{
					isOnClosedList = true;
					break;
				}
			}

			if(!isOnClosedList)
			{
				isOnOpenList = false;
				openListsize = openList.size();
				for(int t = 0; t < openListsize; t++)
				{
					if(openList[t].x == temp_x && openList[t].y == temp_y)
					{
						isOnOpenList = true;
						openListAllocated = t;
						break;
					}
				}

				if(!isOnOpenList)
				{
					waypoint.x = temp_x;
					waypoint.y = temp_y;
					waypoint.parent_x = current.x;
					waypoint.parent_y = current.y;
					waypoint.G = 14;
					waypoint.H = this->Calculate_PathH(temp_x, target_x) + this->Calculate_PathH(temp_y, target_y);
					waypoint.F = waypoint.G + waypoint.H;
					openList.push_back(waypoint);
				}else
				{
					if(openList[openListAllocated].G < current.G)
					{
						openList[openListAllocated].parent_x = current.x;
						openList[openListAllocated].parent_y = current.y;
					}
				}
			}
		}
	} while (1);

	//Path Berechnung
	int closeSize = closedList.size();
	int currentallocated = closeSize - 1;
	structWaypoints PathWP;
	std::vector<structWaypoints> FinalPath;
	int parent_x, parent_y;

	for(int y = 0; y < closeSize; y++)
	{
		PathWP.x = closedList[currentallocated].x;
		PathWP.y = closedList[currentallocated].y;
		FinalPath.insert(FinalPath.begin(), PathWP);

		parent_x = closedList[currentallocated].parent_x;
		parent_y = closedList[currentallocated].parent_y;

		if(parent_x == start_x && parent_y == start_y)
		{
			break;
		}

		for(int o = 0; o < closeSize; o++)
		{
			if(closedList[o].x == parent_x && closedList[o].y == parent_y)
			{
				currentallocated = o;
				break;
			}
		}
	}

	int pathsize = FinalPath.size();
	//std::cout << "FinalPath Size: " << pathsize << std::endl;

	//for(int pathi = 0; pathi < pathsize; pathi++ )
	//{
	//	std::stringstream message;
	//	message << "drop " << 500 << " " << General.CreateRandomNumber(0, 10000) << " " << FinalPath[pathi].x << " " << FinalPath[pathi].y << " " << 1 << " " << 0 << " " << 0;

	//	Map.SendMap(mapindex, NULL, true, message.str());
	//}

	// std::cout << std::endl;
	Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints = FinalPath;
	Map.MapData[mapindex].Enemy[all_en].pathfinding.chasing = true;
	Map.MapData[mapindex].Enemy[all_en].pathfinding.started = false;
	Map.MapData[mapindex].Enemy[all_en].pathfinding.target_x = target_x;
	Map.MapData[mapindex].Enemy[all_en].pathfinding.target_y = target_y;
}

void TEnemies::Kill_Range(TSocket *Client)
{
	//su 1 554897 3 1507 200 6 11 300 0 0 0 29 40000 0 0
	int mapindex = Map.GetMapIndex(Client->Character->Map.id);
	int enemysize = Map.MapData[mapindex].Enemy.size();

	for(int i = 0; i < enemysize;)
	{
		int distance = Math.Distance(Client->Character->Map.x, Map.MapData[mapindex].Enemy[i].x,Client->Character->Map.y, Map.MapData[mapindex].Enemy[i].y);
		if(distance < 10)
		{
			this->Enemy_Kill(Client, mapindex, i);
			enemysize -= 1;
		}else
		{
			i++;
		}
	}
}

void TEnemies::Random_Move(int all_map, int all_en)
{
	//if not attacking
	if(!Map.MapData[all_map].Enemy[all_en].pathfinding.chasing)
	{
		int time = GetTickCount() / 10;
		if(time > Map.MapData[all_map].Enemy[all_en].random_move_next)
		{
			 Map.MapData[all_map].Enemy[all_en].random_move_next = time + General.CreateRandomNumber(100,1000);
			 //Generate Coords
			 bool notwalkable = true;
			 int gen_x, gen_y;
			 int tries = 0;

			 do 
			 {
				 gen_x = Map.MapData[all_map].Enemy[all_en].x;
				 gen_y = Map.MapData[all_map].Enemy[all_en].y;

				 int x_min = General.CreateRandomNumber(0,10);
				 int y_min = General.CreateRandomNumber(0,10);

				 if(x_min > 6){ gen_x -= 2; } else if(x_min < 4) { gen_x += 2; }
				 if(y_min > 6){ gen_y -= 2; } else if(y_min < 4) { gen_y += 2; }

				 notwalkable = Map.isBlockedZone(all_map, gen_x, gen_y);

				 if(Math.Distance(gen_x, Map.MapData[all_map].Enemy[all_en].start_x, gen_y, Map.MapData[all_map].Enemy[all_en].start_y) > 8)
				 {
					notwalkable = true;
				 }

				 tries++;
			 } while (notwalkable || tries < 5);

			 if(!notwalkable)
			 {
				 this->Walk_To(all_map, all_en, gen_x, gen_y);
			 }
		}
	}
}
	
int TEnemies::s_EnID(int mapindex)
{
	bool found = true;
	int id = 0;
	int enemysize = 0;
	enemysize = Map.MapData[mapindex].Enemy.size();

	do 
	{
		id = General.CreateRandomNumber(100, 100000);
		found = false;

		for(int i = 0; i < enemysize; i++)
		{
			if(Map.MapData[mapindex].Enemy[i].id == id)
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

void TEnemies::Enemy_Spawn( TSocket *Client, int vnum, int level, int count)
{
	int mapindex = Map.GetMapIndex(Client->Character->Map.id);
	
	this->lockthread.lock();
	for(int counter = 0; counter < count; counter++)
	{
		structMapEnemy enemy;
	
		enemy.id = s_EnID(mapindex);
		enemy.vnum = vnum;
		enemy.x = Client->Character->Map.x;
		enemy.y = Client->Character->Map.y;
		enemy.start_x = enemy.x;
		enemy.start_y = enemy.y;
		enemy.level = level;

		enemy.random_move_next = 0;
		enemy.walking = false;
		enemy.attack.isattacking = false;
		enemy.pathfinding.chasing = false;

		int typesize = this->types.size();

		int max_hp, max_mp;
		bool found = false;

		for(int t = 0; t < typesize; t++)
		{
			if(this->types[t].vnum == enemy.vnum)
			{
				enemy.max_hp = this->Calc_BaseHP(this->types[t].hp, enemy.level);
				enemy.max_mp = this->Calc_BaseMP(this->types[t].mp, enemy.level);
				enemy.cur_hp = enemy.max_hp;
				enemy.cur_mp = enemy.max_mp;
				enemy.droptable = this->types[t].droptable;
				enemy.dropgold = this->types[t].dropgold;
				enemy.xp = this->types[t].xp * enemy.level;
				enemy.jxp = this->types[t].jxp * enemy.level;
				found = true;
				break;
			}
		}

		if(!found)
		{
			std::cout << "SPAWN ERROR: TYPE " << enemy.vnum << " NOT FOUND!" << std::endl;
			enemy.max_hp = this->Calc_BaseHP(100, enemy.level);
			enemy.max_mp = this->Calc_BaseMP(100, enemy.level);
			enemy.cur_hp = enemy.max_hp;
			enemy.cur_mp = enemy.max_mp;
			enemy.xp = 1;
			enemy.jxp = 1;
		}

		Map.MapData[mapindex].Enemy.push_back(enemy);

		//Send to all players
		std::stringstream packet;
		packet << "in " << OBJ_ENEMIES << " " << enemy.vnum << " " << enemy.id << " " << enemy.x << " " << enemy.y << " 0 100 100 0 0 0 -1 0 0 -1 -";

		int playercount = Map.MapData[mapindex].Players.size();

		for(int i = 0; i < playercount; i++)
		{
			Map.MapData[mapindex].Players[i]->Send(packet.str());
		}
	}
	this->lockthread.unlock();
}


void TEnemies::Enemy_Respawn( int allocated_spawn, int allocated_map)
{
	structMapEnemy enemy;

	enemy.id = s_EnID(allocated_map);
	enemy.vnum = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].vnum;
	enemy.x = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].x;
	enemy.y = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].y;
	enemy.start_x = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].x;
	enemy.start_y = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].y;
	int minlevel = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].level_min;
	int maxlevel = Map.MapData[allocated_map].EnemySpawns[allocated_spawn].level_max;
	enemy.level = General.CreateRandomNumber(minlevel, maxlevel);

	enemy.random_move_next = 0;
	enemy.walking = false;
	enemy.attack.isattacking = false;
	enemy.pathfinding.chasing = false;
	enemy.attack.iscasting = false;

	int typesize = this->types.size();

	int max_hp, max_mp;
	bool found = false;

	for(int t = 0; t < typesize; t++)
	{
		if(this->types[t].vnum == enemy.vnum)
		{
			enemy.max_hp = this->Calc_BaseHP(this->types[t].hp, enemy.level);
			enemy.max_mp = this->Calc_BaseMP(this->types[t].mp, enemy.level);
			enemy.cur_hp = enemy.max_hp;
			enemy.cur_mp = enemy.max_mp;
			enemy.droptable = this->types[t].droptable;
			enemy.dropgold = this->types[t].dropgold;
			enemy.xp = this->types[t].xp * enemy.level;
			enemy.jxp = this->types[t].jxp * enemy.level;

			//Skills
			int skillsize = this->types[t].skills.size();

			structEnemySkillsUse Skill;
			for(int e = 0; e < skillsize; e++)
			{
				int skillvnum = this->types[t].skills[e];
				int allocated = Skills.Find_EnemySkill_allocated(skillvnum);
				if(allocated != SKILL_NOTFOUND)
				{
					Skill.allocated_skill = allocated;
					Skill.cooldown_time = 0;
					enemy.skills.push_back(Skill);
				}
			}

			found = true;
			break;
		}
	}

	if(!found)
	{
		std::cout << "SPAWN ERROR: TYPE " << enemy.vnum << " NOT FOUND!" << std::endl;
		enemy.max_hp = this->Calc_BaseHP(100, enemy.level);
		enemy.max_mp = this->Calc_BaseMP(100, enemy.level);
		enemy.cur_hp = enemy.max_hp;
		enemy.cur_mp = enemy.max_mp;
		enemy.xp = 1;
		enemy.jxp = 1;
	}


	Map.MapData[allocated_map].EnemySpawns[allocated_spawn].active_enemy_id = enemy.id;
	
	Map.MapData[allocated_map].Enemy.push_back(enemy);

	//Send to all players
	std::stringstream packet;
	packet << "in " << OBJ_ENEMIES << " " << enemy.vnum << " " << enemy.id << " " << enemy.x << " " << enemy.y << " 0 100 100 0 0 0 -1 0 0 -1 -";

	int playercount = Map.MapData[allocated_map].Players.size();

	for(int i = 0; i < playercount; i++)
	{
		Map.MapData[allocated_map].Players[i]->Send(packet.str());
	}
}

int TEnemies::Enemy_Alive( int id, int allocated_map)
{
	int enemysize = Map.MapData[allocated_map].Enemy.size();

	for(int i = 0; i < enemysize; i++)
	{
		if(Map.MapData[allocated_map].Enemy[i].id == id)
		{
			return i;
		}
	}

	return ENEMY_NOT_EXIST;
}

void TEnemies::Check_Respawn(int all_map)
{
	int enemyspawnsize;
	int enemy_id;
	int en_alive;

	enemyspawnsize = Map.MapData[all_map].EnemySpawns.size();
	for(int t = 0; t < enemyspawnsize; t++)
	{
		enemy_id = Map.MapData[all_map].EnemySpawns[t].active_enemy_id;
		en_alive = this->Enemy_Alive(enemy_id, all_map);

		//Respawn
		if(en_alive == ENEMY_NOT_EXIST)
		{
			int respawntime = Map.MapData[all_map].EnemySpawns[t].next_respawn_time;
			int time = GetTickCount() / 1000;

			if(time > respawntime)
			{
				this->Enemy_Respawn(t, all_map);
			}
		}
	}
}

void TEnemies::Handle_Chase(int mapindex, int all_en)
{
	int speed = 11;
	float walkspeed = BASE_WALKSPEED_MS / speed;
	int kaestchen_laufen;

	int waypoints_size = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.size();
	if(waypoints_size > 0)
	{
		if(!Map.MapData[mapindex].Enemy[all_en].pathfinding.started)
		{
			if(waypoints_size >= 3) kaestchen_laufen = 3;
			if(waypoints_size == 2) kaestchen_laufen = 2;
			if(waypoints_size == 1) kaestchen_laufen = 1;

			//Noch nicht gestartet
			Map.MapData[mapindex].Enemy[all_en].pathfinding.started = true;
			Map.MapData[mapindex].Enemy[all_en].pathfinding.nextposition_timer = (GetTickCount() / 100) + ( walkspeed * kaestchen_laufen);
			int first_x = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[kaestchen_laufen - 1].x;
			int first_y = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[kaestchen_laufen - 1].y;

			if(kaestchen_laufen >= 2)
			{
				for(int i = 0; i < kaestchen_laufen - 1; i++)
				{
					Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.erase(Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.begin() + 0);
				}
			}

			std::stringstream movepacket;
			movepacket << "mv " << OBJ_ENEMIES << " " << Map.MapData[mapindex].Enemy[all_en].id << " " << first_x << " " << first_y << " " << speed;
			Map.SendMap(mapindex, NULL, true, movepacket.str());
		}else
		{
			//Am laufen
			//Angekommen
			if((GetTickCount() / 100) > Map.MapData[mapindex].Enemy[all_en].pathfinding.nextposition_timer)
			{
				Map.MapData[mapindex].Enemy[all_en].x = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[0].x;
				Map.MapData[mapindex].Enemy[all_en].y = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[0].y;
				Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.erase(Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.begin());
				
				//Checken ob es noch einen Waypoint gibt
				waypoints_size = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.size();
				if(waypoints_size > 0)
				{
					if(waypoints_size >= 3) kaestchen_laufen = 3;
					if(waypoints_size == 2) kaestchen_laufen = 2;
					if(waypoints_size == 1) kaestchen_laufen = 1;

					Map.MapData[mapindex].Enemy[all_en].pathfinding.nextposition_timer = (GetTickCount() / 100) + (walkspeed * kaestchen_laufen);
					int first_x = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[kaestchen_laufen - 1].x;
					int first_y = Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints[kaestchen_laufen - 1].y;

					std::stringstream movepacket;
					movepacket << "mv " << OBJ_ENEMIES << " " << Map.MapData[mapindex].Enemy[all_en].id << " " << first_x << " " << first_y << " " << speed;
					Map.SendMap(mapindex, NULL, true, movepacket.str());

					if(kaestchen_laufen >= 2)
					{
						for(int i = 0; i < kaestchen_laufen - 1; i++)
						{
							Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.erase(Map.MapData[mapindex].Enemy[all_en].pathfinding.waypoints.begin() + 0);
						}
					}

				}else
				{
					Map.MapData[mapindex].Enemy[all_en].pathfinding.chasing = false;
				}
			}
		}
	}else
	{
		Map.MapData[mapindex].Enemy[all_en].pathfinding.chasing = false;
	}
}

void TEnemies::Handle_Attack(int mapindex, int all_en)
{
	if(!Map.MapData[mapindex].Enemy[all_en].attack.iscasting)
	{
		//Noch nicht am casten - cast starten

		//Ziel suchen
		int attacker_size = Map.MapData[mapindex].Enemy[all_en].attack.attackers.size();
		int current_max_aggro = -1;
		int Allocated_Target = -1;
		bool target_aquired = false;

		for(int i = 0; i < attacker_size; i++)
		{
			if(Map.MapData[mapindex].Enemy[all_en].attack.attackers[i].damage_done > current_max_aggro)
			{
				current_max_aggro = Map.MapData[mapindex].Enemy[all_en].attack.attackers[i].damage_done;
				Map.MapData[mapindex].Enemy[all_en].attack.target = Map.MapData[mapindex].Enemy[all_en].attack.attackers[i].character_id;

				//Schauen ob Player noch auf der Map
				Allocated_Target = Map.GetPlayerAllocatedByID(mapindex, Map.MapData[mapindex].Enemy[all_en].attack.target);
				if(Allocated_Target == PLAYER_NOT_FOUND)
				{
					Map.MapData[mapindex].Enemy[all_en].attack.attackers.erase(Map.MapData[mapindex].Enemy[all_en].attack.attackers.begin() + i);
					i = 0;
					attacker_size = Map.MapData[mapindex].Enemy[all_en].attack.attackers.size();
					if(attacker_size == 0)
					{
						Map.MapData[mapindex].Enemy[all_en].attack.isattacking = false;
						Map.MapData[mapindex].Enemy[all_en].cur_hp = Map.MapData[mapindex].Enemy[all_en].max_hp;
						Map.MapData[mapindex].Enemy[all_en].cur_mp = Map.MapData[mapindex].Enemy[all_en].max_mp;
						int start_x = Map.MapData[mapindex].Enemy[all_en].start_x;
						int start_y = Map.MapData[mapindex].Enemy[all_en].start_y;
						this->Walk_To(mapindex, all_en, start_x, start_y);
						return;
					}
				}else
				{
					target_aquired = true;
				}

			}
		}

		//int Allocated_Target = Map.GetPlayerAllocatedByID(mapindex, Map.MapData[mapindex].Enemy[all_en].attack.target);
		if(target_aquired)
		{

			int target_x = Map.MapData[mapindex].Players[Allocated_Target]->Character->Map.x;
			int target_y = Map.MapData[mapindex].Players[Allocated_Target]->Character->Map.y;
			int myself_x = Map.MapData[mapindex].Enemy[all_en].x;
			int myself_y = Map.MapData[mapindex].Enemy[all_en].y;
			int distance = Math.Distance(target_x, myself_x, target_y, myself_y);
			int castsize = Map.MapData[mapindex].Enemy[all_en].skills.size();
			bool skill_available = false;
			bool but_outofrange = false;

			for(int i = 0; i < castsize; i++)
			{
				if((GetTickCount() / 100) > Map.MapData[mapindex].Enemy[all_en].skills[i].cooldown_time)
				{
					int all_skill = Map.MapData[mapindex].Enemy[all_en].skills[i].allocated_skill;
					int range = Skills.EnemySkills[all_skill].range;
					int mpcost = Skills.EnemySkills[all_skill].mpcost;
					if(Map.MapData[mapindex].Enemy[all_en].cur_mp >= mpcost)
					{
						if(range >= distance)
						{

							Map.MapData[mapindex].Enemy[all_en].attack.iscasting = true;
							Map.MapData[mapindex].Enemy[all_en].attack.casting_time = (GetTickCount() / 100) + Skills.EnemySkills[all_skill].casttime;
							Map.MapData[mapindex].Enemy[all_en].attack.skill_using = i;
							skill_available = true;
							but_outofrange = false;
							break;
						}else
						{
							skill_available = true;
							but_outofrange = true;
						}
					}
				}
			}

			//Out of range, hinlaufen
			if(skill_available && but_outofrange)
			{
				this->Walk_To(mapindex, all_en, target_x, target_y);
			}
		}

	}else
	{
		//Am Casten - cast überprüfen
		if((GetTickCount() / 100) > Map.MapData[mapindex].Enemy[all_en].attack.casting_time)
		{
			//Range überprüfen
			//----------------
			int Allocated_Target = Map.GetPlayerAllocatedByID(mapindex, Map.MapData[mapindex].Enemy[all_en].attack.target);
			int allocated_used_skill = Map.MapData[mapindex].Enemy[all_en].attack.skill_using;
			int allocated_skill_list = Map.MapData[mapindex].Enemy[all_en].skills[allocated_used_skill].allocated_skill;

			//Cooldown setzen
			int skill_cooldown = Skills.EnemySkills[allocated_skill_list].cooldown;
			Map.MapData[mapindex].Enemy[all_en].skills[allocated_used_skill].cooldown_time = (GetTickCount() / 100) + skill_cooldown;

			//Damage
			int damage = General.CreateRandomNumber(0,10);
			Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.currenthp -= damage;

			//Tot
			bool alive = true;
			if(Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.currenthp <= 0)
			{
				Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.currenthp = 0;
				alive = false;
			}

			//MP reduzieren
			int mpcost = Skills.EnemySkills[allocated_skill_list].mpcost;
			Map.MapData[mapindex].Enemy[all_en].cur_mp -= mpcost;


			//Schaden senden
			int castid = Skills.EnemySkills[allocated_skill_list].vnum;
			int effect = Skills.EnemySkills[allocated_skill_list].effect;
			int moveid = Skills.EnemySkills[allocated_skill_list].moveid;

			int resthp = ((Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.currenthp *100) / Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.maxhp);
			int hitmode = 0;
			int typ = 0;

			std::stringstream packet;
			packet << "su " << OBJ_ENEMIES << " " << Map.MapData[mapindex].Enemy[all_en].id << " " <<  OBJ_PLAYERS << " " << Map.MapData[mapindex].Players[Allocated_Target]->Character->CharacterInfo.ID << " " << castid << " 6 " << moveid << " " << effect << " 0 0 " << alive << " " << resthp << " " << damage << " " << hitmode << " " << typ;
			Map.SendMap(mapindex, NULL, true, packet.str());
			CharacterInfo.Send_CharacterStats(Map.MapData[mapindex].Players[Allocated_Target]);

			//Tod Setzen
			if(!alive)
			{
				Map.MapData[mapindex].Players[Allocated_Target]->Character->Status.alive = false;
				Map.MapData[mapindex].Players[Allocated_Target]->Send("TOT"); // TODO
			}

			//Casting false
			Map.MapData[mapindex].Enemy[all_en].attack.iscasting = false;
		}
	}
}

DWORD WINAPI TEnemies::EnemyThread(LPVOID lpdwThreadParam)
{
	Sleep(100);

	std::cout << "TEnemies::EnemyThread() running" << std::endl;

	do 
	{
		int mapsize = Map.MapData.size();
		int enemysize;

		for(int i = 0; i < mapsize; i++)
		{
			Enemies.lockthread.lock();

			Enemies.Check_Respawn(i);

			enemysize = Map.MapData[i].Enemy.size();

			for(int t = 0; t < enemysize; t++)
			{
				if(!Map.MapData[i].Enemy[t].attack.isattacking && !Map.MapData[i].Enemy[t].pathfinding.chasing)
				{
					Enemies.Random_Move(i,t);
				}

				if(Map.MapData[i].Enemy[t].pathfinding.chasing)
				{
					Enemies.Handle_Chase(i, t);
				}

				if(!Map.MapData[i].Enemy[t].pathfinding.chasing && Map.MapData[i].Enemy[t].attack.isattacking )
				{
					Enemies.Handle_Attack(i, t);
				}
			}

			Enemies.lockthread.unlock();
		}

		Sleep(9);
	} while (1);
}