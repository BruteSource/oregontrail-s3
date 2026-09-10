// The hub while on the trail — big touch buttons: press on, plus supplies, map,
// pace & rations, rest, and (in context) hunt / talk. (Travel.cs command menu.)
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class TrailMenuScreen : public Screen {
public:
    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    ui::Rect go_{}, supplies_{}, map_{}, paceRations_{}, rest_{};
    ui::Rect hunt_{}, talk_{}, settings_{}, abandon_{};
    bool     showHunt_ = false, showTalk_ = false;
};
