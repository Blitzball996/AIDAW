#pragma once

#include "../project/ProjectInfo.hpp"

namespace aidaw {

/// Abstract view onto ProjectManager — read-only project info today.
class ProjectApi {
  public:
    virtual ~ProjectApi() = default;

    virtual const ProjectInfo& getCurrentProjectInfo() const = 0;
};

}  // namespace aidaw
