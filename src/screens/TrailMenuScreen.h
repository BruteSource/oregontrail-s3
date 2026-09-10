// The hub while on the trail. Shows where the party is and offers the day-to-day
// choices (Travel.cs command menu): press on, check supplies, look at the map,
// change pace or rations, rest. Talk-to-people appears at settlements/landmarks.
#pragma once
#include <Arduino.h>
#include <vector>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class TrailMenuScreen : public Screen {
public:
    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    ui::MenuList      menu_;
    std::vector<int>  act_;   // action id per visible row
    void rebuildMenu();
};
