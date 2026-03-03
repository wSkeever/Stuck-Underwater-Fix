#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
using namespace RE;
using namespace SKSE;
using namespace REL;

namespace StuckUnderwater {
    bool g_wasUnderWater = false;
    float g_lastProcessTime = FLT_MIN;

    struct detail {
        static void UpdateUnderwaterVariables(TESWaterSystem* a_manager, bool a_underWater, float a_waterHeight) {
            using func_t = decltype(&UpdateUnderwaterVariables);
            static Relocation<func_t> func{ RELOCATION_ID(31409, 32216) };
            func(a_manager, a_underWater, a_waterHeight);
        }
    };

    static bool IsCameraUnderwater(PlayerCharacter* a_actor, float a_waterHeight, bool a_originalResult) {
        if (!a_originalResult) {
            return a_originalResult;
        }
        const auto camera = PlayerCamera::GetSingleton();
        if (!camera) {
            return a_originalResult;
        }
        const auto cameraBody = camera->GetRuntimeData().rigidBody;
        if (!cameraBody) {
            return a_originalResult;
        }
        hkVector4 position;
        cameraBody->GetPosition(position);
        const float cameraHeight = position.quad.m128_f32[2];
        return a_waterHeight > cameraHeight;
    }

    struct ProcessInWater {
        static bool thunk(PlayerCharacter* a_actor, hkpCollidable* a_collidable, float a_waterHeight, float a_deltaTime) {
            auto calendar = Calendar::GetSingleton();
            if (calendar) {
                g_lastProcessTime = calendar->GetCurrentGameTime();
            }
            const bool originalResult = func(a_actor, a_collidable, a_waterHeight, a_deltaTime);
            if (!originalResult) {
                g_wasUnderWater = false;
                return false;
            }
            const bool underWater = IsCameraUnderwater(a_actor, a_waterHeight, originalResult);
            if (!underWater && g_wasUnderWater) {
                detail::UpdateUnderwaterVariables(TESWaterSystem::GetSingleton(), underWater, a_waterHeight);
            }
            g_wasUnderWater = underWater;
            return underWater;
        }
        static inline Relocation<decltype(thunk)> func;
        static constexpr size_t idx{0x9C};
    };

    static void PostWaterFix() {
        if (!g_wasUnderWater) {
            return;
        }
        if (g_lastProcessTime == FLT_MIN) {
            return;
        }
        auto calendar = Calendar::GetSingleton();
        if (!calendar) {
            return;
        }
        auto processTime = calendar->GetCurrentGameTime();
        auto timescale = calendar->GetTimescale();
        if ((processTime - g_lastProcessTime) * 86400.0f / timescale < 1.0f) {
            return;
        }
        detail::UpdateUnderwaterVariables(TESWaterSystem::GetSingleton(), false, FLT_MIN);
        g_lastProcessTime = processTime;
        g_wasUnderWater = false;
        SKSE::log::info("Clearing uncleared underwater effects after exiting water.");
    }

    struct PlayerUpdate {
        static void thunk(RE::PlayerCharacter* a_actor, float a_deltaTime) {
            PostWaterFix();
            func(a_actor, a_deltaTime);
        }
        static inline Relocation<decltype(thunk)> func;
        static constexpr size_t idx{0xAD};
    };

    void InstallUnderwaterFix() {
        Relocation<uintptr_t> vtbl{PlayerCharacter::VTABLE[0]};
        ProcessInWater::func = vtbl.write_vfunc(ProcessInWater::idx, ProcessInWater::thunk);
        PlayerUpdate::func = vtbl.write_vfunc(PlayerUpdate::idx, PlayerUpdate::thunk);
        SKSE::log::info("Data loaded, hooks installed.");
    }

    SKSEPluginLoad(const LoadInterface* skse) {
        Init(skse);
        GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) {
            if (message->type == MessagingInterface::kDataLoaded) {
                InstallUnderwaterFix();
            }
        });
        return true;
    }
}
