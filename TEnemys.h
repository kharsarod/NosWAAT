#pragma once

#define ENEMY_RESPAWN_TIME 10
#define ENEMY_NOT_EXIST -1

#define ERROR_ENEMYID -1

#ifndef _H_ENEMIES
#define _H_ENEMIES

class TSocket;
class TEnemies;
extern TEnemies Enemies;

struct structDroptable
{
	int vnum;
	int chance;
};

struct structDropGold
{
	int count;
	int chance;
};

struct structEnemySkillsUse
{
	int allocated_skill;
	int cast_time;
	int cooldown_time;
};

struct structEnemyAttackAttackers
{
	int character_id;
	int damage_done;
};

struct structEnemyAttack
{
	bool isattacking;
	int charid_firstattack;
	bool iscasting;
	int casting_time;
	int target;
	int skill_using;
	std::vector<structEnemyAttackAttackers> attackers;
};

struct structEnemyType
{
	int vnum;
	int hp;
	int mp;
	std::vector<structDroptable> droptable;
	std::vector<int> skills;
	structDropGold dropgold;
	int xp;
	int jxp;
};

class TEnemies
{
public:
	TEnemies(void);
	~TEnemies(void);
	
	std::vector<structEnemyType> types;

	int Enemy_Alive( int id, int map); //Schauen ob Gegner am leben
	void Enemy_Respawn( int allocated_spawn, int allocated_map); //Gegner Respawnen

	int Calc_BaseHP(int BaseHP, int Level); //Base HP auf Level umrechnen
	int Calc_BaseMP(int BaseMP, int Level); //Base MP auf Level umrechnen
	int Calculate_PathH(int var1, int var2);

	// GM Funktionen ------------
	void Enemy_Spawn( TSocket *Client, int vnum, int level, int count);
	void Kill_Range( TSocket *Client);
	//---------------------------
	// Thread Funktionen --------
	std::mutex lockthread;
	void Check_Respawn(int all_map); //Respawn Handle
	//---------------------------
	// Allgemeine Funktionen
	int Get_Spawnpoint(int mapindex, int enemyid); //Spawn des Gegners ermitteln
	void Enemy_Kill(TSocket *Client, int mapindex, int all_en); //Gegner wird gekillt
	int s_EnID(int mapindex);
	//---------------------------
	// Gegner Fähigkeiten
	void Random_Move(int all_map, int all_en); //Zufall bewegung
	void Handle_Chase(int mapindex, int all_en);
	void Handle_Attack(int mapindex, int all_en);
	void Walk_To(int mapindex, int all_en, int target_x, int target_y);

	static DWORD WINAPI EnemyThread(LPVOID lpdwThreadParam);
};

#endif _H_ENEMIES
