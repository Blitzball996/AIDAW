#include "alias_api_live.hpp"

#include "../core/aliases/AliasRegistry.hpp"
#include "../core/aliases/ResolverRegistry.hpp"

namespace aidaw {

AliasRegistry& AliasApiLive::aliasRegistry() {
    return AliasRegistry::getInstance();
}

ResolverRegistry& AliasApiLive::resolverRegistry() {
    return ResolverRegistry::getInstance();
}

}  // namespace aidaw
