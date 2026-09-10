// A modal notice: heading, word-wrapped body, single dismiss button that pops
// back. Used as a placeholder for not-yet-built features in Phase 1 and as the
// event/dialog presenter later.
#pragma once
#include <Arduino.h>

#include "ui/App.h"
#include "ui/Screen.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <LovyanGFX.hpp>

class MessageScreen : public Screen {
public:
    MessageScreen(const String& heading, const String& body,
                  const String& button = "OK")
        : heading_(heading), body_(body), button_(button) {}

    void render(LGFX_Sprite& g) override {
        g.fillScreen(theme::BG);
        ui::Rect panel{theme::MARGIN, theme::MARGIN,
                       (int16_t)(320 - 2 * theme::MARGIN),
                       (int16_t)(240 - 2 * theme::MARGIN)};
        ui::drawPanel(g, panel);

        ui::drawCentered(g, heading_, 160, panel.y + 12, theme::ACCENT, 4);
        ui::drawText(g, body_, panel.x + 12, panel.y + 48, panel.w - 24,
                     theme::INK, 2, theme::LINE_H);

        btn_ = ui::Rect{(int16_t)(160 - 70), (int16_t)(panel.y + panel.h - 46),
                        140, 30};
        ui::drawButton(g, btn_, button_, true);
    }

    void onTap(int16_t x, int16_t y) override {
        if (btn_.contains(x, y)) app::screens.pop();
    }

private:
    String heading_, body_, button_;
    ui::Rect btn_{};
};
