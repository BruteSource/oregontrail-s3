// Name the five travellers. Slot 0 is the leader. Each slot can be typed on the
// on-screen keyboard, picked from a preset list, or left to a random fill.
#pragma once
#include <Arduino.h>

#include <LovyanGFX.hpp>

#include "ui/Screen.h"
#include "ui/Widgets.h"

class NameEntryScreen : public Screen {
public:
    static bool devStartInKeyboard;   // set by the 'k' dev command

    void onEnter() override;
    void render(LGFX_Sprite& g) override;
    void onTap(int16_t x, int16_t y) override;

private:
    enum class Mode { Menu, Keyboard, Picker };

    Mode    mode_ = Mode::Menu;
    int     slot_ = 0;
    char    names_[5][16] = {};
    String  draft_;
    int     pickerPage_ = 0;

    ui::Rect btnType_{}, btnPick_{}, btnRandom_{}, btnBack_{};
    ui::Rect pickerCells_[12];
    ui::Rect pickerPrev_{}, pickerNext_{}, typeInstead_{};

    void commit(const String& name);   // set current slot, advance or finish
    void randomFillRest();
    void finish();
    void drawPartyList(LGFX_Sprite& g, int16_t y) const;
};
