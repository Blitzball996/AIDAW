// LicenseManager — AIDAW (MAGDA / Blitz DAW) licensing shim around LicenseCore.
//
// Mirrors CloseCrab's LicenseGate but uses JUCE for transport/UI:
//   - HTTP via juce::URL (JUCE_USE_CURL=0 -> WinINet on Windows, which honours
//     the system / Clash proxy automatically)
//   - JSON via juce::JSON
//   - activation dialog via juce::AlertWindow
//
// The security-critical logic (Ed25519 verify, serial checksum, device id,
// registry storage) is the shared LicenseCore, identical to CloseCrab.
#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include "LicenseCore.h"

namespace magda {

class LicenseManager : public juce::Timer {
public:
    static constexpr const char* kAppKey = "BlitzDAW";  // registry subkey (HKCU\Software\Blitzball)
    static constexpr const char* kPrefix2 = "BD";       // accept only BD** serials

    struct ActivateResult {
        bool        ok = false;
        juce::String errorCode;
        juce::String message;   // Chinese, user-facing
        juce::String edition;
    };

    static LicenseManager& getInstance();

    // Resolved server base URL (env MAGDA_LICENSE_URL > default).
    static juce::String baseUrl();

    lic::Status status() const { return lic::evaluate(kAppKey, kPrefix2); }

    // Online activation + local persistence. Safe to call from the message thread.
    ActivateResult activate(const juce::String& rawKey);
    void deactivate() { lic::clearActivation(kAppKey); }

    // Startup gate. Call from finishInitialisation() AFTER the main window exists.
    //   - Activated: returns immediately.
    //   - Trial: shows an info box + starts the countdown timer (quits on expiry).
    //   - Locked: shows an (async) activation dialog; if the user cancels, quits.
    void enforceAtStartup();

    // Show the activation dialog (async). onDone(true) if activated, onDone(false)
    // if the user cancelled. When lockedMode is true, cancelling quits the app.
    void promptActivation(std::function<void(bool)> onDone = {}, bool lockedMode = false);

private:
    LicenseManager() = default;
    void timerCallback() override;  // trial expiry

    int  trialRun_ = 0;
    int  trialAllowanceSec_ = 0;
    juce::int64 trialStartMs_ = 0;
    bool committed_ = false;
};

} // namespace magda
