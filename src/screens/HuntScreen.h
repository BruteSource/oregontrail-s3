// The hunt, adapted from the disassembled `& HUNT` (HuntGame.cs). The hunter
// stands in the field; aim is eight compass points and turning takes time
// proportional to how far round you swing (one step per few ticks, the short
// way). Tap where you want to shoot; a bullet flies when the hunter comes on
// line. Up to 100 lb of meat comes back to the wagon; every shot spends a round.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class HuntScreen : public Screen {
public:
    void onEnter() override;
    void tick(uint32_t dtMs) override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;
    void onTouchMove(int16_t x, int16_t y) override;

private:
    static constexpr int kMaxBul = 5;
    static constexpr int kSlots = 2;
    static constexpr int kMaxCarc = 8;

    struct Bullet { float x, y, dx, dy; bool live; };
    struct Animal { float x, y, vx, vy; uint8_t kind; bool alive; int lb; };

    int      aim_ = 0;          // 0..7, N clockwise
    int      target_ = -1;      // requested octant
    int      rotTicks_ = 0;     // ticks until the next turn step

    Bullet   bul_[kMaxBul]{};
    Animal   ani_[kSlots]{};
    int16_t  carcX_[kMaxCarc]{}, carcY_[kMaxCarc]{};
    uint8_t  carcKind_[kMaxCarc]{};
    int      carc_ = 0;

    int      meat_ = 0;         // lb shot, on the ground
    int      shots_ = 0;        // rounds spent
    uint32_t timeLeftMs_ = 0;
    bool     over_ = false;
    uint32_t accum_ = 0;
    uint32_t animMs_ = 0;

    ui::Rect done_{};

    void step();                // one hunt tick
    void spawn(int slot);
    void fire();
    bool anyBulletLive() const;
};
