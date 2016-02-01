#include "Game/Systems/PlayerHUD.h"

PlayerHUD::PlayerHUD(World* world, EventBroker* eventBrokerer)
    :System(world, eventBrokerer)
    , m_World(world)
    , m_EventBroker(eventBrokerer)
{
    
    
}

PlayerHUD::~PlayerHUD()
{
    

}

void PlayerHUD::Update(double dt)
{
    auto healthHUDs = m_World->GetComponents("HealthHUD");
    if (healthHUDs == nullptr) {
        return;
    }

    for (auto& healthHUDComponent : *healthHUDs) {
        EntityWrapper entity = EntityWrapper(m_World, healthHUDComponent.EntityID);

        EntityWrapper entityIDParent = entity;
        while (entityIDParent.Parent().Valid()) {
            entityIDParent = entityIDParent.Parent();
                
            if (entityIDParent.HasComponent("Health")) {
                break;
            }
        }

        if (entityIDParent.HasComponent("Health")) {

            if (entity.HasComponent("Text")) {
                std::string s = "";
                s = s + std::to_string((int)(double)entityIDParent["Health"]["Health"]);
                s = s + "/";
                s = s + std::to_string((int)(double)entityIDParent["Health"]["MaxHealth"]);
                float healthPercentage = (double)entityIDParent["Health"]["Health"]/(double)entityIDParent["Health"]["MaxHealth"];
                (glm::vec4&)entity["Text"]["Color"] = glm::vec4(1.0 - healthPercentage, 0.f, healthPercentage, 1.f);
                entity["Text"]["Content"] = s;
            }

            if(entity.HasComponent("Fill")) {
                float healthPercentage = (double)entityIDParent["Health"]["Health"]/(double)entityIDParent["Health"]["MaxHealth"];
                (glm::vec4&)entity["Fill"]["Color"] = glm::vec4(1.0 - healthPercentage, 0.f, healthPercentage, 0.f);
                (double&)entity["Fill"]["Percentage"] = healthPercentage;

            }
        }
    }
}
