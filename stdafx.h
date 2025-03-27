// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <iostream>
#include <windows.h>
#include <memory>
#include <fstream>
#include <stdarg.h>
#include <sstream>
#include <mutex>
#include <thread>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "mysql_connection.h"
#include "TCritical.h"
#include "TEnemys.h"
#include "TDatabase.h"
#include "TItems.h"
#include "TAccount.h"
#include "TNPCShop.h"
#include "TMap.h"
#include "TEffects.h"
#include "TFriend.h"
#include "TTrade.h"
#include "TGroup.h"
#include "TChat.h"
#include "TCharacterInventory.h"
#include "TCharacterMove.h"
#include "TCharacterInfo.h"
#include "TCharacter.h"
#include "TServer.h"
#include "TPacketCypher.h"
#include "TPacketHandler.h"
#include "TGeneral.h"
#include "TConfig.h"
#include "TMath.h"
#include "TMessage.h"
#include "TNpc.h"
#include "TSkills.h"


#pragma comment(lib, "mysqlcppconn.lib")
#pragma comment(lib, "ws2_32.lib")

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
