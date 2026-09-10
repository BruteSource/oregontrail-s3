// End of the road — arrival in Oregon or the loss of the party. Shows the final
// score (FinalPoints.cs) and records it if it makes the Top Five.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "game/Scoring.h"
#include "ui/Screen.h"
#include "ui/Widgets.h"

class GameOverScreen : public Screen {
public:
    explicit GameOverScreen(bool won) : won_(won) {}
    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    bool        won_;
    game::Score score_{};
    int         rank_ = -1;
    ui::Rect    scores_{}, menu_{};
};
