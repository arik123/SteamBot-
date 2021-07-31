#include "CustomEMsgHandler.h"

#include <steammessages_clientserver_login.pb.h>
#include <steammessages_clientserver_2.pb.h>
//
// Created by Max on 31. 7. 2021.
//

bool emsgHandler (Steam::EMsg emsg, const unsigned char * data, size_t length, uint64_t job_id) {
    switch (emsg) {
        case Steam::EMsg::ClientEmailAddrInfo:
        {
            CMsgClientEmailAddrInfo info;
            info.ParseFromArray(data, length);
            std::cout << color(colorFG::Blue) << info.email_address() << color() << '\n';
            return true;
        }
    }
    return false;
}
