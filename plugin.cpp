#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
using namespace RE;

namespace StuckUnderwater {
    struct detail {
        static void UpdateUnderWaterVariables(TESWaterSystem* a_manager, bool a_underWater, float a_waterHeight) {
            using func_t = decltype(&UpdateUnderWaterVariables);
            static REL::Relocation<func_t> func{ RELOCATION_ID(31409, 32216) };
            func(a_manager, a_underWater, a_waterHeight);
        }
    };

    struct ProcessInWater {
        static bool thunk(PlayerCharacter* a_actor, hkpCollidable* a_collidable, float a_waterHeight, float a_deltaTime) {
            bool original_result = func(a_actor, a_collidable, a_waterHeight, a_deltaTime);
            if (!original_result) {
                return false;
            }
            const auto camera = PlayerCamera::GetSingleton();
            if (!camera) {
                return original_result;
            }
            const auto cameraBody = camera->GetRuntimeData().rigidBody;
            if (!cameraBody) {
                return original_result;
            }
            float worldWaterHeight{ NI_INFINITY };
            hkVector4 position;
            cameraBody->GetPosition(position);
            const float cameraHeight = position.quad.m128_f32[2];
            const bool underWater = a_waterHeight > cameraHeight;
            if (underWater) {
                worldWaterHeight = a_waterHeight * bhkWorld::GetWorldScaleInverse();
            }
            detail::UpdateUnderWaterVariables(TESWaterSystem::GetSingleton(), underWater, worldWaterHeight);
            return underWater;
        }
        static inline REL::Relocation<decltype(thunk)> func;
        static constexpr std::size_t idx{ 0x9C };
    };

    void InstallUnderwaterFix() {
        REL::Relocation<std::uintptr_t> vtbl{PlayerCharacter::VTABLE[0]};
        ProcessInWater::func = vtbl.write_vfunc(ProcessInWater::idx, ProcessInWater::thunk);
    }

    SKSEPluginLoad(const SKSE::LoadInterface* skse) {
        SKSE::Init(skse);
        SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
            if (message->type == SKSE::MessagingInterface::kDataLoaded) {
                InstallUnderwaterFix();
            }
        });
        return true;
    }
}
