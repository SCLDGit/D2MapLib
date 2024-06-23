
#include <cstdio>
#include <windows.h>

#include <fstream>
#include <iostream>
#include <string>

#include "d2_ptrs.h"
#include "d2_structs.h"
#include "d2_version.h"
#include "d2data/d2_game_object.h"
#include "d2data/d2_npc_type.h"
#include "d2data/d2_level.h"
#include "json.h"
#include "log.h"
#include "map.h"
#include "offset.h"
#include "d2_client_version.h"

enum UnitType {
    UNIT_TYPE_PLAYER = 0,
    UNIT_TYPE_NPC = 1,
    UNIT_TYPE_OBJECT = 2,
    UNIT_TYPE_MISSILE = 3,
    UNIT_TYPE_ITEM = 4,
    UNIT_TYPE_TILE = 5
};

d2client_struct D2Client;
char D2_DIR[MAX_PATH] = "";
const CHAR *DIABLO_2 = (CHAR *)"Diablo II";
const CHAR *DIABLO_2_VERSION = (CHAR *)"v1.xy";

const CHAR *PATH_OF_DIABLO = "Path of Diablo";
const CHAR *PROJECT_DIABLO = "ProjectD2";

unsigned long D2ClientInterface(VOID) {
    return D2Client.dwInit;
}

VOID __stdcall ExceptionHandler(VOID) {
    fprintf(stderr, "\n] We got a big Error here! [\n");
    ExitProcess(0);
}

D2Version gameVersion = VersionUnknown;

/** If this value changes, update __asm JMP */
int D2CLIENT_Pod_InitGameMisc_I_P = 0x6faf559b;
void /* __declspec(naked) */ D2CLIENT_Pod_InitGameMisc() {
    __asm(
        "MOVL %EBP, %ESP\n"
        "POPL %EBP\n"
        ".intel_syntax noprefix\n"
        "PUSH ECX\n"
        "PUSH EBP\n"
        "PUSH ESI\n"
        "PUSH EDI\n"
        ".att_syntax prefix\n"
        "JMP 0x6faf559b\n"  // Magic Jump
        "PUSHL %EBP\n");
}

// bool isPathOfDiablo = false;
void d2_game_init_pod() {
    *p_STORM_Pod_MPQHashTable = (DWORD)NULL;
    D2Client.dwInit = 1;
    D2Client.fpInit = reinterpret_cast<DWORD>(D2ClientInterface);

    log_trace("Init:Dll", lk_s("dll", "Fog.dll"));
    FOG_10021("D2");
    //FOG_10019(DIABLO_2, (DWORD)ExceptionHandler, DIABLO_2_VERSION, 1);
    FOG_10101(1, 0);
    FOG_10089(1);
    if (!FOG_10218()) {
        log_error("Init:Dll:Failed", lk_s("dll", "Fog.dll"));
        //ExitProcess(1);
    }
    log_debug("Init:Dll:Done", lk_s("dll", "Fog.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Win.dll"));
    if (!D2WIN_10174() || !D2WIN_10072((DWORD)NULL, (DWORD)NULL, (DWORD)NULL, &D2Client)) {
        log_error("Init:Dll:Failed", lk_s("dll", "D2Win.dll"));
        //ExitProcess(1);
    }
    log_debug("Init:Dll:Done", lk_s("dll", "D2Win.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Lang.dll"));
    D2LANG_10009(0, "ENG", 0);
    log_debug("Init:Dll:Done", lk_s("dll", "D2Lang.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Client.dll"));
    D2COMMON_Pod_InitDataTables(0, 0, 0);
    D2CLIENT_Pod_InitGameMisc();
    log_debug("Init:Dll:Done", lk_s("dll", "D2Client.dll"));
}

int D2CLIENT_Pd2_InitGameMisc_I_P = 0x6faf454b;
void /* __declspec(naked) */ D2CLIENT_Pd2_InitGameMisc() {
    __asm(
        "MOVL %EBP, %ESP\n"
        "POPL %EBP\n"
        "PUSHL %ECX\n"
        "PUSHL %EBP\n"
        "PUSHL %ESI\n"
        "PUSHL %EDI\n"
        "JMP 0x6faf454b\n"  // Magic Jump
        );
}
bool isProjectDiablo2 = false;
void d2_game_init_pd2() {
    *p_STORM_Pd2_MPQHashTable = 0;
    D2Client.dwInit = 1;
    D2Client.fpInit = reinterpret_cast<DWORD>(D2ClientInterface);

    log_trace("Init:Dll", lk_s("dll", "Fog.dll"));
    FOG_10021("D2");
    //FOG_10019(DIABLO_2, (DWORD)ExceptionHandler, DIABLO_2_VERSION, 1);
    FOG_10101(1, 0);
    FOG_10089(1);

    if (!FOG_10218()) {
        log_error("Init:Dll:Failed", lk_s("dll", "Fog.dll"));
        //ExitProcess(1);
    }
    log_debug("Init:Dll:Done", lk_s("dll", "Fog.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Win.dll"));
    if (!D2WIN_10086() || !D2WIN_10005(0, 0, 0, &D2Client)) {
        log_error("InitFailed", lk_s("dll", "D2Win.dll"));
        //ExitProcess(1);
    }
    log_debug("Init:Dll:Done", lk_s("dll", "D2Win.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Lang.dll"));
    D2LANG_10008(0, "ENG", 0);
    log_debug("Init:Dll:Done", lk_s("dll", "D2Lang.dll"));

    log_trace("Init:Dll", lk_s("dll", "D2Client.dll"));
    D2COMMON_Pd2_InitDataTables(0, 0, 0);
    D2CLIENT_Pd2_InitGameMisc();
    log_debug("Init:Dll:Done", lk_s("dll", "D2Client.dll"));
}

void d2_game_init(char *folderName) {
    log_debug("Init:Dll", lk_s("path", folderName));

    gameVersion = game_version(folderName);
    if (gameVersion == VersionUnknown) {
        log_error("Init:Failed:UnknownGameVersion", lk_s("path", folderName));
        ExitProcess(1);
    }

    const char *gamePath = game_version_path(gameVersion);
    if (gamePath == nullptr) {
        log_error("Init:Failed:UnknownGamePath", lk_s("path", folderName), lk_s("version", game_version_path(gameVersion)));
        ExitProcess(1);
    }

    sprintf_s(D2_DIR, sizeof(D2_DIR), "%s/%s", folderName, game_version_path(gameVersion));
    log_info("Init:Game", lk_s("version", game_version_path(gameVersion)), lk_s("path", D2_DIR));
    memset(&D2Client, (DWORD)NULL, sizeof(d2client_struct));
    SetCurrentDirectory(D2_DIR);

    DefineOffsets();
    log_debug("Init:Offsets:Defined");

    if (gameVersion == VersionPathOfDiablo) {
        d2_game_init_pod();
    } else if (gameVersion == VersionProjectDiablo2 || gameVersion == VersionDiablo2) {
        d2_game_init_pd2();
    } else {
        log_error("Init:Failed:GameInit", lk_s("path", D2_DIR));
        ExitProcess(1);
    }

    SetCurrentDirectory(folderName);
}

Level *__fastcall d2_get_level(ActMisc *misc, DWORD levelCode) {
    LevelTxt *levelData = d2common_get_level_text(gameVersion, levelCode); 
    if (!levelData) return nullptr;

    for (Level *pLevel = misc->pLevelFirst; pLevel; pLevel = pLevel->pNextLevel) {
        if (pLevel->dwLevelNo == levelCode) return pLevel;
    }

    return d2common_get_level(gameVersion, misc, levelCode);
}

void add_collision_data(CollMap *pCol, unsigned int originX, unsigned int originY) {
    if (pCol == nullptr) return;

    unsigned long x = pCol->dwPosGameX - originX;
    unsigned long y = pCol->dwPosGameY - originY;
    unsigned long cx = pCol->dwSizeGameX;
    unsigned long cy = pCol->dwSizeGameY;

    unsigned long nLimitX = x + cx;
    unsigned long nLimitY = y + cy;

    WORD *p = pCol->pMapStart;
    for (auto j = y; j < nLimitY; j++) {
        for (auto i = x; i < nLimitX; i++) {
            int pVal = *p;
            if (pVal == 1024) pVal = 1;
            map_set(i, j, pVal);
            p++;
        }
    }
}

const char *get_object_type() {
    return "object";
}

const char *get_object_class(unsigned int code, int operateFn) {
    switch (operateFn){
        case 1: return "casket";
        case 2: return "shrine";
        case 3: return "urn";

        case 5: return "barrel";
        case 7: return "barrel-exploding";
        case 14: return "bolder";
        case 19: return "rack-armor";
        case 20: return "rack-weapon";
        case 22: return "well";
        case 23: return "waypoint";
        case 68: return "urn-evil";
        case 30: return "chest-exploding";
        case 40:
        case 41:
        case 59:
        case 58:
        case 4: 
            return "chest";
        case 8: 
        case 18:
        case 29:
            return "door";
        /** Diablo Seals */
        case 54:
        case 52:
        case 55:
        case 56:
        /** Trist stones */
        case 9:

        /* complelling orb */
        case 53:
        /* Horiadric orifice */
        case 25:
        /* Sewer Lever */
        case 45:
        // /* Hell forge */
        case 49:
        /** Tome */
        case 28:
        /** Sun altar */
        case 24: 
            return "quest";
        default:
            break;
    }

    if (code == 580 || code == 581) return "chest-super";
    return nullptr;
}

bool is_good_exit(Act *pAct, Level *pLevel, unsigned int exitId) {
    // Act 1
    // BloodMoor -> Den of evil
    if (pLevel->dwLevelNo == AreaLevel::BloodMoor && exitId == AreaLevel::DenOfEvil) return true;

    // Tamoe Highlands -> Pit
    if (pLevel->dwLevelNo == AreaLevel::TamoeHighland && exitId == AreaLevel::PitLevel1) return true;
    // Black Forest -> ForgottenTower
    if (pLevel->dwLevelNo == AreaLevel::BlackMarsh && exitId == AreaLevel::ForgottenTower) return true;

    // Act 2
    // Correct tomb
    if (exitId == pAct->pMisc->dwStaffTombLevel) return true;
    // Staff Components
    if (pLevel->dwLevelNo == AreaLevel::FarOasis && exitId == AreaLevel::MaggotLairLevel1) return true;
    if (pLevel->dwLevelNo == AreaLevel::ValleyOfSnakes && exitId == AreaLevel::ClawViperTempleLevel1) return true;
    if (pLevel->dwLevelNo == AreaLevel::RockyWaste && exitId == AreaLevel::StonyTombLevel1) return true;

    // Ancient tunnels
    if (pLevel->dwLevelNo == AreaLevel::LostCity && exitId == AreaLevel::AncientTunnels) return true;

    // Act 3
    // Parts
    if (pLevel->dwLevelNo == AreaLevel::SpiderForest && exitId == AreaLevel::SpiderCavern) return true;
    if (pLevel->dwLevelNo == AreaLevel::FlayerJungle && exitId == AreaLevel::FlayerDungeonLevel1) return true;

    // Kurast -> RuinedTemple 
    if (pLevel->dwLevelNo == AreaLevel::KurastBazaar && exitId == AreaLevel::RuinedTemple) return true;

    // Act 5
    // Crystaline passage -> Frozen River
    if (pLevel->dwLevelNo == AreaLevel::CrystallinePassage && exitId == AreaLevel::FrozenRiver) return true;

    return false;
}

void dump_objects(Act *pAct, Level *pLevel, Room2 *pRoom2) {
    unsigned int offsetX = pLevel->dwPosX * 5;
    unsigned int offsetY = pLevel->dwPosY * 5;

    unsigned int roomOffsetX = pRoom2->dwPosX * 5 - offsetX;
    unsigned int roomOffsetY = pRoom2->dwPosY * 5 - offsetY;

    for (PresetUnit *pPresetUnit = pRoom2->pPreset; pPresetUnit; pPresetUnit = pPresetUnit->pPresetNext) {
        const char *objectType = nullptr;
        const char *objectName = nullptr;
        const char *objectClass = nullptr;
        bool isGoodExit = false;
        int operateFn = -1;

        unsigned int objectId = -1;

        unsigned int coordX = roomOffsetX + pPresetUnit->dwPosX;
        unsigned int coordY = roomOffsetY + pPresetUnit->dwPosY;

        if (pPresetUnit->dwType == UNIT_TYPE_NPC) {
            if (npc_is_useless(pPresetUnit->dwTxtFileNo)) continue;
            objectType = "npc";
            objectId = pPresetUnit->dwTxtFileNo;

        } else if (pPresetUnit->dwType == UNIT_TYPE_OBJECT) {
            objectType = get_object_type();
            objectId = pPresetUnit->dwTxtFileNo;
            if (pPresetUnit->dwTxtFileNo < 580) {
                ObjectTxt *txt = d2common_get_object_txt(gameVersion, pPresetUnit->dwTxtFileNo);
                objectName = txt->szName;
                if (txt->nSelectable0) operateFn = txt->nOperateFn;
            }
            objectClass = get_object_class(pPresetUnit->dwTxtFileNo, operateFn);
        } else if (pPresetUnit->dwType == UNIT_TYPE_TILE) {
            for (RoomTile *pRoomTile = pRoom2->pRoomTiles; pRoomTile; pRoomTile = pRoomTile->pNext) {
                if (*pRoomTile->nNum == pPresetUnit->dwTxtFileNo) {
                    objectId = pRoomTile->pRoom2->pLevel->dwLevelNo;
                    if (is_good_exit(pAct, pLevel, objectId)) isGoodExit = true;
                    objectType = "exit";
                }
            }
        }

        if (objectType) {
            json_object_start();
            json_key_value("id", objectId);
            json_key_value("type", objectType);
            json_key_value("x", coordX);
            json_key_value("y", coordY);
            if (objectName) json_key_value("name", objectName);
            if (operateFn > -1) json_key_value("op", operateFn);
            if (isGoodExit) json_key_value("isGoodExit", true);
            if (objectClass) json_key_value("class", objectClass);
            json_object_end();
        }
    }
}

void dump_rooms(Level *pLevel) {
    // Room2 *room = pLevel->pRoom2First;

    for (Room2 *pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
        json_object_start();
        unsigned int coordX = pRoom->dwPosX * 5;
        unsigned int coordY = pRoom->dwPosY * 5;
        unsigned int width = pRoom->dwSizeX * 5;
        unsigned int height = pRoom->dwSizeY * 5;
        json_key_value("x", coordX);
        json_key_value("y", coordY);
        json_key_value("width", width);
        json_key_value("height", height);
        json_object_end();
    }
}

void dump_map_collision(unsigned int width) {
    int maxY = map_max_y();
    for (int y = 0; y <= maxY; y++) {
        json_array_start();
        char last = 'X';
        int count = 0;
        int outputCount = 0;
        for (int x = 0; x < width; x++) {
            char mapVal = map_value(x, y) % 2 ? 'X' : ' ';
            if (mapVal == last) {
                count++;
                continue;
            }

            if (outputCount == 0 && last == ' ') fprintf(stderr, "-1, ");

            json_value(count);

            outputCount++;
            count = 1;
            last = mapVal;
        }

        // if (maxX < width) 
        json_array_end();
    }
}
/** Get the correct Act for a level */
int get_act(int levelCode) {
    if (levelCode < 40) return 0;
    if (levelCode < 75) return 1;
    if (levelCode < 103) return 2;
    if (levelCode < 109) return 3;
    if (levelCode < 200) return 4;
    return -1;
}

int d2_dump_map(unsigned int seed, int difficulty, int levelCode) {
    LevelTxt *levelData = d2common_get_level_text(gameVersion, levelCode); 
    if (!levelData) return 1;

    if (gameVersion == VersionPathOfDiablo) {
        switch (levelCode) {
            // Why are these levels broken?
            case 20:
            case 59:
            case 63:
            case 99:
                return 1;
            default:
                break;
        }
    } else if (gameVersion == VersionProjectDiablo2 && levelCode == 150) {
        return 1;
    } 

    int actId = get_act(levelCode);
    Act *pAct = d2common_load_act(gameVersion, actId, seed, difficulty); 
    if (!pAct) return 1;

    Level *pLevel = d2_get_level(pAct->pMisc, levelCode);  // Loading Town Level
    if (!pLevel) return 1;

    char *levelName = levelData->szName;

    if (!pLevel->pRoom2First) d2common_init_level(gameVersion, pLevel); 
    if (!pLevel->pRoom2First) {
        log_warn("Map:SkippingLevel:FailedRoomLoading", lk_i("mapId", levelCode), lk_s("mapName", levelName));
        return 1;
    }

    unsigned int originX = pLevel->dwPosX * 5;
    unsigned int originY = pLevel->dwPosY * 5;

    unsigned int mapWidth = pLevel->dwSizeX * 5;
    unsigned int mapHeight = pLevel->dwSizeY * 5;

    log_trace("MapInit", lk_i("actId", actId), lk_i("mapId", levelCode), lk_s("mapName", levelName), lk_i("originY", originY), lk_i("originX", originX), lk_i("width", mapWidth), lk_i("height", mapHeight));
    map_reset();

    // Start JSON DUMP
    json_start();
    json_key_value("type", "map");
    json_key_value("id", levelCode);
    json_key_value("name", levelName);

    json_object_start("offset");
    json_key_value("x", originX);
    json_key_value("y", originY);
    json_object_end();

    json_object_start("size");
    json_key_value("width", mapWidth);
    json_key_value("height", mapHeight);
    json_object_end();

    json_array_start("rooms");
    dump_rooms(pLevel);
    json_array_end();
    json_array_start("objects");

    for (Room2 *pRoom2 = pLevel->pRoom2First; pRoom2; pRoom2 = pRoom2->pRoom2Next) {
        BOOL bAdded = !pRoom2->pRoom1;

        if (bAdded) d2common_add_room_data(gameVersion, pAct, pLevel, pRoom2);
        dump_objects(pAct, pLevel, pRoom2);

        if (pRoom2->pRoom1) add_collision_data(pRoom2->pRoom1->Coll, originX, originY);
        if (bAdded) d2common_remove_room_data(gameVersion, pAct, pLevel, pRoom2);
    }

    json_array_end();
    json_array_start("map");
    dump_map_collision(mapWidth);
    json_array_end();
    json_end();
    return 0;
}
