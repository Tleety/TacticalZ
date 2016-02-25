#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

void CollisionSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPhysics, double dt)
{
    if (!entity.HasComponent("Collidable")) {
        return;
    }
    boost::optional<EntityAABB> boundingBox = Collision::EntityAbsoluteAABB(entity);
    if (!boundingBox) {
        return;
    }
    ComponentWrapper& cTransform = entity["Transform"];
    EntityAABB& boxA = *boundingBox;
    bool everHitTheGround = false;

    glm::vec3 size = boxA.Size();
    float diameter = std::min(size.x, size.z);
    glm::vec3 prevOrigin = (glm::vec3)cPhysics["PrevOrigin"];
    glm::vec3 toCurrentPos = boxA.Origin() - prevOrigin;
    float rayLength = glm::length(toCurrentPos) + 0.5f*diameter;
    //If the entity has moved farther than the size of its box, we need to handle it specially.
    bool traceCollision = rayLength > diameter;
    //hack solution: If prevOrigin is less than -9000 in all dimensions, 
    //then it means it is not set, i.e. this is the first collision check for the entity.
    if (traceCollision && glm::any(glm::greaterThan((glm::vec3)cPhysics["PrevOrigin"], glm::vec3(-9000.f)))) {
        Ray ray(prevOrigin, toCurrentPos);
        m_OctreeResult.clear();
        m_Octree->ObjectsPossiblyHitByRay(ray, m_OctreeResult);
        for (auto& boxB : m_OctreeResult) {
            if (boxA.Entity == boxB.Entity) {
                continue;
            }
            bool hit;
            float dist;
            if (boxB.Entity.HasComponent("Model")) {
                RawModel* model;
                std::string res = (std::string)boxB.Entity["Model"]["Resource"];
                try {
                    model = ResourceManager::Load<RawModel, true>(res);
                } catch (const std::exception&) {
                    continue;
                }
                float u, v;
                hit = Collision::RayVsModel(ray, model->CollisionVertices(), model->CollisionIndices(), Transform::ModelMatrix(boxB.Entity), dist, u, v);
            } else {
                hit = Collision::RayVsAABB(ray, boxB, dist);
            }
            if (hit && dist < rayLength) {
                //Set the entity to where it was colliding, minus the maximum box size. 
                //TODO: Perhaps this should be done slightly more properly.
                glm::vec3 newOriginPos = ray.Origin() + (dist - 0.707107f*diameter) * ray.Direction();
                glm::vec3 resolve = newOriginPos - boxA.Origin();
                (glm::vec3&)cTransform["Position"] += resolve;
                boxA = *Collision::EntityAbsoluteAABB(entity);
                if (resolve.y > 0) {
                    everHitTheGround = true;
                    (bool)cPhysics["IsOnGround"] = true;
                    ((glm::vec3&)cPhysics["Velocity"]).y = 0.f;
                }
                break;
            }
        }
    }

    // Collide against octree items
    m_OctreeResult.clear();
    m_Octree->ObjectsInSameRegion(*boundingBox, m_OctreeResult);
    for (auto& boxB : m_OctreeResult) {
        glm::vec3 resolutionVector;
        if (boxA.Entity == boxB.Entity) {
            continue;
        }

        if (boxB.Entity.HasComponent("Model") && Collision::AABBVsAABB(boxA, boxB)) {
            //Here we know boxB is a entity with Collideable, AABB, and Model.
            RawModel* model;
            try {
                model = ResourceManager::Load<RawModel, true>(boxB.Entity["Model"]["Resource"]);
            } catch (const std::exception&) {
                continue;
            }

            glm::mat4 modelMatrix = Transform::ModelMatrix(boxB.Entity);

            glm::vec3 inOutVelocity = (glm::vec3)cPhysics["Velocity"];
            bool isOnGround = (bool)cPhysics["IsOnGround"];
            float verticalStepHeight = (float)(double)cPhysics["VerticalStepHeight"];
            if (Collision::AABBvsTriangles(boxA, model->CollisionVertices(), model->CollisionIndices(), modelMatrix, inOutVelocity, verticalStepHeight, isOnGround, resolutionVector)) {
                (glm::vec3&)cTransform["Position"] += resolutionVector;
                cPhysics["Velocity"] = inOutVelocity;
                if (isOnGround) {
                    everHitTheGround = true;
                    (bool)cPhysics["IsOnGround"] = true;
                }
            }
        } else if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            //Enter here if boxB has no Model.
            (glm::vec3&)cTransform["Position"] += resolutionVector;
            if (resolutionVector.y > 0) {
                everHitTheGround = true;
                (bool)cPhysics["IsOnGround"] = true;
                ((glm::vec3&)cPhysics["Velocity"]).y = 0.f;
            }
        }
    }

    //This should apply air friction and such, iff zero models were hit.
    if (!everHitTheGround) {
        (bool)cPhysics["IsOnGround"] = false;
    }

    (glm::vec3&)cPhysics["PrevOrigin"] = boxA.Origin();
}
