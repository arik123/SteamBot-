//
// Created by Max on 31. 7. 2021.
//

#ifndef STEAMBOT_CUSTOMEMSGHANDLER_H
#define STEAMBOT_CUSTOMEMSGHANDLER_H
#include "../lib/SteamPP/include/steam++.h"
#include "../lib/SteamPP/include/consoleColor.h"
bool emsgHandler (Steam::EMsg emsg, const unsigned char * data, size_t length, uint64_t job_id);

#endif //STEAMBOT_CUSTOMEMSGHANDLER_H
