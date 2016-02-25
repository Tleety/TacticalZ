#ifndef EntityWrapper_h__
#define EntityWrapper_h__

#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>
#include "ComponentWrapper.h"

class World;
struct EntityWrapper
{
    EntityWrapper()
        : World(nullptr)
        , ID(EntityID_Invalid)
    { }

    EntityWrapper(::World* world, EntityID id)
        : World(world)
        , ID(id)
    { }

    ::World* World;
    EntityID ID;

    static const EntityWrapper Invalid;

    const std::string Name();
    bool HasComponent(const std::string& componentType);
    void AttachComponent(const char* componentName);
    EntityWrapper Parent();
    EntityWrapper FirstChildByName(const std::string& name);
    EntityWrapper FirstParentWithComponent(const std::string& componentType);
    EntityWrapper Clone(EntityWrapper parent = EntityWrapper::Invalid);
    bool IsChildOf(EntityWrapper potentialParent);
    bool Valid() const;

    ComponentWrapper operator[](const char* componentName);
    bool operator==(const EntityWrapper& e) const;
    bool operator!=(const EntityWrapper& e) const;
    explicit operator EntityID() const;

private:
    EntityWrapper firstChildByNameRecursive(const std::string& name, EntityID parent);
    EntityWrapper cloneRecursive(EntityWrapper entity, EntityWrapper parent);
};

namespace std
{
	template<> struct hash<EntityWrapper>
	{
		std::size_t operator()(const EntityWrapper& e) const
		{
            std::size_t seed = 0;
            boost::hash_combine(seed, e.World);
            boost::hash_combine(seed, e.ID);
            return seed;
		}
	};
}

#endif
