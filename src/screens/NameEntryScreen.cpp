#include "screens/NameEntryScreen.h"

#include <ctype.h>

#include "game/Names.h"
#include "game/Session.h"
#include "screens/MainMenuScreen.h"
#include "screens/StoreScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
constexpr int kPickerPerPage = 12;   // 3 cols x 4 rows
}

bool NameEntryScreen::devStartInKeyboard = false;

void NameEntryScreen::onEnter() {
    mode_ = devStartInKeyboard ? Mode::Keyboard : Mode::Menu;
    devStartInKeyboard = false;
    slot_ = 0;
    draft_ = "";
    pickerPage_ = 0;
    for (auto& n : names_) n[0] = 0;
}

void NameEntryScreen::commit(const String& name) {
    String n = name;
    n.trim();
    if (n.length() == 0) {
        int cnt;
        const char* const* pool = game::presetNames(&cnt);
        n = pool[game::rngRange(0, cnt)];
    }
    if (n.length() > 15) n = n.substring(0, 15);
    strncpy(names_[slot_], n.c_str(), 15);
    names_[slot_][15] = 0;
    draft_ = "";
    mode_ = Mode::Menu;
    if (++slot_ >= 5) finish();
}

void NameEntryScreen::randomFillRest() {
    int cnt;
    const char* const* pool = game::presetNames(&cnt);
    for (int s = slot_; s < 5; ++s) {
        // avoid duplicates within the party
        for (int tries = 0; tries < 20; ++tries) {
            const char* cand = pool[game::rngRange(0, cnt)];
            bool dup = false;
            for (int j = 0; j < s; ++j)
                if (strcmp(names_[j], cand) == 0) { dup = true; break; }
            if (!dup || tries == 19) { strncpy(names_[s], cand, 15); names_[s][15] = 0; break; }
        }
    }
    slot_ = 5;
    finish();
}

void NameEntryScreen::finish() {
    game::Party& p = game::g.party;
    p.count = 5;
    for (int i = 0; i < 5; ++i) {
        p.member[i] = game::Person{};
        p.member[i].setName(names_[i][0] ? names_[i] : "Traveller");
    }
    app::screens.replace(new StoreScreen());
}

void NameEntryScreen::drawPartyList(LGFX_Sprite& g, int16_t y) const {
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_left);
    for (int i = 0; i < 5; ++i) {
        const bool cur = (i == slot_);
        g.setTextColor(cur ? theme::ACCENT : (names_[i][0] ? theme::INK : theme::INK_DIM));
        String line = String(i + 1) + ". ";
        if (names_[i][0]) line += names_[i];
        else if (cur) line += draft_.length() ? draft_ + "_" : String("...");
        else line += "-";
        if (i == 0) line += "  (leader)";
        g.drawString(line, theme::MARGIN + 4, y + i * 16);
    }
}

void NameEntryScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);
    ui::drawCentered(g, "NAME YOUR PARTY", 160, 6, theme::ACCENT, 2);

    if (mode_ == Mode::Menu) {
        drawPartyList(g, 30);
        String q = (slot_ == 0) ? "Enter the leader's name:"
                                : "Enter traveller " + String(slot_ + 1) + "'s name:";
        g.setTextColor(theme::INK);
        g.setTextDatum(textdatum_t::top_left);
        g.drawString(q, theme::MARGIN + 4, 118);

        btnType_   = {theme::MARGIN, 146, 150, 32};
        btnPick_   = {166, 146, 146, 32};
        btnRandom_ = {theme::MARGIN, 186, 150, 32};
        btnBack_   = {166, 186, 146, 32};
        ui::drawButton(g, btnType_, "Type it");
        ui::drawButton(g, btnPick_, "Pick from list");
        ui::drawButton(g, btnRandom_, "Random the rest");
        ui::drawButton(g, btnBack_, slot_ > 0 ? "Back a name" : "Start over");
        return;
    }

    if (mode_ == Mode::Keyboard) {
        g.setFont(&fonts::Font2);
        g.setTextColor(theme::INK_DIM);
        g.setTextDatum(textdatum_t::top_center);
        g.drawString(slot_ == 0 ? "name the leader"
                                : ("name traveller " + String(slot_ + 1)),
                     160, 22);

        // draft field
        g.fillRoundRect(40, 38, 240, 30, 4, theme::PANEL);
        g.drawRoundRect(40, 38, 240, 30, 4, theme::FRAME);
        g.setFont(&fonts::Font4);
        g.setTextDatum(textdatum_t::middle_center);
        if (draft_.length()) {
            g.setTextColor(theme::INK);
            g.drawString(draft_ + "_", 160, 53);
        } else {
            g.setTextColor(theme::INK_DIM);
            g.drawString("type a name", 160, 53);
        }
        ui::keyboard::render(g);
        return;
    }

    // Picker
    int cnt;
    const char* const* pool = game::presetNames(&cnt);
    const int pages = (cnt + kPickerPerPage - 1) / kPickerPerPage;
    if (pickerPage_ >= pages) pickerPage_ = 0;

    g.setTextDatum(textdatum_t::middle_center);
    for (int i = 0; i < kPickerPerPage; ++i) {
        const int idx = pickerPage_ * kPickerPerPage + i;
        const int col = i % 3, row = i / 3;
        ui::Rect r{(int16_t)(theme::MARGIN + col * 102), (int16_t)(30 + row * 40),
                   98, 34};
        pickerCells_[i] = r;
        if (idx >= cnt) { pickerCells_[i] = {0, 0, 0, 0}; continue; }
        ui::drawButton(g, r, pool[idx]);
    }
    pickerPrev_ = {theme::MARGIN, 200, 84, 30};
    pickerNext_ = {228, 200, 84, 30};
    ui::Rect typeInstead{100, 200, 120, 30};
    ui::drawButton(g, pickerPrev_, "<");
    ui::drawButton(g, typeInstead, "custom");
    ui::drawButton(g, pickerNext_, ">");
    typeInstead_ = typeInstead;
    g.setTextColor(theme::INK_DIM);
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::middle_center);
    g.drawString(String("page ") + (pickerPage_ + 1) + " / " + pages, 160, 190);
}

void NameEntryScreen::onTap(int16_t x, int16_t y) {
    if (mode_ == Mode::Menu) {
        if (btnType_.contains(x, y))      { mode_ = Mode::Keyboard; draft_ = ""; }
        else if (btnPick_.contains(x, y)) { mode_ = Mode::Picker; }
        else if (btnRandom_.contains(x, y)) randomFillRest();
        else if (btnBack_.contains(x, y)) {
            if (slot_ > 0) { --slot_; names_[slot_][0] = 0; }
            else app::screens.reset(new MainMenuScreen());
        }
        return;
    }

    if (mode_ == Mode::Keyboard) {
        const char c = ui::keyboard::hitTest(x, y);
        if (c == '\n')       commit(draft_);
        else if (c == '\b')  { if (draft_.length()) draft_.remove(draft_.length() - 1); }
        else if (c == ' ')   { if (draft_.length() && draft_.length() < 15) draft_ += ' '; }
        else if (c >= 'A' && c <= 'Z') {
            if (draft_.length() < 15)
                draft_ += (draft_.length() == 0) ? c : (char)tolower(c);
        }
        return;
    }

    // Picker
    for (int i = 0; i < kPickerPerPage; ++i) {
        if (pickerCells_[i].w && pickerCells_[i].contains(x, y)) {
            int cnt;
            const char* const* pool = game::presetNames(&cnt);
            const int idx = pickerPage_ * kPickerPerPage + i;
            if (idx < cnt) commit(pool[idx]);
            return;
        }
    }
    if (pickerPrev_.contains(x, y)) { if (pickerPage_ > 0) --pickerPage_; }
    else if (pickerNext_.contains(x, y)) ++pickerPage_;
    else if (typeInstead_.contains(x, y)) { mode_ = Mode::Keyboard; draft_ = ""; }
}
