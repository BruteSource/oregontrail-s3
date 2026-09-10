#include "screens/StoreScreen.h"

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "game/Store.h"
#include "screens/MessageScreen.h"
#include "screens/OutfitDoneScreen.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

void StoreScreen::onEnter() {
    for (auto& q : qty_) q = 0;
}

int StoreScreen::forts() const {
    // Independence quotes base prices; a fort store marks up by how far west.
    return first_ ? 0 : game::sim.fortsReached;
}

long StoreScreen::spentCents() const {
    int n;
    const game::StoreItem* cat = game::storeCatalog(&n);
    long cents = 0;
    for (int i = 0; i < n; ++i)
        cents += (long)game::unitPrice(cat[i], forts()) * qty_[i];
    return cents;
}

int StoreScreen::oxenQty() const {
    int n;
    const game::StoreItem* cat = game::storeCatalog(&n);
    for (int i = 0; i < n; ++i)
        if (cat[i].id == game::ItemId::Oxen) return qty_[i];
    return 0;
}

void StoreScreen::render(LGFX_Sprite& g) {
    int n;
    const game::StoreItem* cat = game::storeCatalog(&n);
    const int cash = game::g.vehicle.cash;
    const int spent = (int)((spentCents() + 99) / 100);
    const int left = cash - spent;

    g.fillScreen(theme::BG);

    // storefront strip: the Independence street scene, top 44 px
    g.drawPng(art::lm_0, art::lm_0_len, 0, 0, 320, 44);
    g.fillRect(0, 0, 320, 17, theme::BG);
    ui::drawCentered(g, first_ ? "MATT'S GENERAL STORE" : "TRADING POST", 160, 1,
                     theme::ACCENT, 2);

    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::top_center);
    g.fillRect(0, 44, 320, 12, theme::BG);
    g.setTextColor(left < 0 ? theme::WARN : theme::INK);
    g.drawString(String("cash $") + cash + "    spent $" + spent +
                 "    left $" + left, 160, 44);

    const int16_t rowTop = 58, rowH = 17;
    for (int i = 0; i < n; ++i) {
        const int16_t y = rowTop + i * rowH;
        const int priceCents = game::unitPrice(cat[i], forts());

        g.setTextDatum(textdatum_t::middle_left);
        g.setTextColor(theme::INK);
        g.drawString(cat[i].label, theme::MARGIN, y + rowH / 2);

        char pr[16];
        snprintf(pr, sizeof(pr), "$%d.%02d", priceCents / 100, priceCents % 100);
        g.setTextColor(theme::INK_DIM);
        g.setTextDatum(textdatum_t::middle_right);
        g.drawString(pr, 176, y + rowH / 2);

        minus_[i] = {184, (int16_t)(y + 1), 26, (int16_t)(rowH - 3)};
        plus_[i]  = {250, (int16_t)(y + 1), 26, (int16_t)(rowH - 3)};
        ui::drawButton(g, minus_[i], "-");
        ui::drawButton(g, plus_[i], "+");

        g.setTextDatum(textdatum_t::middle_center);
        g.setTextColor(theme::INK);
        g.drawString(String(qty_[i]), 230, y + rowH / 2);

        const long lc = (long)priceCents * qty_[i];
        if (lc > 0) {
            char lcs[16];
            snprintf(lcs, sizeof(lcs), "$%ld", (lc + 99) / 100);
            g.setTextDatum(textdatum_t::middle_right);
            g.setTextColor(theme::INK_DIM);
            g.drawString(lcs, 316, y + rowH / 2);
        }
    }

    const bool needOxen = first_ && oxenQty() < game::kMinStartOxen;
    const bool ok = !needOxen && (left >= 0);
    leave_ = {(int16_t)(160 - 96), 198, 192, 30};
    const char* label = (left < 0) ? "Over budget"
                        : needOxen ? "Buy a yoke of oxen"
                        : first_   ? "Leave the store"
                                   : "Done trading";
    ui::drawButton(g, leave_, label, ok, ok);
}

void StoreScreen::onTap(int16_t x, int16_t y) {
    int n;
    const game::StoreItem* cat = game::storeCatalog(&n);

    for (int i = 0; i < n; ++i) {
        if (minus_[i].contains(x, y)) {
            qty_[i] -= cat[i].buyStep;
            if (qty_[i] < 0) qty_[i] = 0;
            return;
        }
        if (plus_[i].contains(x, y)) {
            const int step = cat[i].buyStep;
            const long addCents = (long)game::unitPrice(cat[i], forts()) * step;
            if ((spentCents() + addCents + 99) / 100 <= game::g.vehicle.cash)
                qty_[i] += step;
            return;
        }
    }

    if (leave_.contains(x, y)) {
        if (first_ && oxenQty() < game::kMinStartOxen) {
            app::screens.push(new MessageScreen(
                "Hold on",
                "You can't pull a wagon without oxen. Buy at least one yoke "
                "(2) before you leave."));
            return;
        }
        if (game::g.vehicle.cash - (int)((spentCents() + 99) / 100) < 0) return;

        for (int i = 0; i < n; ++i)
            if (qty_[i] > 0)
                game::applyPurchase(game::g.vehicle, cat[i].id, qty_[i], forts());

        if (first_) app::screens.replace(new OutfitDoneScreen());
        else        app::screens.pop();
    }
}
