// Owns the screen stack and routes frame/tick/input to the top screen.
// Transitions requested from inside a screen callback are deferred and applied
// at the next frame boundary, so a screen can safely ask to replace itself.
#pragma once
#include <stdint.h>

#include "ui/Screen.h"

class ScreenStack {
public:
    // Transition requests. `adopt`ed screens are owned and deleted by the stack.
    void push(Screen* s);      // cover the current top
    void pop();                // remove the top, uncover the one below
    void replace(Screen* s);   // pop then push, in one step
    void reset(Screen* s);     // clear everything, then push

    Screen* top() const;
    int depth() const { return count_; }

    // Call once per frame from loop(): applies pending transitions, ticks the
    // top screen, then renders it into `g`. Does not push `g` to the panel.
    void update(uint32_t dtMs, LGFX_Sprite& g);

    void dispatchTap(int16_t x, int16_t y);
    void dispatchTouchMove(int16_t x, int16_t y);
    void dispatchTouchEnd();
    void dispatchKey(char c);

private:
    enum class Op : uint8_t { None, Push, Pop, Replace, Reset };

    static constexpr int kMax = 8;
    Screen* stack_[kMax] = {};
    int     count_ = 0;

    Op      pendingOp_ = Op::None;
    Screen* pendingScreen_ = nullptr;

    void applyPending();
    void destroy(Screen*& s);
};
