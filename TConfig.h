#pragma once

//Base Config
#define BASE_WALKSPEED_MS 32//8 ms bei Walkspeed 1 pro Kästchen
#define GRP_XP_BONUS 0

#ifndef _H_CONFIG
#define _H_CONFIG

class TConfig;
extern TConfig Config;

class TConfig
{
private:
	std::string GetIniString(std::string section, std::string key, std::string path); //String auslesen von ini Datei
	int GetIniValue(std::string section, std::string key, std::string path); //Integer auslesen von ini Datei
	
	bool LoadZone(structMapData *currentMap, std::string filepath); //Zones File Read
	bool LoadPortal(structMapData *currentMap, std::string filepath); //Portals File Read
	bool LoadNpc(); //Npcs File Read
	bool LoadEnemies(); //Enemy Spawns Read
	bool LoadPortals(); //Portal File Read
	void ParsePortalsBuffer(std::string strPortalBuffer); //Portal File Parse
	void ParseMapNpcBuffer(std::string strNpcBuffer); //MapNpcs File Parse
	void ParseMapEnemiesBuffer(std::string strEnemyBuffer); //MapEnemies File Parse
	void ParseShopListBuffer(std::string strShopBuffer); //Shop File Parse
	void ParseItemBuffer(std::string strItembuffer); //Items Parsen
	void ParseSkillBuffer(std::string strSkillbuffer); //Skills parsen
	void ParseEnemySkillBuffer(std::string strSkillbuffer); //Skills parsen
	void ParseNpcBuffer(std::string strNpcBuffer); //Npcs File Parse
	void ParseEnemyTypeBuffer( std::string strEnemybuffer ); //Enemy Type Parse
	void ParseClassesBuffer( std::string strItembuffer ); //Klassen parsen
public:

	//Variablen Server einstellungen
	std::string db_host;
	std::string db_user;
	std::string db_password;
	std::string db_schema;

	std::string server_host;
	u_short loginserver_port;
	u_short gameserver_port;

	float Rate_XP;
	float Rate_JXP;
	float Rate_Drop;
	float Rate_Gold;
	//------------------------------------


	bool LoadConfigurationFiles(); //ini Einstellung auslesen
	bool LoadMapFiles(); //Read all map Files
	bool ReadShopList(); //Shop File Read
	bool ReadItemList(); //Item Dateien auslesen
	bool ReadSkillList(); //Npc Dateien auslesen
	bool ReadEnemySkillList(); //Npc Dateien auslesen
	bool ReadNpcList(); //Npc Dateien auslesen
	bool ReadEnemyList(); //Gegner Base Typen
	bool ReadClassesList(); //Klassen auslesen
};

#endif _H_CONFIG
