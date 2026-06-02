// LicenseManager implementation — see LicenseManager.hpp.
#include "LicenseManager.hpp"
#include "../StringTable.hpp"

namespace magda {

namespace {
// Localized text for a server/spec error code (lang/<code>.json → license.err.*).
juce::String errText(const juce::String& code) {
    return tr("license.err." + code);
}
} // namespace

LicenseManager& LicenseManager::getInstance() {
    static LicenseManager inst;
    return inst;
}

juce::String LicenseManager::baseUrl() {
    auto env = juce::SystemStats::getEnvironmentVariable("MAGDA_LICENSE_URL", {});
    if (env.isNotEmpty()) return env;
    return juce::String(lic::kDefaultBaseUrl());
}

LicenseManager::ActivateResult LicenseManager::activate(const juce::String& rawKey) {
    ActivateResult out;

    // 1. Local validation (prefix + checksum) — no network.
    lic::ParseResult pr = lic::parseKey(rawKey.toStdString(), kPrefix2);
    if (!pr.ok) {
        out.errorCode = pr.errCode;
        out.message = errText(pr.errCode);
        return out;
    }

    // 2. Online activation via juce::URL (WinINet honours system/Clash proxy).
    std::string dev = lic::deviceId();
    auto* obj = new juce::DynamicObject();
    obj->setProperty("key", juce::String(pr.canonical));
    obj->setProperty("device_id", juce::String(dev));
    obj->setProperty("product", juce::String(pr.product));
    obj->setProperty("app_version", juce::String(juce::JUCEApplication::getInstance()
                                                     ? juce::JUCEApplication::getInstance()->getApplicationVersion()
                                                     : juce::String("0.0.0")));
    juce::String body = juce::JSON::toString(juce::var(obj), true);

    juce::URL url = juce::URL(baseUrl() + "/api/license/activate").withPOSTData(body);
    auto opts = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                    .withExtraHeaders("Content-Type: application/json\r\nAccept: application/json")
                    .withConnectionTimeoutMs(15000);

    int statusCode = 0;
    juce::StringPairArray respHeaders;
    std::unique_ptr<juce::InputStream> in(
        url.createInputStream(opts.withStatusCode(&statusCode).withResponseHeaders(&respHeaders)));
    if (in == nullptr) {
        out.errorCode = "NETWORK";
        out.message = errText("NETWORK");
        return out;
    }
    juce::String resp = in->readEntireStreamAsString();

    juce::var j = juce::JSON::parse(resp);
    if (!j.isObject()) {
        out.errorCode = "NETWORK";
        out.message = juce::String::fromUTF8("服务器返回异常（HTTP ") + juce::String(statusCode) +
                      juce::String::fromUTF8("）");
        return out;
    }

    bool okFlag = (bool)j.getProperty("ok", false);
    juce::String errCode = j.getProperty("error", okFlag ? "OK" : "UNKNOWN").toString();
    if (!okFlag) {
        out.errorCode = errCode;
        out.message = errText(errCode);
        return out;
    }

    juce::String token = j.getProperty("token", "").toString();
    juce::String sig = j.getProperty("sig", "").toString();
    juce::String edition = j.getProperty("edition", "").toString();

    // 3. Verify the signed token offline before trusting/persisting it.
    lic::TokenInfo ti = lic::verifyToken(token.toStdString(), sig.toStdString(), dev, kPrefix2);
    if (!ti.ok) {
        out.errorCode = "BAD_TOKEN";
        out.message = errText("BAD_TOKEN");
        return out;
    }

    // 4. Persist.
    std::string ed = edition.isEmpty() ? ti.edition : edition.toStdString();
    lic::saveActivation(kAppKey, ti.key, token.toStdString(), sig.toStdString(), ed);
    out.ok = true;
    out.errorCode = "OK";
    out.edition = juce::String(ed);
    out.message = tr("license.activated_msg").replace("{edition}", out.edition)
                      .replace("{key}", juce::String(ti.key));
    return out;
}

static void quitApp() {
    if (auto* app = juce::JUCEApplication::getInstance())
        app->systemRequestedQuit();
}

void LicenseManager::promptActivation(std::function<void(bool)> onDone, bool lockedMode) {
    if (status().state == lic::State::Activated) {
        if (onDone) onDone(true);
        return;
    }

    auto* aw = new juce::AlertWindow(
        tr("license.dialog_title"),
        tr("license.enter_key_prompt").replace("{device}", juce::String(lic::deviceId())),
        juce::AlertWindow::QuestionIcon);
    aw->addTextEditor("key", "", tr("license.serial"));
    aw->addButton(tr("license.activate"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton(lockedMode ? tr("license.quit") : tr("license.cancel"), 0,
                  juce::KeyPress(juce::KeyPress::escapeKey));
    aw->setVisible(true);

    aw->enterModalState(
        true,
        juce::ModalCallbackFunction::create([this, aw, lockedMode, onDone](int r) {
            const juce::String key = aw->getTextEditorContents("key").trim();
            if (r != 1) {  // cancel / quit
                if (lockedMode) quitApp();
                if (onDone) onDone(false);
                return;
            }
            auto res = activate(key);
            if (res.ok) {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                       tr("license.activate_ok_title"), res.message);
                if (onDone) onDone(true);
            } else {
                // Show the error, then re-open the prompt so the user can retry.
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon, tr("license.activate_fail_title"),
                    res.message, {}, nullptr,
                    juce::ModalCallbackFunction::create([this, lockedMode, onDone](int) {
                        promptActivation(onDone, lockedMode);
                    }));
            }
        }),
        true /* deleteWhenDismissed */);
}

void LicenseManager::enforceAtStartup() {
    lic::Status s = status();

    if (s.state == lic::State::Activated) {
        juce::Logger::writeToLog("License: activated (" + juce::String(s.edition) + " " +
                                 juce::String(s.key) + ")");
        return;
    }

    if (s.state == lic::State::Locked) {
        // 30+10 min used up: force activation or quit.
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, tr("license.trial_ended_title"),
            tr("license.locked_msg"),
            {}, nullptr,
            juce::ModalCallbackFunction::create([this](int) {
                promptActivation(nullptr, /*lockedMode*/ true);
            }));
        return;
    }

    // Trial: announce + offer to activate right now; otherwise start countdown.
    trialRun_ = s.trialRun;
    trialAllowanceSec_ = s.allowanceSeconds;

    auto startTrial = [this]() {
        trialStartMs_ = juce::Time::currentTimeMillis();
        committed_ = false;
        startTimer(1000);  // check every second
    };

    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::InfoIcon, tr("license.trial_title"),
        tr("license.trial_msg")
            .replace("{run}", juce::String(trialRun_))
            .replace("{min}", juce::String(trialAllowanceSec_ / 60)),
        tr("license.enter_key"), tr("license.start_trial"), nullptr,
        juce::ModalCallbackFunction::create([this, startTrial](int r) {
            if (r == 1) {
                // Activate now; if the user cancels or it fails, fall back to trial.
                promptActivation([startTrial](bool ok) { if (!ok) startTrial(); },
                                 /*lockedMode*/ false);
            } else {
                startTrial();
            }
        }));
}

void LicenseManager::timerCallback() {
    const juce::int64 elapsedMs = juce::Time::currentTimeMillis() - trialStartMs_;
    const int elapsedSec = (int)(elapsedMs / 1000);

    // Count this run as "used" only after 60s of real use (spec: >60s).
    if (!committed_ && elapsedSec >= 60) {
        lic::commitTrialRun(kAppKey, trialRun_);
        committed_ = true;
    }

    if (elapsedSec >= trialAllowanceSec_) {
        stopTimer();
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon, tr("license.trial_over_title"),
            tr("license.trial_over_msg").replace("{min}", juce::String(trialAllowanceSec_ / 60)),
            {}, nullptr,
            juce::ModalCallbackFunction::create([](int) { quitApp(); }));
    }
}

} // namespace magda
