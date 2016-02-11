#include "Common.h"
#include "Core/System.h"

class RaptorCopterSystem : public PureSystem
{
public:
    RaptorCopterSystem(SystemParams params)
        : System(params)
        , PureSystem("RaptorCopter")
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override
    {
        ComponentWrapper& transform = m_World->GetComponent(component.EntityID, "Transform");
        (glm::vec3&)transform["Orientation"] += (float)(double)component["Speed"] * (float)dt * (glm::vec3)component["Axis"];
    }
};