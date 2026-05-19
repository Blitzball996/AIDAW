#include "InspectorFactory.hpp"

#include "ClipInspector.hpp"
#include "DeviceInspector.hpp"
#include "NoteInspector.hpp"
#include "TrackInspector.hpp"

namespace aidaw::daw::ui {

std::unique_ptr<BaseInspector> InspectorFactory::createInspector(aidaw::SelectionType type) {
    switch (type) {
        case aidaw::SelectionType::Track:
            return std::make_unique<TrackInspector>();

        case aidaw::SelectionType::Clip:
            return std::make_unique<ClipInspector>();

        case aidaw::SelectionType::MultiClip:
            return std::make_unique<ClipInspector>();

        case aidaw::SelectionType::Note:
            return std::make_unique<NoteInspector>();

        case aidaw::SelectionType::MultiTrack:
            return std::make_unique<TrackInspector>();

        case aidaw::SelectionType::ChainNode:
            return std::make_unique<DeviceInspector>();

        case aidaw::SelectionType::None:
        default:
            return nullptr;
    }
}

}  // namespace aidaw::daw::ui
