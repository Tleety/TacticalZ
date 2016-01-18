#include "Systems/PlayerSpawnSystem.h"

PlayerSpawnSystem::PlayerSpawnSystem(EventBroker* eventBroker) 
    : System(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_OnInputCommand, &PlayerSpawnSystem::OnInputCommand);
}

void PlayerSpawnSystem::Update(World* world, double dt)
{
    auto playerSpawns = world->GetComponents("PlayerSpawn");
    if (playerSpawns == nullptr) {
        return;
    }

    for (auto& team : m_SpawnRequests) {
        for (auto& cPlayerSpawn : *playerSpawns) {
            EntityWrapper spawner(world, cPlayerSpawn.EntityID);
            if (!spawner.HasComponent("Spawner")) {
                continue;
            }

            // If the spawner has a team affiliation, check it
            if (spawner.HasComponent("Team")) {
                if ((int)spawner["Team"]["Team"] != team) {
                    continue;
                }
            }

            // Spawn the player!
            EntityWrapper player = SpawnerSystem::Spawn(spawner);
            // Set the player team affiliation
            player["Team"]["Team"] = team;
        }
    }
    m_SpawnRequests.clear();
}

bool PlayerSpawnSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.Command != "PickTeam") {
        return false;
    }

    if (e.Value != 0) {
        m_SpawnRequests.push_back((int)e.Value);
    }

    return true;
}

