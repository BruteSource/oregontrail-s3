// Oregon Trail — Hosyond ESP32-S3.  Phase 1: hardware shell + screen stack.
//
// loop() runs a fixed ~33 ms frame: poll touch, feed the top screen, render the
// whole frame into one PSRAM sprite, push it once (flicker-free per target
// ref §5a).  Touch + calibration are the proven implementation from
// github.com/BruteSource/gridiron-esp32s3, adapted to landscape.
#include <Arduino.h>
#include <esp_random.h>

#include "Settings.h"
#include "game/Session.h"
#include "game/Sim.h"
#include "game/Store.h"
#include "art/gen/music.h"
#include "hw/Audio.h"
#include "hw/Battery.h"
#include "hw/Display.h"
#include "hw/Screenshot.h"
#include "hw/Storage.h"
#include "hw/Touch.h"
#include "ui/App.h"
#include "ui/ScreenStack.h"
#include "ui/Theme.h"

#include "screens/MainMenuScreen.h"
#include "screens/MonthScreen.h"
#include "screens/NameEntryScreen.h"
#include "screens/ProfessionScreen.h"
#include "game/Events.h"
#include "screens/EventScreen.h"
#include "screens/GameOverScreen.h"
#include "screens/StoreScreen.h"
#include "screens/TitleScreen.h"
#include "screens/TrailMenuScreen.h"

static LGFX        lcd;
static LGFX_Sprite frame(&lcd);

namespace app {
ScreenStack screens;
bool wantRecal = false;
bool wantSleep = false;
bool wantRestart = false;
void setBrightness(int level) { lcd.setBrightness(level); }
}

static uint32_t s_lastFrame = 0;
static uint32_t s_lastTouch = 0;
static bool     s_haveSprite = false;

// Idle time before the screen sleeps on its own. Deep sleep / light sleep both
// hang on this S3 + octal-PSRAM board (target ref §13), so "sleep" is just:
// backlight off, poll the touch chip until a tap.
constexpr uint32_t kAutoSleepMs = 4 * 60 * 1000;
constexpr int      kBootBtn = 0;   // BOOT button, active low

static void enterStandby() {
    frame.fillScreen(theme::BG);
    frame.setTextDatum(textdatum_t::middle_center);
    frame.setFont(&fonts::Font4);
    frame.setTextColor(theme::ACCENT);
    frame.drawString("Sleeping", OT_W / 2, OT_H / 2 - 8);
    frame.setFont(&fonts::Font2);
    frame.setTextColor(theme::INK_DIM);
    frame.drawString("tap the screen to wake", OT_W / 2, OT_H / 2 + 16);
    frame.pushSprite(0, 0);

    for (uint32_t t = millis(); touch::isTouched() && millis() - t < 3000;)
        delay(20);                 // wait for the finger to lift
    delay(60);
    lcd.setBrightness(0);
    Serial.println("[power] standby");

    for (;;) {
        if (touch::isTouched() || digitalRead(kBootBtn) == LOW) break;
        delay(110);
    }

    Serial.println("[power] wake");
    lcd.setBrightness(prefs::brightness);
    touch::busResume();
    for (uint32_t t = millis(); touch::isTouched() && millis() - t < 2000;)
        delay(20);                 // swallow the wake tap
    touch::poll();
    s_lastTouch = millis();
    s_lastFrame = 0;
}

static void bootCard(const char* msg) {
    frame.fillScreen(theme::BG);
    frame.setTextDatum(textdatum_t::middle_center);
    frame.setFont(&fonts::Font4);
    frame.setTextColor(theme::ACCENT);
    frame.drawString("THE OREGON TRAIL", OT_W / 2, OT_H / 2 - 16);
    frame.setFont(&fonts::Font2);
    frame.setTextColor(theme::INK_DIM);
    frame.drawString(msg, OT_W / 2, OT_H / 2 + 12);
    frame.pushSprite(0, 0);
}

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(30);   // keep dev-command parseInt() from stalling a frame
    for (uint32_t t = millis(); !Serial && millis() - t < 1200;) delay(10);
    Serial.println("\n[OregonTrail] boot");

    lcd.init();
    lcd.setRotation(OT_ROTATION);
    prefs::load();
    lcd.setBrightness(prefs::brightness);
    lcd.fillScreen(TFT_BLACK);

    storage::begin();

    frame.setColorDepth(16);
    frame.setPsram(true);
    s_haveSprite = frame.createSprite(OT_W, OT_H);
    if (!s_haveSprite) {
        Serial.println("[OregonTrail] FATAL: could not allocate 320x240 sprite");
        lcd.setTextColor(TFT_RED);
        lcd.drawString("sprite alloc failed", 10, 10);
    }

    touch::begin();
    touch::diag();
    bootCard("loading...");

    // Calibrate if never done, or if a finger is being held on the screen now.
    {
        bool held = false;
        for (uint32_t t0 = millis(); millis() - t0 < 600;) {
            if (touch::isTouched()) { held = true; break; }
            delay(20);
        }
        if (!touch::isCalibrated() || held) {
            Serial.println("[OregonTrail] running touch calibration");
            touch::runCalibration(frame);
        }
    }

    battery::begin();
    audio::begin();
    audio::setLevel(prefs::volume);
    game::seedRng(esp_random());

    app::screens.reset(new TitleScreen());
    s_lastFrame = millis();
    s_lastTouch = millis();
}

void loop() {
    const uint32_t now = millis();
    const uint32_t dt = now - s_lastFrame;
    if (dt < 33) {
        delay(1);
        return;
    }
    s_lastFrame = now;

    battery::update();

    if (app::wantRestart) {
        delay(50);
        ESP.restart();
    }
    if (app::wantRecal) {
        app::wantRecal = false;
        touch::runCalibration(frame);
        s_lastFrame = millis();
        s_lastTouch = millis();
        return;
    }
    if (app::wantSleep ||
        (kAutoSleepMs && now - s_lastTouch > kAutoSleepMs)) {
        app::wantSleep = false;
        enterStandby();
        return;
    }

    // Dev serial commands.
    bool wantShot = false;
    while (Serial.available()) {
        const int c = Serial.read();
        switch (c) {
            case 'S': case 's': wantShot = true; break;
            case 'T': {   // "T<x>,<y>" — inject a synthetic tap (dev/testing)
                const long tx = Serial.parseInt();
                const long ty = Serial.parseInt();
                audio::click();
                app::screens.dispatchTap((int16_t)tx, (int16_t)ty);
                break;
            }
            case 't': app::screens.reset(new TitleScreen()); break;
            case 'm': app::screens.reset(new MainMenuScreen()); break;
            case 'p':   // jump into the new-game flow at profession select
                game::g.beginNewGame(game::Profession::Banker);
                app::screens.reset(new ProfessionScreen());
                break;
            case 'n':   // jump to party-name entry
                game::g.beginNewGame(game::Profession::Banker);
                app::screens.reset(new NameEntryScreen());
                break;
            case 'k':   // jump straight into the on-screen keyboard
                game::g.beginNewGame(game::Profession::Banker);
                NameEntryScreen::devStartInKeyboard = true;
                app::screens.reset(new NameEntryScreen());
                break;
            case 'o':   // jump to start-month select
                game::g.beginNewGame(game::Profession::Banker);
                app::screens.reset(new MonthScreen());
                break;
            case 'g': {  // jump straight to the store with a test party
                game::g.beginNewGame(game::Profession::Banker);
                game::g.party.count = 5;
                const char* nm[5] = {"Sean", "Amos", "Clara", "Hank", "Nettie"};
                for (int i = 0; i < 5; ++i) game::g.party.member[i].setName(nm[i]);
                app::screens.reset(new StoreScreen());
                break;
            }
            case 'r': {  // jump onto the trail with a stocked wagon
                game::g.beginNewGame(game::Profession::Banker);
                game::g.party.count = 5;
                const char* nm[5] = {"Sean", "Amos", "Clara", "Hank", "Nettie"};
                for (int i = 0; i < 5; ++i) game::g.party.member[i].setName(nm[i]);
                game::applyPurchase(game::g.vehicle, game::ItemId::Oxen, 6, 0);
                game::applyPurchase(game::g.vehicle, game::ItemId::Food, 1500, 0);
                game::applyPurchase(game::g.vehicle, game::ItemId::Clothes, 5, 0);
                game::applyPurchase(game::g.vehicle, game::ItemId::Bullets, 100, 0);
                game::sim.begin();
                app::screens.reset(new TrailMenuScreen());
                break;
            }
            case 'j': {   // "j<n>" — teleport the wagon to trail node n (dev)
                const long node = Serial.parseInt();
                game::sim.locIndex = (int)node;
                game::sim.milesIntoLeg = 0;
                game::sim.departed = false;
                game::sim.needBranch = false;
                game::sim.mustCross = game::sim.here().kind == game::Stop::River;
                app::screens.reset(new TrailMenuScreen());
                break;
            }
            case 'e': {   // "e<n>" force a random event (n = EventKind, 0 = any)
                const long want = Serial.parseInt();
                for (int tries = 0; tries < 400; ++tries) {
                    game::GameEvent ev;
                    if (game::rollDailyEvent(ev) &&
                        (want == 0 || (long)ev.kind == want)) {
                        game::sim.pendingEvent = ev;
                        app::screens.push(new EventScreen());
                        break;
                    }
                }
                break;
            }
            case 'V': {   // "V" prints the battery reading; "V<volts>" calibrates
                const float trueV = Serial.parseFloat();
                if (trueV > 2.5f) {
                    const float d = battery::calibrate(trueV);
                    Serial.printf("[bat] calibrated to %.2fV -> divider %.4f\n",
                                  trueV, d);
                } else {
                    Serial.printf("[bat] raw %.3fV  smoothed %.3fV  %d%%  "
                                  "divider %.4f\n",
                                  battery::rawVolts(), battery::volts(),
                                  battery::percent(), battery::divider());
                }
                break;
            }
            case 'B':   // dev: play the audio test chime
                Serial.printf("[audio] test chime (ready=%d)\n", audio::ready());
                audio::testChime();
                break;
            case 'M': {   // "M<n>" — dev: play landmark song n (n<0 = tombstone)
                const long n = Serial.parseInt();
                if (n < 0 || n >= 18)
                    audio::playSong(music::tombstone.notes, music::tombstone.len);
                else
                    audio::playSong(music::landmark[n].notes, music::landmark[n].len);
                Serial.printf("[audio] play song %ld\n", n);
                break;
            }
            case 'x': game::eventsEnabled = false; Serial.println("[dev] events off"); break;
            case 'X': game::eventsEnabled = true;  Serial.println("[dev] events on");  break;
            case 'A':   // dev: fast-forward to the next arrival / fork / river
                game::sim.departed = true;
                game::sim.milesIntoLeg = game::sim.legLength() > 3
                                            ? game::sim.legLength() - 3 : 0;
                app::screens.reset(new TrailMenuScreen());
                break;
            case 'W': app::screens.reset(new GameOverScreen(true)); break;
            case 'L':
                for (int i = 0; i < game::g.party.count; ++i) {
                    game::g.party.member[i].alive = false;
                    strncpy(game::g.party.member[i].causeOfDeath, "the trail", 23);
                }
                app::screens.reset(new GameOverScreen(false));
                break;
            default: break;
        }
    }

    touch::Point p = touch::poll();
    static bool s_wasDown = false;
    if (p.down) { app::screens.dispatchTouchMove(p.x, p.y); s_lastTouch = now; }
    else if (s_wasDown) app::screens.dispatchTouchEnd();
    s_wasDown = p.down;
    // Debounce: the FT6336 sometimes reports a momentary release + re-touch,
    // which lands as two taps (double-press, double click sound). Ignore a new
    // tap that arrives right on the heels of the last one.
    static uint32_t s_lastTap = 0;
    constexpr uint32_t kTapDebounceMs = 220;
    if (p.pressed && now - s_lastTap >= kTapDebounceMs) {
        s_lastTap = now;
        audio::click();
        app::screens.dispatchTap(p.x, p.y);
        s_lastTouch = now;
    }

    app::screens.update(dt, frame);
    if (s_haveSprite) {
        frame.pushSprite(0, 0);
        if (wantShot) screenshot::dump(frame);   // 'S' on serial grabs this frame
    }
}
