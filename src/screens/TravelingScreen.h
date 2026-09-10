// The wagon rolling. Each short animation beat is one simulated day
// (Sim::takeTurn). Runs until the party arrives somewhere, something stops them,
// or the traveller taps to pull up.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"

class TravelingScreen : public Screen {
public:
    void onEnter() override;
    void tick(uint32_t dtMs) override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    uint32_t accum_ = 0;
    uint32_t animMs_ = 0;
    int      wheelPhase_ = 0;
    bool     halted_ = false;
    String   note_;
    void resolve();   // act on the last turn result
};
