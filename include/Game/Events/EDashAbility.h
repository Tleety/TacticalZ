#ifndef Events_DashAbility_h__
#define Events_DashAbility_h__

#include "Core/Event.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct DashAbility : public Event 
{
    EntityID Player;
};

}

#endif