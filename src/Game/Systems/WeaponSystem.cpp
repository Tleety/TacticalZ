#include "Systems/WeaponSystem.h"

WeaponSystem::WeaponSystem(World* world, EventBroker* eventBroker, IRenderer* renderer)
    : System(world, eventBroker)
    , ImpureSystem()
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &WeaponSystem::OnPlayerSpawned);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &WeaponSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EShoot, &WeaponSystem::OnShoot);
}

void WeaponSystem::Update(double dt)
{

}

bool WeaponSystem::OnPlayerSpawned(const Events::PlayerSpawned& e)
{
    if (e.PlayerID == -1) {
        m_LocalPlayer = e.Player;
    }
    return true;
}

bool WeaponSystem::OnInputCommand(const Events::InputCommand& e)
{
    // Only shoot client-side!
    if (e.PlayerID != -1) {
        return false;
    }

    // Only shoot if the player is alive
    if (!m_LocalPlayer.Valid()) {
        return false;
    }

    if (e.Command == "PrimaryFire" && e.Value > 0) {
        Events::Shoot eShoot;
        if (e.PlayerID == -1) {
            eShoot.Player = m_LocalPlayer;
        } else {
            eShoot.Player = e.Player;
        }
        m_EventBroker->Publish(eShoot);
    }

    return true;
}

bool WeaponSystem::OnShoot(Events::Shoot& eShoot) 
{
    if (!eShoot.Player.Valid()) {
        return false;
    }

    // TODO: Weapon firing effects here

    auto rayRed = ResourceManager::Load<EntityFile>("Schema/Entities/RayRed.xml");
    auto rayBlue = ResourceManager::Load<EntityFile>("Schema/Entities/RayBlue.xml");

    EntityWrapper weapon = eShoot.Player.FirstChildByName("WeaponMuzzle");
    if (weapon.Valid()) {
        EntityWrapper ray;
        if ((ComponentInfo::EnumType)eShoot.Player["Team"]["Team"] == eShoot.Player["Team"]["Team"].Enum("Red")) {
            EntityFileParser parser(rayRed);
            EntityID rayID = parser.MergeEntities(m_World);
            ray = EntityWrapper(m_World, rayID);
        } else {
            EntityFileParser parser(rayBlue);
            EntityID rayID = parser.MergeEntities(m_World);
            ray = EntityWrapper(m_World, rayID);
        }

        glm::mat4 transformation = Transform::AbsoluteTransformation(weapon);
        glm::vec3 scale;
        glm::vec3 translation;
        glm::quat orientation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transformation, scale, orientation, translation, skew, perspective);
        
        // Matrix to euler angles
        glm::vec3 euler;
        euler.y = glm::asin(-transformation[0][2]);
        if (cos(euler.y) != 0) {
            euler.x = atan2(transformation[1][2], transformation[2][2]);
            euler.z = atan2(transformation[0][1], transformation[0][0]);
        } else {
            euler.x = atan2(-transformation[2][0], transformation[1][1]);
            euler.z = 0;
        }

        //LOG_DEBUG("rotation: %f %f %f", euler.x, euler.y, euler.z);
        (glm::vec3&)ray["Transform"]["Position"] = translation;
        (glm::vec3&)ray["Transform"]["Orientation"] = euler;
        //(glm::vec3&)ray["Transform"]["Orientation"] = Transform::AbsoluteOrientationEuler(weapon);
    }

    // Only run further picking code client-side!
    if (eShoot.Player != m_LocalPlayer) {
        return false;
    }

    // Screen center, based on current resolution!
    // TODO: check if player has enough ammo and if weapon has a cooldown or not
    Rectangle screenResolution = m_Renderer->GetViewPortSize();
    glm::vec2 centerScreen = glm::vec2(screenResolution.Width / 2, screenResolution.Height / 2);

    // TODO: check if player has enough ammo and if weapon has a cooldown or not

    // Pick middle of screen
    PickData pickData = m_Renderer->Pick(centerScreen);
    if (pickData.Entity == EntityID_Invalid) {
        return false;
    }

    EntityWrapper player(m_World, pickData.Entity);

    // Only care about players being hit
    if (!player.HasComponent("Player")) {
        player = player.FirstParentWithComponent("Player");
    }
    if (!player.Valid()) {
        return false;
    }

    // Check for friendly fire
    EntityWrapper shooter = eShoot.Player;
    if ((ComponentInfo::EnumType)player["Team"]["Team"] == (ComponentInfo::EnumType)shooter["Team"]["Team"]) {
        return false;
    }

    // TODO: Weapon damage calculations etc
    Events::PlayerDamage ePlayerDamage;
    ePlayerDamage.Player = player;
    ePlayerDamage.Damage = 100;
    m_EventBroker->Publish(ePlayerDamage);

    return true;
}