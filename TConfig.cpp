#include "TConfig.h"
#include "stdafx.h"

TConfig Config;
//------------------------------------------------------------
//------------------------------------------------------------

std::string TConfig::GetIniString(std::string section, std::string key, std::string path)
{
	std::vector<char> buffer(255);
	GetPrivateProfileString(section.c_str(), key.c_str(), NULL, &buffer.front(), buffer.size(), path.c_str());
	return &buffer.front();
}

//------------------------------------------------------------

int TConfig::GetIniValue(std::string section, std::string key, std::string path)
{
	return GetPrivateProfileInt(section.c_str(), key.c_str(), NULL, path.c_str());
}
//------------------------------------------------------------

bool TConfig::LoadMapFiles()
{
	HANDLE hFile;
	WIN32_FIND_DATA wFindData;
	int NumberofMaps = 0;
	bool error = 0;
	std::string path = General.GetExePath();
	path += "\\maps\\zones\\*.*";

	std::cout << "Called TConfig::LoadMapFiles" << std::endl;

	//Erste Datei im Ordner suchen
	hFile = FindFirstFile(path.c_str(), &wFindData);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "TMap::LoadMapFiles failed, Code: " << GetLastError() << std::endl;
		return 1;
	}

	//Nächste Datei im Ordner suchen (überbrückung, da eine Fehlerhaft)
	FindNextFile(hFile, &wFindData);

	Map.MapData.reserve(500);

	//Alle Dateien im Ordner suchen und auslesen
	while(FindNextFile(hFile, &wFindData))
	{
		//Mapfile Namen auslesen und in Integer konvertieren
		structMapData tempMapData;
		tempMapData.id = atoi(wFindData.cFileName);

		//Zonen der Map laden
		error = this->LoadZone(&tempMapData, General.GetExePath() += (std::string)"\\maps\\zones\\" += wFindData.cFileName);

		if(error)
		{
			return 1;
		}

		//Map hinzufügen
		Map.MapData.push_back(tempMapData);
		NumberofMaps++;
	}

	std::cout << "TConfig::LoadMapFiles successfully" << std::endl;
	std::cout << "Number of Maps are loaded: " << NumberofMaps << std::endl;

	bool ec = this->LoadNpc();

	if(ec)
	{
		return 1;
	}

	ec = this->LoadEnemies();

	if(ec)
	{
		return 1;
	}

	ec = this->LoadPortals();

	if(ec)
	{
		return 1;
	}

	//Handle schließen
	FindClose(hFile);
	return 0;
}

//------------------------------------------------------------

bool TConfig::LoadZone( structMapData *currentMap, std::string filepath )
{
	FILE *file;
	
	//Zone File öffnen
	fopen_s(&file, filepath.c_str(), "rb");

	//Erste 2 Bytes für Höhe und Breite auslesen
	fread(&currentMap->zones.x, 1, sizeof(short), file);
	fread(&currentMap->zones.y, 1, sizeof(short), file);

	//Vector Höhe Allozieren
	currentMap->zones.grid.resize(currentMap->zones.y);

	//Grid durchlaufen und setzen
	for(auto i = 0; i < currentMap->zones.y; ++i)
	{
		currentMap->zones.grid[i].resize(currentMap->zones.x);
		for(auto t = 0; t < currentMap->zones.x; ++t)
		{
			currentMap->zones.grid[i][t] = std::getc(file);
		}
	}

	//Datei schließen
	fclose(file);
	return 0;
}

//------------------------------------------------------------

bool TConfig::LoadPortals()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\MapPortals.txt", std::ios::in);

	std::cout << "Called TConfig::LoadPortals" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParsePortalsBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::LoadPortals successfully" << std::endl;
	std::cout << "Number of Portals are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParsePortalsBuffer( std::string strPortalsbuffer )
{
	std::vector<std::string> vRow = General.split(strPortalsbuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structMapPortal MapPortal;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("MAP") == NULL)
		{
			MapPortal.mapid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("PORTALTYPE") == NULL)
		{
			MapPortal.portal_type = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MAPTITLE") == NULL)
		{
			MapPortal.maptitle_id = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("X") == NULL)
		{
			MapPortal.x = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("Y") == NULL)
		{
			MapPortal.y = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TARGET_MAP") == NULL)
		{
			MapPortal.target_id = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TARGET_X") == NULL)
		{
			MapPortal.target_x = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TARGET_Y") == NULL)
		{
			MapPortal.target_y = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("DISABLED") == NULL)
		{
			MapPortal.isDisabled = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			//Map suchen und hinzufügen
			for(auto i = 0; i < Map.MapData.size(); i++)
			{
				if(Map.MapData[i].id == MapPortal.mapid)
				{
					Map.MapData[i].Portal.push_back(MapPortal);
					return;
				}
			}
		}
	}
}

//------------------------------------------------------------

bool TConfig::LoadEnemies()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\MapEnemies.txt", std::ios::in);

	std::cout << "Called TConfig::LoadEnemies" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseMapEnemiesBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::LoadEnemies successfully" << std::endl;
	std::cout << "Number of MapEnemies are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseMapEnemiesBuffer( std::string strEnemybuffer )
{
	std::vector<std::string> vRow = General.split(strEnemybuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structMapEnemySpawn MapEnemySpawn;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		MapEnemySpawn.active_enemy_id = 0;
		MapEnemySpawn.next_respawn_time = 0;

		if(vIndex[0].compare("MAP") == NULL)
		{
			MapEnemySpawn.mapid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("VNUM") == NULL)
		{
			MapEnemySpawn.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LVLMIN") == NULL)
		{
			MapEnemySpawn.level_min = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LVLMAX") == NULL)
		{
			MapEnemySpawn.level_max = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("X") == NULL)
		{
			MapEnemySpawn.x = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("Y") == NULL)
		{
			MapEnemySpawn.y = General.s_atoi(vIndex[1]);
			
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			//NPC Daten aus der Liste übernehmen
			for(auto i = 0; i < Npcs.Npcs.size(); i++)
			{
/*				if(Npcs.Npcs[i].vnum == MapNpc.vnum)
				{
					MapNpc.level = Npcs.Npcs[i].level;
					MapNpc.hp_max = Npcs.Npcs[i].hp;
					MapNpc.hp_cur = Npcs.Npcs[i].hp;
					MapNpc.mp_max = Npcs.Npcs[i].mp;
					MapNpc.mp_cur = Npcs.Npcs[i].mp;*/

					//NPC hinzufügen
					for(auto i = 0; i < Map.MapData.size(); i++)
					{
						if(Map.MapData[i].id == MapEnemySpawn.mapid)
						{
							Map.MapData[i].EnemySpawns.push_back(MapEnemySpawn);
							return;
						}
					}
				//}
			}
		}
	}
}

//------------------------------------------------------------

bool TConfig::LoadNpc()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\MapNpcs.txt", std::ios::in);

	std::cout << "Called TConfig::LoadNpc" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseMapNpcBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::LoadNpc successfully" << std::endl;
	std::cout << "Number of MapNpcs are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseMapNpcBuffer( std::string strNpcbuffer )
{
	std::vector<std::string> vRow = General.split(strNpcbuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structMapNpc MapNpc;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("MAP") == NULL)
		{
			MapNpc.mapid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("VNUM") == NULL)
		{
			MapNpc.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("ID") == NULL)
		{
			MapNpc.npc_id = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("X") == NULL)
		{
			MapNpc.x = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("Y") == NULL)
		{
			MapNpc.y = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("DIRECTION") == NULL)
		{
			MapNpc.direction = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SHOP") == NULL)
		{
			MapNpc.shop = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MENUTYPE") == NULL)
		{
			MapNpc.menutype = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SHOPTYPE") == NULL)
		{
			MapNpc.shoptype = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SHOPNAME") == NULL)
		{
			MapNpc.shopname = vIndex[1];
		} else if(vIndex[0].compare("SHOPLIST") == NULL)
		{
			vIndexSplit = General.split(vIndex[1], '.');

			//ShopListen hinzufügen
			for(auto i = 0; i < NpcShop.ShopList.size(); i++)
			{
				for(auto x = 0; x < vIndexSplit.size(); x++)
				{
					if(NpcShop.ShopList[i].list_id == General.s_atoi(vIndexSplit[x]))
					{
						MapNpc.shoplist.push_back(NpcShop.ShopList[i]);
					}
				}
			}

		} else if(vIndex[0].compare("DIALOGID") == NULL)
		{
			MapNpc.dialogid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			//NPC Daten aus der Liste übernehmen
			for(auto i = 0; i < Npcs.Npcs.size(); i++)
			{
				if(Npcs.Npcs[i].vnum == MapNpc.vnum)
				{
					MapNpc.level = Npcs.Npcs[i].level;
					MapNpc.hp_max = Npcs.Npcs[i].hp;
					MapNpc.hp_cur = Npcs.Npcs[i].hp;
					MapNpc.mp_max = Npcs.Npcs[i].mp;
					MapNpc.mp_cur = Npcs.Npcs[i].mp;

					//NPC hinzufügen
					for(auto i = 0; i < Map.MapData.size(); i++)
					{
						if(Map.MapData[i].id == MapNpc.mapid)
						{
							Map.MapData[i].Npc.push_back(MapNpc);
							return;
						}
					}
				}
			}
		}
	}
}

//------------------------------------------------------------

bool TConfig::LoadConfigurationFiles()
{
	TCHAR Directory[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, Directory);

	std::string path = Directory;
	path += "\\configs\\server.ini";

	std::cout << "Called TConfig::LoadConfigurationFiles" << std::endl;

	//Ini File auslesen
	this->db_host = this->GetIniString("DATABASE", "HOST", path);
	this->db_user = this->GetIniString("DATABASE", "USER", path);
	this->db_password = this->GetIniString("DATABASE", "PASSWORD", path);
	this->db_schema = this->GetIniString("DATABASE", "SCHEMA", path);

	this->server_host = this->GetIniString("SERVER", "HOST", path);
	this->loginserver_port = this->GetIniValue("SERVER", "LOGIN_PORT", path);
	this->gameserver_port = this->GetIniValue("SERVER", "GAME_PORT", path);

	this->Rate_XP = General.s_ToFloat(this->GetIniString("GAME", "RATE_XP", path));
	this->Rate_JXP = General.s_ToFloat(this->GetIniString("GAME", "RATE_JXP", path));
	this->Rate_Drop = General.s_ToFloat(this->GetIniString("GAME", "RATE_DROP", path));
	this->Rate_Gold = General.s_ToFloat(this->GetIniString("GAME", "RATE_GOLD", path));

	//Prüfen ob alles erfolgreich ausgelesen
	if(this->db_host.empty() || this->db_user.empty() ||
		this->db_password.empty() || this->db_schema.empty() || this->server_host.empty() ||
		this->loginserver_port == 0 || this->gameserver_port == 0)
	{
		std::cout << "Error: Config.LoadConfigurationFiles() failed" << std::endl;
		std::cout << "Path: " << path.c_str() << std::endl;
		std::cout << "Please press Enter to exit..." << std::endl;
		return 1;
	}

	std::cout << "TConfig::LoadConfigurationFiles successfully" << std::endl;

	return 0;
}

//------------------------------------------------------------

bool TConfig::ReadShopList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Shoplist.txt", std::ios::in);

	std::cout << "Called TConfig::ReadShopList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseShopListBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadShopList successfully" << std::endl;
	std::cout << "Number of Shoplists are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseShopListBuffer( std::string strShopbuffer )
{
	std::vector<std::string> vRow = General.split(strShopbuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	std::vector<std::string> vDetailSplit;
	structShopListData ShopList;
	std::vector<structShopListData> ListContainer;
	structShopList List;	
	int IndexSize, Type;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("LISTID") == NULL)
		{
			List.list_id = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("ITEMS") == NULL)
		{
			vIndexSplit = General.split(vIndex[1], ' ');
			IndexSize = vIndexSplit.size();

			for(auto i = 0; i < IndexSize; i++)
			{
				ShopList.inventorytype = -1;
				ShopList.itemcount = -1;
				ShopList.itemid = -1;
				ShopList.itemprice = -1;
				ShopList.itemrare = -1;
				ShopList.itemupgrade = -1;
				ShopList.slot = -1;

				vDetailSplit = General.split(vIndexSplit[i], '.');
				Type = General.s_atoi(vDetailSplit[0]);

				switch(Type)
				{
				case EQUIP_INVENTORY:
					ShopList.inventorytype = Type;
					ShopList.slot = General.s_atoi(vDetailSplit[1]);
					ShopList.itemid = General.s_atoi(vDetailSplit[2]);
					ShopList.itemrare = General.s_atoi(vDetailSplit[3]);
					ShopList.itemupgrade = General.s_atoi(vDetailSplit[4]);
					ShopList.itemprice = General.s_atoi(vDetailSplit[5]);
					break;
				case MAIN_INVENTORY:
					ShopList.inventorytype = Type;
					ShopList.slot = General.s_atoi(vDetailSplit[1]);
					ShopList.itemid = General.s_atoi(vDetailSplit[2]);
					ShopList.itemcount = General.s_atoi(vDetailSplit[3]);
					ShopList.itemprice = General.s_atoi(vDetailSplit[4]);
					break;
				case OTHER_INVENTORY:
					ShopList.inventorytype = Type;
					ShopList.slot = General.s_atoi(vDetailSplit[1]);
					ShopList.itemid = General.s_atoi(vDetailSplit[2]);
					ShopList.itemcount = General.s_atoi(vDetailSplit[3]);
					ShopList.itemprice = General.s_atoi(vDetailSplit[4]);
					break;
				}

				ListContainer.push_back(ShopList);			
			}

		} else if(vIndex[0].compare("<END>") == NULL)
		{
			NpcShop.ShopList.push_back(List);
			NpcShop.ShopList[NpcShop.ShopList.size()-1].list = ListContainer;
			return;
		}
	}
}

//----------------------------------------------------------

bool TConfig::ReadItemList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;
	std::string cuttedItem;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Item.txt", std::ios::in);

	std::cout << "Called TConfig::ReadItemList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseItemBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadItemList successfully" << std::endl;
	std::cout << "Number of Items are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseItemBuffer( std::string strItembuffer )
{
	std::vector<std::string> vRow = General.split(strItembuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structItems Item;
	int IndexSize;

	//Stats initialisieren
	//Short Range
	Item.stats.sr_crit_chance = 0;
	Item.stats.sr_crit_damage = 0;
	Item.stats.sr_damage_min = 0;
	Item.stats.sr_damage_max = 0;
	Item.stats.sr_hitrate = 0;
	Item.stats.sr_defence = 0;
	//Long Range
	Item.stats.lr_crit_chance = 0;
	Item.stats.lr_crit_damage = 0;
	Item.stats.lr_damage_min = 0;
	Item.stats.lr_damage_max = 0;
	Item.stats.lr_hitrate = 0;
	Item.stats.lr_defence = 0;
	//Magic
	Item.stats.magic_defence = 0;
	//Def
	Item.stats.dodge = 0;

	//Itemsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{

		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("VNUM") == NULL)
		{
			Item.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("NAME") == NULL)
		{
			Item.name = vIndex[1];
		} else if(vIndex[0].compare("INVENTAR") == NULL)
		{
			Item.inventorytype = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("PRICE") == NULL)
		{
			Item.price = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("EFFECT") == NULL)
		{
			vIndexSplit = General.split(vIndex[1], '.');
			IndexSize = vIndexSplit.size();

			for(auto i = 0; i < IndexSize; i++)
			{					
				Item.effects.push_back(General.s_atoi(vIndexSplit[i]));
			}
		} else if(vIndex[0].compare("DELETE_ON_USE") == NULL)
		{
			Item.on_use_delete = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("EQ_SLOT") == NULL)
		{
			Item.equipment_slot = General.s_atoi(vIndex[1]);
		//-------- STATS
		} else if(vIndex[0].compare("SR_DAMAGEMIN") == NULL)
		{
			Item.stats.sr_damage_min = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SR_DAMAGEMAX") == NULL)
		{
			Item.stats.sr_damage_max = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SR_HITRATE") == NULL)
		{
			Item.stats.sr_hitrate = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SR_CRITCHANCE") == NULL)
		{
			Item.stats.sr_crit_chance = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SR_CRITDAMAGE") == NULL)
		{
			Item.stats.sr_crit_damage = General.s_atoi(vIndex[1]);
		}  else if(vIndex[0].compare("LR_DAMAGEMIN") == NULL)
		{
			Item.stats.lr_damage_min = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LR_DAMAGEMAX") == NULL)
		{
			Item.stats.lr_damage_max = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LR_HITRATE") == NULL)
		{
			Item.stats.lr_hitrate = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LR_CRITCHANCE") == NULL)
		{
			Item.stats.lr_crit_chance = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LR_CRITDAMAGE") == NULL)
		{
			Item.stats.lr_crit_damage = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("SR_DEF") == NULL)
		{
			Item.stats.sr_defence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("LR_DEF") == NULL)
		{
			Item.stats.lr_defence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MAGIC_DEF") == NULL)
		{
			Item.stats.magic_defence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("DODGE") == NULL)
		{
			Item.stats.dodge = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			Items.vItems.push_back(Item);
		}
	}
}

//------------------------------------------------------------

bool TConfig::ReadSkillList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Skills.txt", std::ios::in);

	std::cout << "Called TConfig::ReadSkillsList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseSkillBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadSkillList successfully" << std::endl;
	std::cout << "Number of Skills are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseSkillBuffer( std::string strItembuffer )
{
	std::vector<std::string> vRow = General.split(strItembuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structSkills Skill;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("VNUM") == NULL)
		{
			Skill.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("CASTID") == NULL)
		{
			Skill.castid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("EFFECT") == NULL)
		{
			Skill.effect = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MOVEMENT") == NULL)
		{
			Skill.moveid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TYPE") == NULL)
		{
			Skill.typ = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MPCOST") == NULL)
		{
			Skill.mpcost = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("JLEVEL") == NULL)
		{
			Skill.jlevel = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("CP") == NULL)
		{
			Skill.cp = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("RANGE") == NULL)
		{
			Skill.range = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TARGET") == NULL)
		{
			Skill.target = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("CASTTIME") == NULL)
		{
			Skill.casttime = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("COOLDOWN") == NULL)
		{
			Skill.cooldown = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("COST") == NULL)
		{
			Skill.cost = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("ITEMNEED") == NULL)
		{
			Skill.itemneed = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			Skills.Skills.push_back(Skill);
		}
	}
}

//------------------------------------------------------------

bool TConfig::ReadClassesList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Classes.txt", std::ios::in);

	std::cout << "Called TConfig::ReadClassesList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseClassesBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadClassesList successfully" << std::endl;
	std::cout << "Number of Classes are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseClassesBuffer( std::string strItembuffer )
{
	std::vector<std::string> vRow = General.split(strItembuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structClasses Klasse;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("class") == NULL)
		{
			Klasse.id = General.s_atoi(vIndex[1]);
		// ------ BASE STATS -------------
		} else if(vIndex[0].compare("stamina") == NULL)
		{
			Klasse.Basic.stamina = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("stam_plus") == NULL)
		{
			Klasse.Basic.stam_plus = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("intelligence") == NULL)
		{
			Klasse.Basic.intelligence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("int_plus") == NULL)
		{
			Klasse.Basic.int_plus = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("walkspeed") == NULL)
		{
			Klasse.Basic.walkspeed = General.s_atoi(vIndex[1]);
		// ------ //Erste Attack -------------
		} else if(vIndex[0].compare("pa_type") == NULL)
		{
			Klasse.Attack_Primary.type = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_attackgrade") == NULL)
		{
			Klasse.Attack_Primary.attackgrade = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_dmgmin") == NULL)
		{
			Klasse.Attack_Primary.dmg_min = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_dmgmax") == NULL)
		{
			Klasse.Attack_Primary.dmg_max = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_hitrate") == NULL)
		{
			Klasse.Attack_Primary.hitrate = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_critchance") == NULL)
		{
			Klasse.Attack_Primary.critchance = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("pa_critdamage") == NULL)
		{
			Klasse.Attack_Primary.critdamage = General.s_atoi(vIndex[1]);
		// ------ //Zweite Attack -------------
		} else if(vIndex[0].compare("sa_type") == NULL)
		{
			Klasse.Attack_Secondary.type = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_attackgrade") == NULL)
		{
			Klasse.Attack_Secondary.attackgrade = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_dmgmin") == NULL)
		{
			Klasse.Attack_Secondary.dmg_min = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_dmgmax") == NULL)
		{
			Klasse.Attack_Secondary.dmg_max = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_hitrate") == NULL)
		{
			Klasse.Attack_Secondary.hitrate = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_critchance") == NULL)
		{
			Klasse.Attack_Secondary.critchance = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sa_critdamage") == NULL)
		{
			Klasse.Attack_Secondary.critdamage = General.s_atoi(vIndex[1]);
		// ------ //Defence -------------
		} else if(vIndex[0].compare("def_grade") == NULL)
		{
			Klasse.Defence.DefenceGrade = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sr_defence") == NULL)
		{
			Klasse.Defence.Short_Defence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("sr_dodge") == NULL)
		{
			Klasse.Defence.Short_Dodge = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("lr_defence") == NULL)
		{
			Klasse.Defence.Long_Defence = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("lr_dodge") == NULL)
		{
			Klasse.Defence.Long_Dodge = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("magic_decrease") == NULL)
		{
			Klasse.Defence.Magic_Decrease = General.s_atoi(vIndex[1]);
		// ------ Resis -------------
		} else if(vIndex[0].compare("res_fire") == NULL)
		{
			Klasse.Resistances.Fire = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("res_water") == NULL)
		{
			Klasse.Resistances.Water = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("res_light") == NULL)
		{
			Klasse.Resistances.Light = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("res_darkness") == NULL)
		{
			Klasse.Resistances.Darkness = General.s_atoi(vIndex[1]);
		//-------------------
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			CharacterInfo.CharacterClasses.push_back(Klasse);
		}
	}
}

//------------------------------------------------------------

bool TConfig::ReadEnemySkillList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\EnemySkills.txt", std::ios::in);

	std::cout << "Called TConfig::ReadEnemySkillsList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseEnemySkillBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadEnemySkillList successfully" << std::endl;
	std::cout << "Number of EnemySkills are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseEnemySkillBuffer( std::string strItembuffer )
{
	std::vector<std::string> vRow = General.split(strItembuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structEnemySkills Skill;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("VNUM") == NULL)
		{
			Skill.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("EFFECT") == NULL)
		{
			Skill.effect = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MOVEMENT") == NULL)
		{
			Skill.moveid = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("TYPE") == NULL)
		{
			Skill.typ = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MPCOST") == NULL)
		{
			Skill.mpcost = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("RANGE") == NULL)
		{
			Skill.range = General.s_atoi(vIndex[1]);
		}  else if(vIndex[0].compare("CASTTIME") == NULL)
		{
			Skill.casttime = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("COOLDOWN") == NULL)
		{
			Skill.cooldown = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			Skills.EnemySkills.push_back(Skill);
		}
	}
}

//------------------------------------------------------------

bool TConfig::ReadNpcList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Npcs.txt", std::ios::in);

	std::cout << "Called TConfig::ReadNpcList" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseNpcBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::ReadNpcList successfully" << std::endl;
	std::cout << "Number of Npcs are loaded: " << iItemsCount << std::endl;

	return false;
}

//------------------------------------------------------------

void TConfig::ParseNpcBuffer( std::string strItembuffer )
{
	std::vector<std::string> vRow = General.split(strItembuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structNpcs Npc;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("VNUM") == NULL)
		{
			Npc.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("HP") == NULL)
		{
			Npc.hp = General.s_atoi(vIndex[1]);
		}else if(vIndex[0].compare("MP") == NULL) 
		{
			Npc.mp = General.s_atoi(vIndex[1]);
		}else if(vIndex[0].compare("LEVEL") == NULL)
		{
			Npc.level = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("<END>") == NULL)
		{
			Npcs.Npcs.push_back(Npc);
		}
	}
}

//------------------------------------------------------------

bool TConfig::ReadEnemyList()
{
	bool bReadloop = true;
	unsigned long ulFilesize;
	std::fstream fsFile;
	std::string strFilebuffer, strIndex;
	std::size_t sztStart = NULL, sztEnd = NULL;
	int iItemsCount = NULL;

	//Datei öffnen
	fsFile.open(General.GetExePath() += "\\data\\Enemies.txt", std::ios::in);

	std::cout << "Called TConfig::LoadEnemyTypes" << std::endl;

	if(!fsFile.is_open())
	{
		std::cout << "Error, can't load List Files!" << std::endl;
		return true;
	}

	//Filesize ermitteln und wieder zur Anfangsposition setzen
	fsFile.seekg(NULL, std::ios::end);
	ulFilesize = fsFile.tellg();
	fsFile.seekg(NULL, std::ios::beg);

	//Speicher allozieren und Filebuffer einlesen
	strFilebuffer.resize(ulFilesize);
	fsFile.read((char*)strFilebuffer.c_str(), ulFilesize);

	//Informationen einlesen
	do
	{
		//Start und End Position vom Item suchen
		sztStart = strFilebuffer.find("<START>", sztEnd);
		sztEnd = strFilebuffer.find("<END>", sztStart);

		//Wenn Positionen gefunden dann buffer rausschneiden, ansonsten Schleife verlassen
		if(sztStart != std::string::npos || sztEnd != std::string::npos)
		{
			this->ParseEnemyTypeBuffer(strFilebuffer.substr(sztStart, (std::size_t)((sztEnd - sztStart) + 5)).c_str());
			iItemsCount++;
		} else
		{
			bReadloop = false;
		}

	} while (bReadloop == true);

	std::cout << "TConfig::LoadEnemyTypes successfully" << std::endl;
	std::cout << "Number of Enemy Types are loaded: " << iItemsCount << std::endl;

	return false;
}
//------------------------------------------------------------

void TConfig::ParseEnemyTypeBuffer( std::string strEnemybuffer )
{
	std::vector<std::string> vRow = General.split(strEnemybuffer, '\n');
	std::vector<std::string> vIndex;
	std::vector<std::string> vIndexSplit;
	structEnemyType enemytype;
	int IndexSize;

	//Npcsbuffer parsen
	for(std::vector<std::string>::iterator it = vRow.begin(); it < vRow.end(); it++)
	{
		vIndex = General.split(*it, '=');

		if(vIndex[0].compare("VNUM") == NULL)
		{
			enemytype.vnum = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("HP") == NULL)
		{
			enemytype.hp = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("MP") == NULL)
		{
			enemytype.mp = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("XP") == NULL)
		{
			enemytype.xp = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("JXP") == NULL)
		{
			enemytype.jxp = General.s_atoi(vIndex[1]);
		} else if(vIndex[0].compare("DROPTABLE") == NULL)
		{
			std::vector<std::string> dropitem = General.split(vIndex[1], '|');
			structDroptable droptable;
			for(int i = 0; i < dropitem.size(); i++)
			{
				std::vector<std::string> enditem = General.split(dropitem[i], ';');
				droptable.vnum = General.s_atoi(enditem[0]);
				droptable.chance = General.s_atoi(enditem[1]);
				enemytype.droptable.push_back(droptable);
			}

		} else if(vIndex[0].compare("GOLD") == NULL)
		{
			std::vector<std::string> gold = General.split(vIndex[1], ';');
			enemytype.dropgold.count = General.s_atoi(gold[0]);
			enemytype.dropgold.chance = General.s_atoi(gold[1]);

		} else if(vIndex[0].compare("SKILLS") == NULL)
		{
			std::vector<std::string> skill = General.split(vIndex[1], '|');

			for(int i = 0; i < skill.size(); i++)
			{
				int id = General.s_atoi(skill[i]);
				enemytype.skills.push_back(id);
			}

		} else if(vIndex[0].compare("<END>") == NULL)
		{
			Enemies.types.push_back(enemytype);
		}
	}
}

//------------------------------------------------------------
