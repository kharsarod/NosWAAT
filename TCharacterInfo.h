#pragma once

#define QL_TYPE_ITEM 0
#define QL_TYPE_SKILL 1
#define QL_TYPE_NONE 7
#define QL_MAINTYPE_NSKILL 1
#define QL_MAINTYPE_EMOTION 3
#define QL_MAINTYPE_NONE 7
#define QL_NONE -1

#define REST_TIME 2 //in Sec
#define REST_PERC 10

#ifndef _H_CHARACTERINFO
#define _H_CHARACTERINFO

//Typedefinitionen

class TSocket;
class TPacketCypher;
class TCharacterInfo;
extern TCharacterInfo CharacterInfo;

//----------------------

//Strukturen
struct structQuicklistslot
{
	short type;
	int maintype;
	int id;
};

struct structCharacterQuicklist
{
	std::vector<structQuicklistslot> slot;
};

struct structCharacterGroup
{
	bool ingroup;
	int groupid;
	std::vector<int> requests;
	int next_stat_send;
};

struct structPing
{
	int packetcount;
	int lastpulsetime;
	int lastpulsecount;
	int lastautopulse;
};

struct structCharacterSkills
{
	int vnum;
	int castid;
	int cooldown;
};

//sc <type> <AttackGrade> <Dmgmin> <Dmgmax> <hitrate> <critchance> <critdamage>
//<type> <AttackGrade> <Dmgmin> <Dmgmax> <hitrate> <critchance> <critdamage>
//<DefenceGrade> <S.R. Dodge Min> <S.R. Dodge Max> <L.R. Dodge Min> <L.R. Dodge Max> <Magic Decrease>
//<FIRE> <GET DA WHATA NIGGA> <LIGHT> <Darkness>
struct structCharacterStatsResis
{
	int Fire;
	int Water;
	int Light;
	int Darkness;
};

struct structCharacterStatsDefence
{
	int DefenceGrade;
	int Short_Defence;
	int Short_Dodge;
	int Long_Defence;
	int Long_Dodge;
	int Magic_Decrease;
};

struct structCharacterStatsAttack
{
	int type;
	int attackgrade;
	int dmg_min;
	int dmg_max;
	int hitrate;
	int critchance;
	int critdamage;
};

struct structCharacterStatsBasic
{
	int stamina; int stam_plus;
	int intelligence; int int_plus;
	int walkspeed;
};

struct structCharacterStatsBase
{
	structCharacterStatsAttack Attack_Primary;
	structCharacterStatsAttack Attack_Secondary;
	structCharacterStatsDefence Defence;
	structCharacterStatsResis Resistances;
	structCharacterStatsBasic Basic;
};

struct structCharacterStats
{
	structCharacterStatsBase Stats_Base;
	structCharacterStatsBase Stats_Real;
};

struct structClasses
{
	int id;
	structCharacterStatsAttack Attack_Primary;
	structCharacterStatsAttack Attack_Secondary;
	structCharacterStatsDefence Defence;
	structCharacterStatsResis Resistances;
	structCharacterStatsBasic Basic;
};

struct structCharacterInfo
{
	int ID;
	int Slot;
	std::string Name;
	bool Gender;
	int Hairstyle;
	int Haircolor;
	int ClassType;
};

struct structCharacterStatus
{
	int currenthp;
	int currentmp;
	int maxhp;
	int maxmp;
	bool alive;
	bool isrest;
	int rest_next;
};

struct structCharacterLevel
{
	int mainlevel;
	int secondlevel;
	int mainexp;
	int mainexp_levelup;
	int secondexp;
	int secondexp_levelup;
};

struct structCharacterCast
{
	bool iscasting;
	int casttime;
	int allocated_id;
	int allocated_cast;
	int targetid;
	int targetobj;
};
//---------------------------

class TCharacterInfo
{
public:
	TCharacterInfo(void); //Konstruktor
	~TCharacterInfo(void); //Destruktor

	std::vector<structClasses> CharacterClasses;

	void GetAllMapPackets(TSocket *Client);
	void Send_CharacterInventory(TSocket *Client); //Character Inventar laden
	void Send_CharacterInfo(TSocket *Client); //Character Normale Informationen senden
	void Send_CharacterSkills(TSocket *Client); //Character Skills Informationen senden
	void Send_CharacterMap(TSocket *Client); //Character Map Informationen senden
	void Send_CharacterSpeed(TSocket *Client); //Character Geschwindigkeits Informationen senden
	void Send_CharacterMapout(TSocket *Client); //Character Map verlassen senden
	void Send_PlayerIn(TSocket *Player, TSocket *Client); //Beitritt eines Spielers auf der Map senden
	void Send_PlayerOut(TSocket *Player, TSocket *Client); //Verlassen eines Spielers auf der Map senden
	void Save_Character(TSocket *Client); //Charakterdaten in der Datenbank speichern
	void Send_CharacterStats(TSocket *Client); //Character Status senden 
	void Send_CharacterGold(TSocket *Client); //Charakter Gold Senden
	void Send_CharacterLevel(TSocket *Client); //Charakter Level Senden
	void Send_Quicklist( TSocket *Client ); //Skill Quicklist schicken
	void Quicklist_Change( TSocket *Client, int type, int list, int slot, int maintype, int id);
	void Quicklist_Remove( TSocket *Client, int type, int list, int slot);
	void Send_Equipment( TSocket *Client);
	int Classes_GetbyID(int id);
	void Send_CharacterStatPoints(TSocket *Client);
	void Stats_GetBase(TSocket *Client, int currentslot);
	void Stats_CalcReal(TSocket *Client);
	void XP_Add(TSocket *Client, int mainxp, int secondxp, int spxp); //XP geben
	void Player_Rest(TSocket *Client, int param1, int param2, int id);
	void Rest_Check(int mapindex, int playerindex);

	static DWORD WINAPI PlayerThread(LPVOID lpdwThreadParam);
	std::mutex lockthread;
};

#endif _H_CHARACTERINFO