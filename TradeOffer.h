//
// Created by Max on 31. 7. 2021.
//

#ifndef STEAMBOT_TRADEOFFER_H
#define STEAMBOT_TRADEOFFER_H


#include <cstdint>
#include <string>
#include <vector>

class TradeOffer {
public:
    struct OfferAsset {
        uint64_t appid;
        uint64_t contextid;
        uint64_t amount;
        uint64_t assetid;
    };
private:
    uint64_t partner;
    std::string token;
    std::vector<OfferAsset> ourItems;
    std::vector<OfferAsset> theirItems;
public:
    explicit TradeOffer(uint64_t partner, const std::string& token = "");
    void addOurItem(OfferAsset & item);
    void addTheirItem(OfferAsset & item);
    /**
     *
     * @return success
     */
    bool send();
};


#endif //STEAMBOT_TRADEOFFER_H
