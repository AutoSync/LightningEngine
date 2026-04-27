// HelloPlugin.cpp — minimal Lightning Engine plugin example.
//
// Demonstrates the C ABI handshake (LightningPluginApiVersion) and the
// IEnginePlugin lifecycle hooks. See docs/enduser/03-c-api-plugin.md for
// the walkthrough.
#include "PluginContracts.h"
#include "EngineApiVersion.h"

#include <cstdio>

using namespace LightningEngine;

namespace {

PluginManifest gManifest{
    /* id              */ "com.lightningengine.examples.hello",
    /* name            */ "Hello Plugin",
    /* version         */ "0.1.0",
    /* engineVersionMin*/ "0.1.0",
    /* engineVersionMax*/ "",
    /* scope           */ PluginScope::Project,
    /* category        */ "Examples",
    /* subcategory     */ "",
    /* dependencies    */ {},
    /* permissions     */ PluginPermission::None,
    /* enabledByDefault*/ true,
    /* entryNative     */ "HelloPlugin.dll",
    /* entryCSharp     */ "",
    /* entryIgnite     */ "",
};

class HelloPlugin final : public IEnginePlugin {
public:
    const PluginManifest& Manifest() const override { return gManifest; }

    bool OnLoad() override
    {
        std::printf("[HelloPlugin] OnLoad (api=%u)\n",
                    LIGHTNING_ENGINE_API_VERSION);
        return true;
    }

    void OnRegister() override
    {
        std::printf("[HelloPlugin] OnRegister\n");
        // Register types/components/systems here.
        // Example:
        //   LE_REGISTER_TYPE(Health)
        //       .Field("current", &Health::current)
        //       .Field("max",     &Health::max);
    }

    void OnActivate() override
    {
        std::printf("[HelloPlugin] OnActivate\n");
        // Subscribe to event buses here. Keep tokens for OnDeactivate.
    }

    void OnDeactivate() override
    {
        std::printf("[HelloPlugin] OnDeactivate\n");
        // Unsubscribe everything you subscribed in OnActivate.
    }

    void OnUnload() override
    {
        std::printf("[HelloPlugin] OnUnload\n");
    }
};

} // namespace

extern "C" {

LIGHTNING_PLUGIN_EXPORT std::uint32_t LightningPluginApiVersion()
{
    return LIGHTNING_ENGINE_API_VERSION;
}

LIGHTNING_PLUGIN_EXPORT IEnginePlugin* LightningPluginCreate()
{
    return new HelloPlugin();
}

LIGHTNING_PLUGIN_EXPORT void LightningPluginDestroy(IEnginePlugin* plugin)
{
    delete plugin;
}

} // extern "C"
