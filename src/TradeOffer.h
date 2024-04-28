//
// Created by Max on 31. 7. 2021.
//

#ifndef STEAMBOT_TRADEOFFER_H
#define STEAMBOT_TRADEOFFER_H


#include <cstdint>
#include <string>
#include <vector>
#include "SteamCommunity.h"

class TradeOffer {
public:
    struct OfferAsset {
        uint32_t appid;
        uint64_t contextid;
        uint32_t amount;
        uint64_t assetid;
    };
private:
    uint64_t partner;
    std::string token;
    std::vector<OfferAsset> ourItems;
    std::vector<OfferAsset> theirItems;
public:
    std::string message;
    explicit TradeOffer(uint64_t partner, std::string  token = "");
    void addOurItem(OfferAsset & item);
    void addOurItem(SteamCommunity::InventoryItem & item, uint32_t count = 1);
    void addTheirItem(OfferAsset & item);
    void addTheirItem(SteamCommunity::InventoryItem & item, uint32_t count = 1);
    /**
     *
     * @return success
     */
    bool send(SteamCommunity & community);
};


#endif //STEAMBOT_TRADEOFFER_H
