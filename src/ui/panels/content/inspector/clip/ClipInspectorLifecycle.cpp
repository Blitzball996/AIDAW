#include "../../../../themes/DarkTheme.hpp"
#include "../ClipInspector.hpp"

namespace aidaw::daw::ui {

void ClipInspector::onActivated() {
    aidaw::ClipManager::getInstance().addListener(this);
}

void ClipInspector::onDeactivated() {
    aidaw::ClipManager::getInstance().removeListener(this);
}

void ClipInspector::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getBackgroundColour());
}

}  // namespace aidaw::daw::ui
