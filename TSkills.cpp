#include "stdafx.h"
#include "TSkills.h"

//Typedefinitionen
TSkills Skills;

//------------------------------------------------------------

TSkills::TSkills(void)
{
}

//------------------------------------------------------------

TSkills::~TSkills(void)
{
}
//------------------------------------------------------------
int TSkills::Find_EnemySkill_allocated(int vnum)
{
	int skillsize = this->EnemySkills.size();
	for(int i = 0; i < skillsize; i++)
	{
		if(this->EnemySkills[i].vnum == vnum)
		{
			return i;
		}
	}

	return SKILL_NOTFOUND;
}
//------------------------------------------------------------
void TSkills::Hit_Enemy(TSocket *Client, int mapindex, int allocated_en )
{
	bool alive;	int resthp; int hitmode;

	int all_sk = Client->Character->Cast.allocated_id;
	int castid = Skills[all_sk].vnum;
	int effectid = Skills[all_sk].effect;
	int moveid = Skills[all_sk].moveid;
	int typ = Skills[all_sk].typ;


	//Set to attacking
	if(!Map.MapData[mapindex].Enemy[allocated_en].attack.isattacking)
	{
		Map.MapData[mapindex].Enemy[allocated_en].attack.isattacking = true;
		Map.MapData[mapindex].Enemy[allocated_en].attack.charid_firstattack = Client->Character->CharacterInfo.ID;
	}

	//Add to Attackers
	int attacker_size = Map.MapData[mapindex].Enemy[allocated_en].attack.attackers.size();
	bool isattacker = false;
	int attacker_allocated = -1;

	for(int i = 0; i < attacker_size; i++)
	{
		if(Map.MapData[mapindex].Enemy[allocated_en].attack.attackers[i].character_id == Client->Character->CharacterInfo.ID)
		{
			isattacker = true;
			attacker_allocated = i;
			break;
		}
	}

	//Calc Damage und sow
	int damage = General.CreateRandomNumber(1,20);
	int generated = General.CreateRandomNumber(0,100);
	int critical_chance = 10;
	int miss_chance = 10;
	int miss = 100 - miss_chance;

	hitmode = 0; //normal
	if(generated < critical_chance){ hitmode = 3; }
	if(generated > miss){ hitmode = 1; }

	std::stringstream packet;

	switch(hitmode)
	{
		case 0: // normal
			Map.MapData[mapindex].Enemy[allocated_en].cur_hp -= damage;
			break;
		case 1: //miss
			break;
		case 3: //critical
			damage = damage * 2;
			Map.MapData[mapindex].Enemy[allocated_en].cur_hp -= damage;
			break;
	}

	if(Map.MapData[mapindex].Enemy[allocated_en].cur_hp <= 0)
	{
		//Kill it with fire!!111
		alive = false;
		resthp = 0;
	}else
	{
		//Meh pussy shot!
		alive = true;
		resthp = (Map.MapData[mapindex].Enemy[allocated_en].cur_hp * 100) / Map.MapData[mapindex].Enemy[allocated_en].max_hp;
	}

	packet << "su " << OBJ_PLAYERS << " " << Client->Character->CharacterInfo.ID << " " <<  OBJ_ENEMIES << " " << Map.MapData[mapindex].Enemy[allocated_en].id << " " << castid << " 6 " << moveid << " " << effectid << " 0 0 " << alive << " " << resthp << " " << damage << " " << hitmode << " " << typ;
	Map.SendMap(mapindex, NULL, true, packet.str());

	//Add to attacker list
	if(isattacker)
	{
		Map.MapData[mapindex].Enemy[allocated_en].attack.attackers[attacker_allocated].damage_done += damage;
	}else
	{
		structEnemyAttackAttackers PlayerAttacker;
		PlayerAttacker.character_id = Client->Character->CharacterInfo.ID;
		PlayerAttacker.damage_done = damage;
		Map.MapData[mapindex].Enemy[allocated_en].attack.attackers.push_back(PlayerAttacker);
	}

	if(!alive){ Enemies.Enemy_Kill(Client, mapindex, allocated_en); }

}
//------------------------------------------------------------
void TSkills::Player_CheckCast(int mapindex, int player_all)
{
	if(Map.MapData[mapindex].Players[player_all]->Character->Cast.iscasting)
	{
		if((GetTickCount() / 100) > Map.MapData[mapindex].Players[player_all]->Character->Cast.casttime)
		{
			switch(Map.MapData[mapindex].Players[player_all]->Character->Cast.targetobj)
			{
			case OBJ_ENEMIES:
				int targetid = Map.MapData[mapindex].Players[player_all]->Character->Cast.targetid;

				//Enemies.lockthread.lock();
				int enemyalive = Enemies.Enemy_Alive(targetid, mapindex);

				if(enemyalive != ENEMY_NOT_EXIST)
				{
					int distance = Math.Distance(Map.MapData[mapindex].Players[player_all]->Character->Map.x, Map.MapData[mapindex].Enemy[enemyalive].x, Map.MapData[mapindex].Players[player_all]->Character->Map.y, Map.MapData[mapindex].Enemy[enemyalive].y);
					if(distance <= Skills[Map.MapData[mapindex].Players[player_all]->Character->Cast.allocated_id].range)
					{
						this->Hit_Enemy(Map.MapData[mapindex].Players[player_all], mapindex, enemyalive);
					}else
					{
						std::string message = "cancel 0 0";
						Map.MapData[mapindex].Players[player_all]->Send(message);
					}
				}

				//Enemies.lockthread.unlock();
				int all_cast = Map.MapData[mapindex].Players[player_all]->Character->Cast.allocated_cast;
				Map.MapData[mapindex].Players[player_all]->Character->CharacterSkills[all_cast].cooldown = (GetTickCount() / 100) + Skills[Map.MapData[mapindex].Players[player_all]->Character->Cast.allocated_id].cooldown;
				Map.MapData[mapindex].Players[player_all]->Character->Cast.iscasting = false;

				break;
			}
		}
	}
}

//------------------------------------------------------------

void TSkills::Player_Cast(TSocket *Client, int castid, int targetobj, int targetid)
{
	if(!Client->Character->Cast.iscasting)
	{
		//Schauen ob Skill vorhanden
		int skillsize = Client->Character->CharacterSkills.size();
		int totalskillsize = this->Skills.size();

		for(int i = 0; i < skillsize; i++)
		{
			if(Client->Character->CharacterSkills[i].castid == castid)
			{
				//Cooldown check
				if((GetTickCount() / 100) > Client->Character->CharacterSkills[i].cooldown)
				{
					//Hole Skilldaten
					for(int t = 0; t < totalskillsize; t++)
					{
						if(this->Skills[t].vnum == Client->Character->CharacterSkills[i].vnum)
						{
							Client->Character->Cast.iscasting = true;
							Client->Character->Cast.allocated_id = t;
							Client->Character->Cast.allocated_cast = i;
							Client->Character->Cast.targetid = targetid;
							Client->Character->Cast.targetobj = targetobj;
							Client->Character->Cast.casttime = (GetTickCount() / 100) + this->Skills[t].casttime;
							return;
						}
					}
				}
			}
		}

		//Irgendwas ging schief
		//Client->Disconnect();
	}
}
