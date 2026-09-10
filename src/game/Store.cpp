#include "game/Store.h"

#include <math.h>

namespace game {

int unitPrice(const StoreItem& it, int forts) {
    if (forts < 0) forts = 0;
    if (forts > 6) forts = 6;
    const float scaled = it.basePrice * (1.0f + 0.25f * forts);
    // Round to the cent, then the store quotes whole units; callers multiply by
    // quantity so keep sub-dollar precision by returning cents-accurate dollars.
    return (int)lroundf(scaled * 100.0f);   // price in CENTS per unit
}

void applyPurchase(Vehicle& v, ItemId id, int qty, int forts) {
    if (qty <= 0) return;
    int n;
    const StoreItem* cat = storeCatalog(&n);
    const StoreItem* it = nullptr;
    for (int i = 0; i < n; ++i)
        if (cat[i].id == id) { it = &cat[i]; break; }
    if (!it) return;

    const long costCents = (long)unitPrice(*it, forts) * qty;
    const int  costDollars = (int)((costCents + 99) / 100);   // round up to the dollar
    v.cash -= costDollars;

    switch (id) {
        case ItemId::Oxen:     v.oxen    += qty; break;
        case ItemId::Food:     v.food    += qty; break;
        case ItemId::Clothes:  v.clothes += qty; break;
        case ItemId::Bullets:  v.bullets += qty; break;
        case ItemId::Wheel:    v.wheels  += qty; break;
        case ItemId::Axle:     v.axles   += qty; break;
        case ItemId::Tongue:   v.tongues += qty; break;
        case ItemId::Medicine: v.medicine += qty; break;
        default: break;
    }
}

}  // namespace game
