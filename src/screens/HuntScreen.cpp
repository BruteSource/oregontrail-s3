#include "screens/HuntScreen.h"

#include <math.h>

#include "art/Art.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "screens/Hud.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

namespace {
// aim vectors indexed clockwise from North (HuntGame.AimVectors, y flipped for
// screen space: +y is down)
const float AIMX[8] = {0, 0.9f, 1, 0.9f, 0, -0.9f, -1, -0.9f};
const float AIMY[8] = {-1, -0.9f, 0, 0.9f, 1, 0.9f, 0, -0.9f};

constexpr int   FIELD_TOP = 22;
constexpr int   HUNT_MS = 22000;          // hunt length
constexpr int   TICK_MS = 33;
constexpr int   ROT_TICKS = 3;            // ticks per turn step (TicksPerRotationStep)
constexpr float BULLET_SPD = 8.5f;
constexpr int   FIELD_BOT = 240;

int centerX() { return 160; }
int centerY() { return (FIELD_TOP + FIELD_BOT) / 2 + 10; }

int shortWay(int from, int to) {
    int d = to - from;
    while (d > 4) d -= 8;
    while (d < -4) d += 8;
    return d;   // -4..4, sign = direction
}
}  // namespace

void HuntScreen::onEnter() {
    aim_ = 0;
    target_ = -1;
    rotTicks_ = 0;
    carc_ = 0;
    meat_ = 0;
    shots_ = 0;
    over_ = false;
    accum_ = 0;
    timeLeftMs_ = HUNT_MS;
    for (auto& b : bul_) b.live = false;
    for (int i = 0; i < kSlots; ++i) { ani_[i].alive = false; spawn(i); }
}

void HuntScreen::spawn(int slot) {
    if (carc_ >= 4) return;              // CarcassSpawnBlock
    Animal& a = ani_[slot];
    const int edge = game::rngRange(0, 4);
    const int w = 320, t = FIELD_TOP, b = FIELD_BOT;
    if (edge == 0)      { a.x = 4;       a.y = game::rngRange(t + 8, b - 8); }
    else if (edge == 1) { a.x = w - 4;   a.y = game::rngRange(t + 8, b - 8); }
    else if (edge == 2) { a.x = game::rngRange(20, w - 20); a.y = t + 4; }
    else               { a.x = game::rngRange(20, w - 20); a.y = b - 4; }
    const float sp = 0.35f + game::rngRange(0, 100) / 200.0f;
    a.vx = (game::rngRange(0, 2) ? sp : -sp);
    a.vy = (game::rngRange(0, 2) ? sp : -sp) * 0.55f;
    const int roll = game::rngRange(0, 100);
    if (roll < 55)      { a.kind = 0; a.lb = 2 + game::rngRange(0, 3); }   // rabbit
    else if (roll < 90) { a.kind = 1; a.lb = 30 + game::rngRange(0, 25); } // deer
    else                { a.kind = 2; a.lb = 100; }                        // bison
    a.alive = true;
}

void HuntScreen::fire() {
    if (over_ || game::g.vehicle.bullets - shots_ <= 0) return;
    for (auto& b : bul_) {
        if (b.live) continue;
        b.x = centerX();
        b.y = centerY();
        b.dx = AIMX[aim_] * BULLET_SPD;
        b.dy = AIMY[aim_] * BULLET_SPD;
        b.live = true;
        ++shots_;
        return;
    }
}

void HuntScreen::step() {
    if (over_) return;

    // swing toward the octant the finger asked for — the short way, one step
    // per ROT_TICKS. This pacing is the whole difficulty of the hunt.
    if (target_ >= 0 && target_ != aim_) {
        if (--rotTicks_ <= 0) {
            rotTicks_ = ROT_TICKS;
            const int d = shortWay(aim_, target_);
            aim_ = (aim_ + (d > 0 ? 1 : -1) + 8) % 8;
        }
    }

    // bullets
    for (auto& b : bul_) {
        if (!b.live) continue;
        b.x += b.dx;
        b.y += b.dy;
        if (b.x < 0 || b.x > 320 || b.y < FIELD_TOP || b.y > FIELD_BOT) {
            b.live = false;
            continue;
        }
        for (auto& a : ani_) {
            if (!a.alive) continue;
            const int hitR = a.kind == 0 ? 7 : a.kind == 1 ? 10 : 14;
            if (fabsf(a.x - b.x) < hitR && fabsf(a.y - b.y) < hitR - 2) {
                a.alive = false;
                b.live = false;
                meat_ += a.lb;
                if (carc_ < kMaxCarc) {
                    carcX_[carc_] = (int16_t)a.x;
                    carcY_[carc_] = (int16_t)a.y;
                    carcKind_[carc_] = a.kind;
                    ++carc_;
                }
            }
        }
    }

    // animals wander
    for (int i = 0; i < kSlots; ++i) {
        Animal& a = ani_[i];
        if (!a.alive) {
            if (game::rngRange(0, 1000) < 22) spawn(i);   // SpawnChance-ish
            continue;
        }
        a.x += a.vx;
        a.y += a.vy;
        if (a.y < FIELD_TOP + 3 || a.y > FIELD_BOT - 3) a.vy = -a.vy;
        if (a.x < -6 || a.x > 326) a.alive = false;       // wandered off
        if (game::rngRange(0, 100) < 3) a.vx = -a.vx;
    }

    if (timeLeftMs_ <= TICK_MS) {
        timeLeftMs_ = 0;
        over_ = true;
    } else {
        timeLeftMs_ -= TICK_MS;
    }
    if (game::g.vehicle.bullets - shots_ <= 0 && !anyBulletLive()) over_ = true;
}

bool HuntScreen::anyBulletLive() const {
    for (auto& b : bul_) if (b.live) return true;
    return false;
}

void HuntScreen::tick(uint32_t dtMs) {
    animMs_ += dtMs;
    accum_ += dtMs;
    while (accum_ >= TICK_MS) {
        accum_ -= TICK_MS;
        step();
    }
}

namespace {
// screen point -> octant (0 = N, clockwise), or -1 if too close to centre
int octantOf(float dx, float dy) {
    if (fabsf(dx) < 6 && fabsf(dy) < 6) return -1;
    float ang = atan2f(dy, dx);                        // 0 = east, +y down
    int oct = (int)lroundf((ang + (float)M_PI / 2) / ((float)M_PI / 4));
    return ((oct % 8) + 8) % 8;
}
}  // namespace

// Drag a finger around the ring to swing the aim; the barrel chases your finger
// at the fixed turn rate.
void HuntScreen::onTouchMove(int16_t x, int16_t y) {
    if (over_) return;
    if (done_.contains(x, y)) return;
    const int oct = octantOf(x - centerX(), y - centerY());
    if (oct >= 0) target_ = oct;
}

// A tap (not a drag) fires in whatever direction the barrel is pointing now.
void HuntScreen::onTap(int16_t x, int16_t y) {
    if (over_) {
        const int carried = meat_ > game::CarryCapLb ? game::CarryCapLb : meat_;
        game::g.vehicle.food += carried;
        game::g.vehicle.bullets -= shots_;
        if (game::g.vehicle.bullets < 0) game::g.vehicle.bullets = 0;
        game::sim.rest(1);   // the day was spent hunting
        app::screens.pop();
        return;
    }
    if (done_.contains(x, y)) { over_ = true; return; }
    fire();
}

void HuntScreen::render(LGFX_Sprite& g) {
    g.fillScreen(theme::BG);

    // status strip
    g.fillRect(0, 0, 320, FIELD_TOP, theme::PANEL);
    g.setFont(&fonts::Font2);
    g.setTextDatum(textdatum_t::middle_left);
    g.setTextColor(theme::INK);
    char l[24];
    snprintf(l, sizeof(l), "Ammo %d", game::g.vehicle.bullets - shots_);
    g.drawString(l, 6, FIELD_TOP / 2);
    g.setTextDatum(textdatum_t::middle_center);
    char m[20];
    snprintf(m, sizeof(m), "%d lb", meat_);
    g.drawString(m, 160, FIELD_TOP / 2);
    // time bar
    const int bw = (int)(80.0f * timeLeftMs_ / 22000.0f);
    g.drawRect(232, 5, 82, FIELD_TOP - 10, theme::FRAME);
    g.fillRect(233, 6, bw, FIELD_TOP - 12, theme::ACCENT);

    // grassy field with a distant tree line
    g.fillRect(0, FIELD_TOP, 320, FIELD_BOT - FIELD_TOP, art::col::ground);
    {
        int off = animMs_ / 200 % 320;
        art::drawScenery(g, 0, -off, FIELD_TOP - 2);
        art::drawScenery(g, 0, 320 - off, FIELD_TOP - 2);
    }

    // animal walk-cycle base frame (0-based art indices)
    auto animalFrame = [&](uint8_t kind, bool dead) -> int {
        const int cyc = (int)(animMs_ / 140) % 6;
        if (kind == 0) return dead ? 32 : 33 + cyc;   // rabbit
        if (kind == 1) return dead ? 16 : 17 + cyc;   // deer
        return dead ? 0 : 1 + cyc;                     // bison
    };

    // carcasses
    for (int i = 0; i < carc_; ++i)
        art::drawAnimal(g, animalFrame(carcKind_[i], true), carcX_[i] - 13,
                        carcY_[i] - 8);

    // live animals
    for (auto& a : ani_) {
        if (!a.alive) continue;
        art::drawAnimal(g, animalFrame(a.kind, false), (int)a.x - 13, (int)a.y - 9);
    }

    // bullets
    for (auto& b : bul_)
        if (b.live) g.fillCircle((int)b.x, (int)b.y, 2, theme::WARN);

    const int hx = centerX(), hy = centerY();

    // aim ring: a filled dot where the barrel points now, a hollow marker where
    // your finger has asked it to swing.
    constexpr int RING = 46;
    g.drawCircle(hx, hy, RING, theme::FRAME);
    g.drawCircle(hx, hy, RING - 1, theme::FRAME);
    if (target_ >= 0 && target_ != aim_) {
        const int gx = hx + (int)(AIMX[target_] * RING);
        const int gy = hy + (int)(AIMY[target_] * RING);
        g.drawCircle(gx, gy, 4, theme::INK_DIM);
    }
    const int ax = hx + (int)(AIMX[aim_] * RING);
    const int ay = hy + (int)(AIMY[aim_] * RING);
    g.fillCircle(ax, ay, 4, theme::ACCENT);
    g.drawLine(hx + (int)(AIMX[aim_] * 12), hy + (int)(AIMY[aim_] * 12),
               ax, ay, theme::ACCENT);

    // hunter — pose by aim octant
    static const int POSE[8] = {0, 4, 10, 14, 20, 14, 10, 4};
    art::drawHunter(g, POSE[aim_], hx - 15, hy - 28, 1.4f);

    // muzzle flash on a fresh shot
    if (bul_[0].live && animMs_ % 100 < 50 &&
        fabsf(bul_[0].x - hx) < 16 && fabsf(bul_[0].y - hy) < 16)
        g.fillCircle((int)bul_[0].x, (int)bul_[0].y, 3, 0xFE60);

    if (over_) {
        g.fillRect(30, 92, 260, 60, theme::PANEL);
        g.drawRect(30, 92, 260, 60, theme::FRAME);
        const int carried = meat_ > game::CarryCapLb ? game::CarryCapLb : meat_;
        char s[48];
        snprintf(s, sizeof(s), "Carried back %d lb of meat", carried);
        ui::drawCentered(g, "HUNT OVER", 160, 100, theme::ACCENT, 2);
        ui::drawCentered(g, s, 160, 122, theme::INK, 2);
        ui::drawCentered(g, "tap to return", 160, 138, theme::INK_DIM, 2);
        return;
    }

    done_ = {250, FIELD_BOT - 24, 66, 20};
    ui::drawButton(g, done_, "stop", true);
}
