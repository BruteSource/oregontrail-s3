// Process-wide singletons for the UI layer. An embedded app has exactly one of
// each; passing them through every screen constructor buys nothing.
#pragma once

class ScreenStack;

namespace app {
extern ScreenStack screens;

// Set by a screen to request an action serviced by the main loop (which owns the
// frame sprite / power state).
extern bool wantRecal;    // blocking touch recalibration
extern bool wantSleep;    // backlight-off standby until a tap
extern bool wantRestart;  // ESP.restart()
}
