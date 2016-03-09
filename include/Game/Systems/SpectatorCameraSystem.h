#ifndef SpectatorCameraSystem_h__
#define SpectatorCameraSystem_h__

#include "Core/System.h"
#include "Input/EInputCommand.h"

class SpectatorCameraSystem : public ImpureSystem
{
public:
    SpectatorCameraSystem(SystemParams params);

    virtual void Update(double dt) override;

private:
    int m_PickedTeam;
    bool m_CamSetToTeamPick;

    EventRelay<SpectatorCameraSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
};

#endif