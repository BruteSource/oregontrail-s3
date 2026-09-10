#include "ui/ScreenStack.h"

#include <LovyanGFX.hpp>

void ScreenStack::destroy(Screen*& s) {
    delete s;
    s = nullptr;
}

void ScreenStack::push(Screen* s) {
    pendingOp_ = Op::Push;
    pendingScreen_ = s;
}
void ScreenStack::pop() {
    pendingOp_ = Op::Pop;
    pendingScreen_ = nullptr;
}
void ScreenStack::replace(Screen* s) {
    pendingOp_ = Op::Replace;
    pendingScreen_ = s;
}
void ScreenStack::reset(Screen* s) {
    pendingOp_ = Op::Reset;
    pendingScreen_ = s;
}

Screen* ScreenStack::top() const {
    return count_ > 0 ? stack_[count_ - 1] : nullptr;
}

void ScreenStack::applyPending() {
    if (pendingOp_ == Op::None) return;
    const Op op = pendingOp_;
    Screen* incoming = pendingScreen_;
    pendingOp_ = Op::None;
    pendingScreen_ = nullptr;

    switch (op) {
        case Op::Push:
            if (count_ < kMax && incoming) {
                if (Screen* t = top()) t->onExit();
                stack_[count_++] = incoming;
                incoming->onEnter();
            } else {
                delete incoming;  // stack full — drop it rather than leak
            }
            break;

        case Op::Pop:
            if (count_ > 0) {
                stack_[count_ - 1]->onExit();
                destroy(stack_[--count_]);
                if (Screen* t = top()) t->onEnter();
            }
            break;

        case Op::Replace:
            if (count_ > 0) {
                stack_[count_ - 1]->onExit();
                destroy(stack_[--count_]);
            }
            if (count_ < kMax && incoming) {
                stack_[count_++] = incoming;
                incoming->onEnter();
            } else {
                delete incoming;
            }
            break;

        case Op::Reset:
            while (count_ > 0) {
                stack_[count_ - 1]->onExit();
                destroy(stack_[--count_]);
            }
            if (incoming) {
                stack_[count_++] = incoming;
                incoming->onEnter();
            }
            break;

        case Op::None:
            break;
    }
}

void ScreenStack::update(uint32_t dtMs, LGFX_Sprite& g) {
    applyPending();
    if (Screen* t = top()) {
        t->tick(dtMs);
        // A tick may have queued a transition; settle it before rendering so the
        // frame shows the screen that is actually current.
        applyPending();
        if (Screen* t2 = top()) t2->render(g);
    }
}

void ScreenStack::dispatchTap(int16_t x, int16_t y) {
    if (Screen* t = top()) t->onTap(x, y);
}

void ScreenStack::dispatchTouchMove(int16_t x, int16_t y) {
    if (Screen* t = top()) t->onTouchMove(x, y);
}

void ScreenStack::dispatchTouchEnd() {
    if (Screen* t = top()) t->onTouchEnd();
}

void ScreenStack::dispatchKey(char c) {
    if (Screen* t = top()) t->onKey(c);
}
