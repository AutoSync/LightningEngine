// EngineApiVersion.h — ABI handshake between engine core and plugins.
//
// Plugins must export `LightningPluginApiVersion` returning this constant.
// The PluginManager rejects loading any binary whose value differs.
//
// Bump policy: see docs/developers/05-engine-api-versioning.md.
#pragma once

#include <cstdint>

#define LIGHTNING_ENGINE_API_VERSION 1u

// Cross-platform symbol export macro for plugin entry points.
//   - Define LIGHTNING_PLUGIN_BUILD when compiling a plugin shared library.
//   - Engine itself never defines it.
#if defined(_WIN32)
    #if defined(LIGHTNING_PLUGIN_BUILD)
        #define LIGHTNING_PLUGIN_EXPORT __declspec(dllexport)
    #else
        #define LIGHTNING_PLUGIN_EXPORT __declspec(dllimport)
    #endif
#else
    #define LIGHTNING_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

namespace LightningEngine {

// Helper called by PluginManager during Load to validate ABI before
// touching any C++ vtable from the plugin.
inline bool IsCompatibleApiVersion(std::uint32_t pluginVersion)
{
    return pluginVersion == LIGHTNING_ENGINE_API_VERSION;
}

} // namespace LightningEngine
