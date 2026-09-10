// General store catalog + pricing. Base prices are the 1985 original's, from
// ItemPrices.cs. Prices climb with distance west: base * (1 + 0.25*forts), forts
// capped at 6 -> 2.5x (StorePrice.cs). At Independence forts = 0, so base price.
#pragma once
#include <stdint.h>

#include "game/Model.h"

namespace game {

enum class ItemId : uint8_t { Oxen, Food, Clothes, Bullets, Wheel, Axle, Tongue, Medicine, COUNT };

struct StoreItem {
    ItemId      id;
    const char* label;
    const char* unit;      // shown after the quantity
    float       basePrice; // dollars per single unit
    int         lot;       // buy in multiples of this (yoke of oxen, box of bullets)
    int         buyStep;   // quantity added per "+" tap
};

// The eight lines Matt sells. Order is display order.
inline const StoreItem* storeCatalog(int* n) {
    static const StoreItem k[] = {
        {ItemId::Oxen,     "Oxen",          "oxen",  20.00f, 2,  2},
        {ItemId::Food,     "Food",          "lb",     0.20f, 1, 50},
        {ItemId::Clothes,  "Clothing",      "sets",  10.00f, 1,  1},
        {ItemId::Bullets,  "Ammunition",    "rounds", 0.10f, 20, 20},
        {ItemId::Wheel,    "Wagon wheel",   "",      10.00f, 1,  1},
        {ItemId::Axle,     "Wagon axle",    "",      10.00f, 1,  1},
        {ItemId::Tongue,   "Wagon tongue",  "",      10.00f, 1,  1},
        {ItemId::Medicine, "Medicine kit",  "kits",  15.00f, 1,  1},
    };
    *n = sizeof(k) / sizeof(k[0]);
    return k;
}

// Price of one unit at the given markup step count (forts passed, 0..6).
int unitPrice(const StoreItem& it, int forts);

// Must buy at least one yoke (2 oxen) before leaving the first store.
constexpr int kMinStartOxen = 2;

// Add `qty` of an item to the wagon and deduct its cost. Caller checks funds.
void applyPurchase(Vehicle& v, ItemId id, int qty, int forts);

}  // namespace game
