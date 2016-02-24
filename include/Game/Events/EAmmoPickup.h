#ifndef EAmmoPickup_h__
#define EAmmoPickup_h__

#include "Core/EventBroker.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct AmmoPickup : Event
{
    EntityWrapper Player;
    int AmmoGain;
};

}

#endif