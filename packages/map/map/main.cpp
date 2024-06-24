#include <unistd.h>

#include <iostream>

#include "log.h"
#include "d2_client.h"
#include "d2_structs.h"
#include "json.h"

enum
{
    INPUT_BUFFER = 1024
};

constexpr char COMMAND_EXIT[] = "$exit";
constexpr char COMMAND_MAP[] = "$map";
constexpr char COMMAND_DIFF[] = "$difficulty";
constexpr char COMMAND_ACT[] = "$act";
constexpr char COMMAND_SEED[] = "$seed";

bool starts_with(const char *prefix, const char *search_string) {
    if (strncmp(prefix, search_string, strlen(search_string)) == 0) return true;
    return false;
}

auto CharGray = "\33[90m";

auto CliUsage = "\nUsage:\n"
                "    d2-map.exe [D2 Game Path] [options]\n"
                "\nOptions:\n"
                "    --seed [-s]          Map Seed\n"
                "    --difficulty [-d]    Game Difficulty [0: Normal, 1: Nightmare, 2:Hell]\n"
                "    --act [-a]           Dump a specific act [0: ActI, 1:ActII, 2: ActIII, 3: ActIV, 4: Act5]\n"
                "    --map [-m]           Dump a specific Map [0: Rogue Encampent ...]\n"
                "    --verbose [-v]       Increase logging level\n"

                "\nExamples:\n"
                "\n    \33[90m# Dump ActI from Normal mode for seed 1122334 \033[0m\n"
                "    d2-map.exe /home/diablo2 --seed 1122334 --difficulty 0 --act 0\n"
                "\n    \33[90m# Dump all acts from Hell mode for seed 1122334 \033[0m\n"
                "    d2-map.exe /home/diablo2 --seed 1122334 --difficulty 2\n";



void dump_info(unsigned int seed, int difficulty, int actId, int mapId) {
    json_start();
    json_key_value("type", "info");
    json_key_value("seed", seed);
    json_key_value("difficulty", difficulty);
    if (actId > -1) json_key_value("act", actId);
    if (mapId > -1) json_key_value("map", mapId);
    json_end();
}


void dump_maps(const unsigned int p_seed, const int p_difficulty, const int p_actId, const int p_mapId) {
    const int64_t totalTime = currentTimeMillis();
    int mapCount = 0;
    if (p_mapId > -1) {
        const int64_t startTime = currentTimeMillis();
        const int res = d2_dump_map(p_seed, p_difficulty, p_mapId);
        if (res == 0) mapCount ++;
        const int64_t duration = currentTimeMillis() - startTime;
        log_debug("Map:Generation", lk_ui("p_seed", p_seed), lk_i("difficulty", p_difficulty), lk_i("mapId", p_mapId), lk_i("duration", duration));
    } else {
        for (int mapId = 0; mapId < 200; mapId++) {
            // Skip map if its not part of the current act
            if (p_actId > -1 && get_act(mapId) != p_actId) continue;

            const int64_t startTime = currentTimeMillis();
            const int res = d2_dump_map(p_seed, p_difficulty, mapId);
            if (res == 0) mapCount ++;
            if (res == 1) continue; // Failed to generate the map

            const int64_t currentTime = currentTimeMillis();
            const int64_t duration = currentTime - startTime;
            log_debug("Map:Generation", lk_ui("p_seed", p_seed), lk_i("difficulty", p_difficulty), lk_i("actId", get_act(mapId)), lk_i("mapId", mapId), lk_i("duration", duration));
        }
    }
    const int64_t duration = currentTimeMillis() - totalTime;
    log_info("Map:Generation:Done", lk_ui("p_seed", p_seed), lk_i("difficulty", p_difficulty), lk_i("count", mapCount), lk_i("duration", duration));
    printf("\n");
}


int main(const int argc, char *argv[]) {
    if (argc < 1) {
        printf("%s", CliUsage);
        return 1;
    }
    log_info("Cli:Start", lk_s("version", GIT_VERSION), lk_s("hash", GIT_HASH));

    char c[INPUT_BUFFER];

    const char *gameFolder = nullptr;
    DWORD argSeed = 0xff00ff00;
    int argMapId = -1;
    int argDifficulty = 0;
    int argActId = -1;
    int foundArgs = 0;
    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (starts_with(arg, "--seed") || starts_with(arg, "-s")) {
            (void)sscanf_s(argv[++i], "%u", &argSeed);
            log_debug("Cli:Arg", lk_ui("seed", argSeed));
            foundArgs ++;
        } else if (starts_with(arg, "--difficulty") || starts_with(arg, "-d")) {
            argDifficulty = atoi(argv[++i]);
            log_debug("Cli:Arg", lk_i("difficulty", argDifficulty));
            foundArgs ++;
        } else if (starts_with(arg, "--map") || starts_with(arg, "-m")) {
            argMapId = atoi(argv[++i]);
            log_debug("Cli:Arg", lk_i("mapId", argMapId));
            foundArgs ++;
        } else if (starts_with(arg, "--act") || starts_with(arg, "-a")) {
            argActId = atoi(argv[++i]);
            log_debug("Cli:Arg", lk_i("actId", argActId));
            foundArgs ++;
        } else if (starts_with(arg, "--verbose") || starts_with(arg, "-v")) {
            log_debug("Cli:Arg", lk_b("verbose", true));
            log_level(LOG_TRACE);
        } else {
            gameFolder = arg;
            log_debug("Cli:Arg", lk_s("game", gameFolder));
        }
    }


    if (argActId > 0 && argMapId > 0) {
        printf("--act and --map cannot be used together\n");
        printf("%s", CliUsage);
        return 1;
    }
    if (gameFolder == nullptr) {
        printf("%s", CliUsage);
        return 1;
    }


    const int64_t initStartTime = currentTimeMillis();
    d2_game_init(gameFolder);
    const int64_t duration = currentTimeMillis() - initStartTime;
    log_info("Map:Init:Done", lk_s("version", GIT_VERSION), lk_s("hash", GIT_HASH), lk_i("duration", duration));

    /** Seed/Diff has been passed in just generate the map that is required */
    if (foundArgs > 0) {
        if (argMapId > -1) dump_maps(argSeed, argDifficulty, -1, argMapId);
        else if (argActId > -1) dump_maps(argSeed, argDifficulty, argActId, -1);
        else dump_maps(argSeed, argDifficulty, -1, -1);
        return 0;
    }

    /** Init the D2 client using the provided path */
    json_start();
    json_key_value("type", "init");
    json_end();
    char buffer[INPUT_BUFFER];

    /** Read in seed/Difficulty then generate all the maps */
    while (fgets(buffer, INPUT_BUFFER, stdin) != nullptr) {
        int rtn;
        if (starts_with(buffer, COMMAND_EXIT) == 1) return 0;

        if (starts_with(buffer, COMMAND_MAP) == 1) {
            dump_maps(argSeed, argDifficulty, argActId, argMapId);
            argActId = -1;
            argMapId = -1;
            json_start();
            json_key_value("type", "done");
            json_end();
        } else if (starts_with(buffer, COMMAND_SEED) == 1) {
            rtn = sscanf_s(buffer, "%s %u", &c, &argSeed);
            dump_info(argSeed, argDifficulty, argActId, argMapId);
        } else if (starts_with(buffer, COMMAND_DIFF) == 1) {
            rtn = sscanf_s(buffer, "%s %d", &c, &argDifficulty);
            dump_info(argSeed, argDifficulty, argActId, argMapId);
        } else if (starts_with(buffer, COMMAND_ACT) == 1) {
            rtn = sscanf_s(buffer, "%s %d", &c, &argActId);
            dump_info(argSeed, argDifficulty, argActId, argMapId);
        }
        printf("\n");
    }

    return 0;
}


