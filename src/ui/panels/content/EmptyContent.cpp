#include "EmptyContent.hpp"

#include "../../themes/DarkTheme.hpp"

namespace aidaw::daw::ui {

EmptyContent::EmptyContent() {
    setName("Empty");
}

void EmptyContent::paint(juce::Graphics& g) {
    g.fillAll(DarkTheme::getPanelBackgroundColour());
}

void EmptyContent::resized() {
    // Nothing to layout
}

}  // namespace aidaw::daw::ui
