// Base class for a full-screen view. Mirrors the WolfCurses Window/Form idea
// stripped to essentials: only the top-of-stack screen is ticked and rendered,
// and it receives touch and on-screen-keyboard input.
#pragma once
#include <stdint.h>

#include <LovyanGFX.hpp>   // LGFX_Sprite is a LovyanGFX type alias, not forward-declarable

class Screen {
public:
    virtual ~Screen() = default;

    // Called once when this screen becomes the top of the stack (including when
    // it is uncovered by a pop).
    virtual void onEnter() {}
    // Called once when it stops being the top (covered by a push, or removed).
    virtual void onExit() {}

    // Advance animation / timers. dtMs is wall time since the last tick.
    virtual void tick(uint32_t dtMs) { (void)dtMs; }

    // Draw the whole screen into `g` (a 320x240 sprite). Called every frame.
    virtual void render(LGFX_Sprite& g) = 0;

    // A tap (short, stationary press then release) at screen coords (x, y).
    virtual void onTap(int16_t x, int16_t y) { (void)x; (void)y; }

    // The finger is down at (x, y) — sent every frame while touched, for
    // drag-style controls. Ends with onTouchEnd().
    virtual void onTouchMove(int16_t x, int16_t y) { (void)x; (void)y; }
    virtual void onTouchEnd() {}

    // A character from the on-screen keyboard. '\b' = backspace, '\n' = done.
    virtual void onKey(char c) { (void)c; }
};
