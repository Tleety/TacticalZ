#include <vector>
#include <algorithm>
#include <bitset>

#include "Core/OctTree.h"
#include "Core/Collision.h"
#include "Core/World.h"
#include "Rendering/Camera.h"

namespace
{
//To be able to sort nodes based on distance to ray origin.
struct ChildInfo
{
    int Index;
    float Distance;
};

bool isFirstLower(const ChildInfo& first, const ChildInfo& second)
{
    return first.Distance < second.Distance;
}

bool isSameBoxProbably(const AABB& first, const AABB& second)
{
    const float EPS = 0.0001f;
    const auto& ma = first.MaxCorner();
    const auto& mi = first.MinCorner();
    return (std::abs(ma.x - mi.x) < EPS) &&
        (std::abs(ma.z - mi.z) < EPS) &&
        (std::abs(ma.y - mi.y) < EPS);
}

}

OctTree::OctTree()
    : OctTree(AABB(), 0)
{}

OctTree::OctTree(const AABB& octTreeBounds, int subDivisions)
    : m_Root(new OctChild(octTreeBounds, subDivisions, m_StaticObjects, m_DynamicObjects))
    , m_UpdatedOnce(false)
{}

OctTree::~OctTree()
{
    delete m_Root;
}

void OctTree::AddDynamicObject(const AABB& box)
{
    m_Root->AddDynamicObject(box);
    m_DynamicObjects.push_back(box);
}

void OctTree::AddStaticObject(const AABB& box)
{
    m_Root->AddStaticObject(box);
    m_StaticObjects.push_back(box);
}

void OctTree::BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes)
{
    falsifyObjectChecks();
    m_Root->BoxesInSameRegion(box, outBoxes);
}

void OctTree::ClearObjects()
{
    m_StaticObjects.clear();
    m_DynamicObjects.clear();
    m_Root->ClearObjects();
}

void OctTree::ClearDynamicObjects()
{
    m_DynamicObjects.clear();
    m_Root->ClearDynamicObjects();
}

bool OctTree::RayCollides(const Ray& ray, Output& data)
{
    falsifyObjectChecks();
    data.CollideDistance = -1;
    return m_Root->RayCollides(ray, data);
}

bool OctTree::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected)
{
    falsifyObjectChecks();
    return m_Root->BoxCollides(boxToTest, outBoxIntersected);
}

void OctTree::falsifyObjectChecks()
{
    for (auto& obj : m_StaticObjects) {
        obj.Checked = false;
    }
    for (auto& obj : m_DynamicObjects) {
        obj.Checked = false;
    }
}

OctTree::OctChild::OctChild(const AABB& octTreeBounds,
    int subDivisions, 
    std::vector<ContainedObject>& staticObjects,
    std::vector<ContainedObject>& dynamicObjects)
    : m_Box(octTreeBounds)
    , m_StaticObjectsRef(staticObjects)
    , m_DynamicObjectsRef(dynamicObjects)
{
    if (subDivisions == 0) {
        for (OctChild*& c : m_Children) {
            c = nullptr;
        }
    } else {
        --subDivisions;
        for (int i = 0; i < 8; ++i) {
            glm::vec3 minPos, maxPos;
            const glm::vec3& parentMin = m_Box.MinCorner();
            const glm::vec3& parentMax = m_Box.MaxCorner();
            const glm::vec3& parentCenter = m_Box.Center();
            std::bitset<3> bits(i);
            //If child is 4,5,6,7.
            if (bits.test(2)) {
                minPos.x = parentCenter.x;
                maxPos.x = parentMax.x;
            } else {
                minPos.x = parentMin.x;
                maxPos.x = parentCenter.x;
            }

            //If child is 2,3,6,7
            if (bits.test(1)) {
                minPos.y = parentCenter.y;
                maxPos.y = parentMax.y;
            } else {
                minPos.y = parentMin.y;
                maxPos.y = parentCenter.y;
            }
            //If child is 1,3,5,7
            if (bits.test(0)) {
                minPos.z = parentCenter.z;
                maxPos.z = parentMax.z;
            } else {
                minPos.z = parentMin.z;
                maxPos.z = parentCenter.z;
            }
            m_Children[i] = new OctChild(AABB(minPos, maxPos), subDivisions, m_StaticObjectsRef, m_DynamicObjectsRef);
        }
    }
}

OctTree::OctChild::~OctChild()
{
    for (OctChild*& c : m_Children) {
        if (c != nullptr) {
            delete c;
            c = nullptr;
        }
    }
}

void OctTree::Update(float dt, World* world, Camera* cam)
{
    for (ComponentWrapper& c : world->GetComponents("Collision")) {
        AABB aabb;
        aabb.CreateFromCenter(c["BoxCenter"], c["BoxSize"]);
        AddDynamicObject(aabb);
    }
    const glm::vec4 redCol = glm::vec4(1, 0.2f, 0, 1);
    const glm::vec4 greenCol = glm::vec4(0.1f, 1.0f, 0.25f, 1);
    const glm::vec4 blueCol = glm::vec4(0.1f, 0.05f, 0.95f, 1);
    const glm::vec4 cyanCol = glm::vec4(0.1f, 0.9f, 0.85f, 1);
    const glm::vec3 boxSize = 0.05f*glm::vec3(1.0f, 1.0f, 1.0f);

    if (!m_UpdatedOnce) {
        m_BoxID = world->CreateEntity();
        ComponentWrapper transform = world->AttachComponent(m_BoxID, "Transform");
        transform["Scale"] = boxSize;

        ComponentWrapper model = world->AttachComponent(m_BoxID, "Model");
        model["Resource"] = "Models/Core/UnitBox.obj";
        m_UpdatedOnce = true;
    }

    AABB box;
    auto boxPos = cam->Position() + 1.2f*cam->Forward();
    box.CreateFromCenter(boxPos, boxSize);
    ComponentWrapper transform = world->GetComponent(m_BoxID, "Transform");
    transform["Position"] = boxPos;
    ComponentWrapper model = world->GetComponent(m_BoxID, "Model");
    bool collBox = BoxCollides(box, AABB());
    if (collBox) {
        cam->SetPosition(m_PrevPos);
        cam->SetOrientation(m_PrevOri);
        bool collRay = RayCollides({ cam->Position(), cam->Forward() }, Output());
        model["Color"] = collRay ? cyanCol : greenCol;
    } else if (RayCollides({ cam->Position(), cam->Forward() }, Output())) {
        model["Color"] = blueCol;
    } else {
        model["Color"] = redCol;
    }

    m_PrevPos = cam->Position();
    m_PrevOri = cam->Orientation();
    ClearDynamicObjects();
}

bool OctTree::OctChild::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const
{
    if (hasChildren()) {
        for (int i : childIndicesContainingBox(boxToTest)) {
            if (m_Children[i]->BoxCollides(boxToTest, outBoxIntersected))
                return true;
        }
    } else {
        for (int i : m_StaticObjIndices) {
            if (!m_StaticObjectsRef[i].Checked) {
                const AABB& objBox = m_StaticObjectsRef[i].Box;
                if (Collision::AABBVsAABB(boxToTest, objBox)) {
                    outBoxIntersected = objBox;
                    return true;
                }
                m_StaticObjectsRef[i].Checked = true;
            }
        }
        for (int i : m_DynamicObjIndices) {
            if (!m_DynamicObjectsRef[i].Checked) {
                const AABB& objBox = m_DynamicObjectsRef[i].Box;
                if (!isSameBoxProbably(boxToTest, objBox) &&
                    Collision::AABBVsAABB(boxToTest, objBox)) {
                    outBoxIntersected = objBox;
                    return true;
                }
                m_DynamicObjectsRef[i].Checked = true;
            }
        }
    }
    return false;
}

bool OctTree::OctChild::RayCollides(const Ray& ray, Output& data) const
{
    //If the node AABB is missed, everything it contains is missed.
    if (Collision::RayAABBIntr(ray, m_Box)) {
        //If the ray shoots the tree, and it is a parent to 8 children :o
        if (hasChildren()) {
            //Sort children according to their distance from the ray origin.
            std::vector<ChildInfo> childInfos;
            childInfos.reserve(8);
            for (int i = 0; i < 8; ++i) {
                childInfos.push_back({ i, glm::distance(ray.Origin, m_Children[i]->m_Box.Center()) });
            }
            std::sort(childInfos.begin(), childInfos.end(), isFirstLower);
            //Loop through the children, starting with the one closest to the ray origin. I.e the first to be hit.
            for (const ChildInfo& info : childInfos) {
                if (m_Children[info.Index]->RayCollides(ray, data)) {
                    return true;
                }
            }
        } else {
            //Check against boxes in the node.
            float minDist = INFINITY;
            bool intersected = false;
            for (int i : m_StaticObjIndices) {
                float dist;
                //If we haven't tested against this object before, and the ray hits.
                if (!m_StaticObjectsRef[i].Checked &&
                    Collision::RayVsAABB(ray, m_StaticObjectsRef[i].Box, dist)) {
                    minDist = std::min(dist, minDist);
                    intersected = true;
                }
                m_StaticObjectsRef[i].Checked = true;
            }
            for (int i : m_DynamicObjIndices) {
                float dist;
                //If we haven't tested against this object before, and the ray hits.
                if (!m_DynamicObjectsRef[i].Checked &&
                    Collision::RayVsAABB(ray, m_DynamicObjectsRef[i].Box, dist)) {
                    minDist = std::min(dist, minDist);
                    intersected = true;
                }
                m_DynamicObjectsRef[i].Checked = true;
            }

            data.CollideDistance = minDist;
            return intersected;
        }
    }
    return false;
}


void OctTree::OctChild::AddDynamicObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddDynamicObject(box);
        }
    } else {
        //Since it hasn't been added yet to the real object list, the index is after the last =size.
        m_DynamicObjIndices.push_back(m_DynamicObjectsRef.size());
    }
}

void OctTree::OctChild::AddStaticObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddStaticObject(box);
        }
    } else {
        //Since it hasn't been added yet to the real object list, the index is after the last =size.
        m_StaticObjIndices.push_back(m_StaticObjectsRef.size());
    }
}

void OctTree::OctChild::BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes) const
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->BoxesInSameRegion(box, outBoxes);
        }
    } else {
        int startIndex = outBoxes.size();
        int numDuplicates = 0;
        outBoxes.resize(outBoxes.size() + m_StaticObjIndices.size() + m_DynamicObjIndices.size());
        for (size_t i = 0; i < m_StaticObjIndices.size(); ++i){
            ContainedObject& obj = m_StaticObjectsRef[m_StaticObjIndices[i]];
            if (obj.Checked) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outBoxes[startIndex + i - numDuplicates] = obj.Box;
            }
        }
        for (size_t i = 0; i < m_DynamicObjIndices.size(); ++i) {
            ContainedObject& obj = m_DynamicObjectsRef[m_DynamicObjIndices[i]];
            if (obj.Checked) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outBoxes[startIndex + i - numDuplicates] = obj.Box;
            }
        }
        for (size_t i = 0; i < numDuplicates; ++i) {
            outBoxes.pop_back();
        }
    }
}

void OctTree::OctChild::ClearObjects()
{
    if (hasChildren()) {
        for (OctChild*& c : m_Children) {
            c->ClearObjects();
        }
    } else {
        m_DynamicObjIndices.clear();
        m_StaticObjIndices.clear();
    }
}

void OctTree::OctChild::ClearDynamicObjects()
{
    if (hasChildren()) {
        for (OctChild*& c : m_Children) {
            c->ClearObjects();
        }
    } else {
        m_DynamicObjIndices.clear();
    }
}

//:     3        7 
//:
//:        2         6 
//:                           |
//:     1        5          \ y
//:                          z 
//:        0         4        0  x-->
//
// child: 0 1 2 3 4 5 6 7
//    x : - - - - + + + +
//    y : - - + + - - + +
//    z : - + - + - + - +
int OctTree::OctChild::childIndexContainingPoint(const glm::vec3& point) const
{
    const glm::vec3& c = m_Box.Center();
    return (1 << 2) * (point.x >= c.x) | (1 << 1) * (point.y >= c.y) | (point.z >= c.z);
}

std::vector<int> OctTree::OctChild::childIndicesContainingBox(const AABB& box) const
{
    int minInd = childIndexContainingPoint(box.MinCorner());
    int maxInd = childIndexContainingPoint(box.MaxCorner());
    //Because of the predictable ordering of the child indices, 
    //the number of bits set when xor:ing the indices will determine the number of children containing the box.
    std::bitset<3> bits(minInd ^ maxInd);
    switch (bits.count()) {
        //Box contained completely in one child.
    case 0:
        return{ minInd };
        //Two children.
    case 1:
        return{ minInd, maxInd };
        //Four children.
    case 2:
    {
        std::vector<int> ret;
        //Bit-hax to calculate the correct 4 children containing the box.
        //This works because of the childrens index determine what part of 
        //the dimensions they are responsible for (which octant).
        bits.flip();
        //At this point the bits necessarily have exactly one bit set.
        for (int c = 0; c < 8; ++c) {
            //If the child index have the same bit set as the bits, add box to it.
            if (bits.to_ulong() & c) {
                ret.push_back(c);
            }
        }
        return ret;
    }
    case 3:                         //Eight children.
        return{ 0,1,2,3,4,5,6,7 };
    default:
        return std::vector<int>();
    }
}

inline bool OctTree::OctChild::hasChildren() const
{
    return m_Children[0] != nullptr;
}