#include <cstdio>
#include <windows.h>

#pragma once

#include "log.h"

enum D2Version {
    VersionUnknown,
    VersionDiablo2,
    VersionPathOfDiablo,
    VersionProjectDiablo2
};

const CHAR* Path_PathOfDiablo = "Path of Diablo/";
const CHAR* Path_ProjectDiablo2 = "ProjectD2/";
const CHAR* Path_Diablo2 = "";

/** Convert the D2Version to the path that the files are normally located in */
const char* game_version_path(D2Version version) {
    if (version == VersionPathOfDiablo) return Path_PathOfDiablo;
    if (version == VersionProjectDiablo2) return Path_ProjectDiablo2;
    if (version == VersionDiablo2) return "";
    return nullptr;
}

/** Determine if a a version of the game exists in the path by checking for Game.exe */
bool game_version_exists(const char* folderName, D2Version version) {
    char gamePathExe[MAX_PATH];
    const char* gamePath = game_version_path(version);
    if (gamePath == nullptr) return false;

    sprintf(gamePathExe, "%s\\%sGame.exe", folderName, gamePath);

    std::ifstream ifs(gamePathExe, std::ifstream::in);
    bool found = !ifs;
    ifs.close();
    log_trace("Init:GamePath", lk_b("exists", !found), lk_s("game", gamePathExe));
    return !found;
}

/** Attempt to determine which mod is installed */
D2Version game_version(const char* folderName) {
    if (game_version_exists(folderName, VersionPathOfDiablo)) return VersionPathOfDiablo;
    if (game_version_exists(folderName, VersionProjectDiablo2)) return VersionProjectDiablo2;
    if (game_version_exists(folderName, VersionDiablo2)) return VersionDiablo2;
    return VersionUnknown;
}