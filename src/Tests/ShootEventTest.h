#ifndef ShootEventTest_h__
#define ShootEventTest_h__

#include "Core/ResourceManager.h"
#include "Core/ConfigFile.h"
#include "Core/EventBroker.h"
#include "Rendering/Renderer.h"
#include "Core/InputManager.h"
#include "GUI/Frame.h"
#include "Core/World.h"
#include "Rendering/RenderQueueFactory.h"
#include "Input/InputProxy.h"
#include "Input/KeyboardInputHandler.h"
#include "Input/MouseInputHandler.h"
#include "Core/EKeyDown.h"
#include "Core/EntityXMLFile.h"
#include "Core/SystemPipeline.h"
#include "RaptorCopterSystem.h"
#include "PlayerSystem.h"
#include "Editor/EditorSystem.h"

#include "Core\EMouseRelease.h"
#include "Core\EShoot.h"

class ShootEventTest
{
public:
    ShootEventTest();
    ~ShootEventTest();

    void Tick();
    bool TestSucceeded = false;

private:
    double m_LastTime;
    ConfigFile* m_Config = nullptr;
    EventBroker* m_EventBroker;
    World* m_World;
    SystemPipeline* m_SystemPipeline;
    int playersID;
};

#endif
