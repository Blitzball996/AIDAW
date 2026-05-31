// LicenseManager implementation — see LicenseManager.hpp.
#include "LicenseManager.hpp"

namespace magda {

namespace {
// Friendly Chinese text for a server/spec error code.
juce::String errText(const juce::String& code) {
    if (code == "NOT_FOUND")         return juce::String::fromUTF8("查无此序列号");
    if (code == "ALREADY_ACTIVATED") return juce::String::fromUTF8("该序列号已在另一台设备激活（一码一机）");
    if (code == "REVOKED")           return juce::String::fromUTF8("该序列号已被吊销/退款");
    if (code == "WRONG_PRODUCT")     return juce::String::fromUTF8("序列号与本软件不符（MAGDA 只接受 BD 开头）");
    if (code == "BAD_CHECKSUM")      return juce::String::fromUTF8("序列号校验失败（请检查输入）");
    if (code == "BAD_FORMAT")        return juce::String::fromUTF8("序列号格式错误");
    if (code == "RATE_LIMITED")      return juce::String::fromUTF8("激活过于频繁，请稍后再试");
    if (code == "NETWORK")           return juce::String::fromUTF8("网络错误，无法连接激活服务器（如使用代理请检查 clash 端口 7897）");
    if (code == "BAD_TOKEN")         return juce::String::fromUTF8("激活令牌签名校验失败");
    return juce::String::fromUTF8("激活失败（") + code + juce::String::fromUTF8("）");
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
    out.message = juce::String::fromUTF8("激活成功！版本：") + out.edition +
                  juce::String::fromUTF8("（") + juce::String(ti.key) + juce::String::fromUTF8("）");
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
        juce::String::fromUTF8("激活 MAGDA"),
        juce::String::fromUTF8("请输入序列号（格式 BDST/BDPR-XXXX-XXXX-XXXX-XXXX）：\n设备指纹: ") +
            juce::String(lic::deviceId()),
        juce::AlertWindow::QuestionIcon);
    aw->addTextEditor("key", "", juce::String::fromUTF8("序列号"));
    aw->addButton(juce::String::fromUTF8("激活"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    aw->addButton(lockedMode ? juce::String::fromUTF8("退出") : juce::String::fromUTF8("取消"), 0,
                  juce::KeyPress(juce::KeyPress::escapeKey));
    aw->setVisible(true);

    aw->enterModalState(
        true,
        juce::ModalCallbackFunction::create([this, aw, lockedMode, onDone](int r) {
            const juce::String key = aw->getTextEditorContents("key").trim();
            if (r != 1) {  // 取消 / 退出
                if (lockedMode) quitApp();
                if (onDone) onDone(false);
                return;
            }
            auto res = activate(key);
            if (res.ok) {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                       juce::String::fromUTF8("激活成功"), res.message);
                if (onDone) onDone(true);
            } else {
                // Show the error, then re-open the prompt so the user can retry.
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon, juce::String::fromUTF8("激活失败"),
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
            juce::AlertWindow::WarningIcon, juce::String::fromUTF8("试用已结束"),
            juce::String::fromUTF8("MAGDA 试用已用尽（30 分钟 + 10 分钟）。请输入序列号激活以继续使用。"),
            {}, nullptr,
            juce::ModalCallbackFunction::create([this](int) {
                promptActivation(nullptr, /*lockedMode*/ true);
            }));
        return;
    }

    // Trial: announce, start countdown.
    trialRun_ = s.trialRun;
    trialAllowanceSec_ = s.allowanceSeconds;
    trialStartMs_ = juce::Time::currentTimeMillis();
    committed_ = false;

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon, juce::String::fromUTF8("试用模式"),
        juce::String::fromUTF8("MAGDA 试用（第 ") + juce::String(trialRun_) +
            juce::String::fromUTF8(" 次）：本次可用 ") + juce::String(trialAllowanceSec_ / 60) +
            juce::String::fromUTF8(" 分钟。\n激活以解除限制（菜单 帮助 → 激活）。"));

    startTimer(1000);  // check every second
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
            juce::AlertWindow::WarningIcon, juce::String::fromUTF8("试用结束"),
            juce::String::fromUTF8("本次试用时间（") + juce::String(trialAllowanceSec_ / 60) +
                juce::String::fromUTF8(" 分钟）已结束，MAGDA 将退出。请激活后继续使用。"),
            {}, nullptr,
            juce::ModalCallbackFunction::create([](int) { quitApp(); }));
    }
}

} // namespace magda
