#include <thread>
#include <string>
#include "modloader/shared/modloader.hpp"
#include "GorillaLocomotion/Player.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils-functions.h"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"
#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "GlobalNamespace/OVRInput.hpp"
#include "GlobalNamespace/OVRInput_Button.hpp"
#include "GlobalNamespace/OVRInput_RawAxis1D.hpp.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/SphereCollider.hpp"
#include "UnityEngine/CapsuleCollider.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Physics.hpp"
#include "UnityEngine/ForceMode.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Register.hpp"
#include "gorilla-utils/shared/GorillaUtils.hpp"
#include "gorilla-utils/shared/Callbacks/MatchMakingCallbacks.hpp"
#include "gorilla-utils/shared/Utils/Player.hpp"
#include "config.hpp"
#include "main.hpp"
#include "WhereGravityGoWatchView.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

static ModInfo modInfo;

using namespace UnityEngine;
using namespace UnityEngine::XR;
using namespace GorillaLocomotion;
using namespace GlobalNamespace;

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

bool inRoom = false;
bool buttonYes = false;
float globalGravity = -9.81;
float configmult = 0.0;

#include "GlobalNamespace/GorillaTagManager.hpp"

MAKE_HOOK_MATCH(GorillaTagManager_Update, &GlobalNamespace::GorillaTagManager::Update, void, GlobalNamespace::GorillaTagManager* self) {
    if (config.enabled) {
    Player* playerInstance = Player::get_Instance();
    Rigidbody* playerPhysics = playerInstance->playerRigidBody;
    if(playerPhysics == nullptr) return;
    GameObject* playerGameObject = playerPhysics->get_gameObject();
    if(playerGameObject == nullptr) return;
    auto* player = playerGameObject->GetComponent<GorillaLocomotion::Player*>(); 
    buttonYes = OVRInput::Get(OVRInput::Button::One, OVRInput::Controller::RTouch);
    if (buttonYes == true)
    {
        configmult = config.multiplier / 5.0;
        playerPhysics->set_useGravity(false);
        UnityEngine::Vector3 gravity = globalGravity * configmult * (UnityEngine::Vector3::get_up());
        playerPhysics->AddForce(gravity, UnityEngine::ForceMode::Acceleration);
    }
    else
    {
        Player* playerInstance = Player::get_Instance();
        Rigidbody* playerPhysics = playerInstance->playerRigidBody;
        if(playerPhysics == nullptr) return;
        playerPhysics->set_useGravity(true);
    }
    GorillaTagManager_Update(self);
    }
    else
    {
        Player* playerInstance = Player::get_Instance();
        Rigidbody* playerPhysics = playerInstance->playerRigidBody;
        if(playerPhysics == nullptr) return;
        playerPhysics->set_useGravity(true);
    }
    GorillaTagManager_Update(self);
}
MAKE_HOOK_MATCH(Player_Awake, &GorillaLocomotion::Player::Awake, void, GorillaLocomotion::Player* self) {
    Player_Awake(self);
    GorillaUtils::MatchMakingCallbacks::onJoinedRoomEvent() += {[&]() {
        Il2CppObject* currentRoom = CRASH_UNLESS(il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "get_CurrentRoom"));

        if (currentRoom)
        {
            inRoom = !CRASH_UNLESS(il2cpp_utils::RunMethod<bool>(currentRoom, "get_IsVisible"));
        } else inRoom = true;
    }
    };
}

extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getLogger().info("Completed setup!");
}

extern "C" void load() {
    GorillaUI::Init();
    il2cpp_functions::Init();
    INSTALL_HOOK(getLogger(), Player_Awake);
    INSTALL_HOOK(getLogger(), GorillaTagManager_Update);
    GorillaUI::Register::RegisterWatchView<WhereGravityGo::WhereGravityGoWatchView*>("<b><i><color=#ff00ff>Where Gravity Go?</color></i></b>", VERSION);
    getLogger().info("Installing hooks...");
    LoadConfig();
    getLogger().info("Installed all hooks!");
}