// A parting of the ways. The two roads out of a fork node, with the historic
// trade-off spelled out. Choosing sets the branch and returns to travelling.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class ForkScreen : public Screen {
public:
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    ui::Rect a_{}, b_{};
};
