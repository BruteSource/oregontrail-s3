// Matt's General Store in Independence. Buy the outfit for the trip: oxen, food,
// clothing, ammunition, spare parts, medicine. Must leave with at least one yoke
// of oxen. Prices are base (no markup at the first store).
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class StoreScreen : public Screen {
public:
    // firstStore: the outfitting stop at Independence (must buy oxen, leads into
    // the trail). Otherwise a fort store mid-journey: prices marked up, no
    // requirements, returns to where you were.
    explicit StoreScreen(bool firstStore = true) : first_(firstStore) {}

    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    bool     first_;
    int      qty_[8] = {0};
    ui::Rect minus_[8], plus_[8];
    ui::Rect leave_{};

    int  forts() const;
    long spentCents() const;
    int  oxenQty() const;
};
