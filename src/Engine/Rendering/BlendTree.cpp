#include "Rendering/BlendTree.h"

BlendTree::BlendTree(EntityWrapper ModelEntity, Skeleton* skeleton)
{

    m_Skeleton = skeleton;



    if (ModelEntity.HasComponent("Animation")) {
        const Skeleton::Animation* animation = skeleton->GetAnimation(ModelEntity["Animation"]["AnimationName"]);
        if (animation == nullptr) {
            return;
        }

        m_Root = new Node();
        m_Root->Entity = ModelEntity;
        m_Root->Name = ModelEntity.Name();
        m_Root->Pose = m_Skeleton->GetFrameBones(animation, (double)ModelEntity["Animation"]["Time"], (bool)ModelEntity["Animation"]["Additive"]);
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Animation;

    } else if (ModelEntity.HasComponent("Blend")) {
        m_Root = new Node();
        m_Root->Entity = ModelEntity;
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Blend;
        m_Root->Weight = (double)ModelEntity["Blend"]["Weight"];
        (double&)ModelEntity["Blend"]["Weight"] = glm::clamp((double)ModelEntity["Blend"]["Weight"], 0.0, 1.0);
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose1"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose2"], ModelEntity);

    } else if (ModelEntity.HasComponent("BlendOverride")) {
        m_Root = new Node();
        m_Root->Entity = ModelEntity;
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Override;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Master"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Slave"], ModelEntity);

    } else if (ModelEntity.HasComponent("BlendAdditive")) {
        m_Root = new Node();
        m_Root->Entity = ModelEntity;
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Additive;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Adder"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Receiver"], ModelEntity);
    }

    m_FinalPose = AccumulateFinalPose();


   // PrintTree();
}

BlendTree::~BlendTree()
{
    Node* currentNode = m_Root;
    if (currentNode != nullptr) {
        while (currentNode->Child[0] != nullptr) {
            currentNode = currentNode->Child[0];
        }

        std::list<Node*> m_NodesToRemove;

        while (currentNode != nullptr) {
            m_NodesToRemove.push_back(currentNode);
            currentNode = currentNode->Next();
        }

        for (auto it = m_NodesToRemove.begin(); it != m_NodesToRemove.end(); it++) {
            delete (*it);
        }
    }
}


glm::mat4 BlendTree::GetBoneTransform(int boneID)
{
    if(m_FinalBoneTransforms.find(boneID) != m_FinalBoneTransforms.end()) {
        return m_FinalBoneTransforms.at(boneID);
    } else {
        return glm::mat4(1);
    }
    
}

void BlendTree::PrintTree()
{
    Node* currentNode = m_Root;
    LOG_INFO("\n\n");

    while(currentNode->Child[0] != nullptr) {
        currentNode = currentNode->Child[0];
    }

    while (currentNode != nullptr)
    {
        LOG_INFO("%s", currentNode->Name.c_str());
        currentNode = currentNode->Next();
    }
}

BlendTree::Node* BlendTree::FillTreeByName(Node* parentNode, std::string name, EntityWrapper parentEntity)
{
    EntityWrapper childEntity = parentEntity.FirstLevelChildByName(name); // Make first level child by name

    if (!childEntity.Valid()) {
        return nullptr;
    }

    if (childEntity.HasComponent("Animation")) {
        const Skeleton::Animation* animation = m_Skeleton->GetAnimation(childEntity["Animation"]["AnimationName"]);
        if (animation == nullptr) {
            return nullptr;
        }

        Node* node = new Node();
        node->Entity = childEntity;
        node->Name = childEntity.Name();
        node->Pose = m_Skeleton->GetFrameBones(animation, (double)childEntity["Animation"]["Time"], (bool)childEntity["Animation"]["Additive"]);
        node->Parent = parentNode;
        node->Type = NodeType::Animation;
        return node;

    } else if (childEntity.HasComponent("Blend")) {
        Node* node = new Node();
        node->Entity = childEntity;
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Blend;
        (double&)childEntity["Blend"]["Weight"] = glm::clamp((double)childEntity["Blend"]["Weight"], 0.0, 1.0);
        node->Weight = (double)childEntity["Blend"]["Weight"];
        //if (node->Weight < 1.f && node->Weight > 0.f) {
            node->Child[0] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose1"], childEntity);
            node->Child[1] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose2"], childEntity);
       /* } else if (node->Weight == 1.f) {
            node->Child[0] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose1"], childEntity);
        } else if (node->Weight == 0.f) {
            node->Child[1] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose2"], childEntity);
        }*/
        
        if(node->Child[0] == nullptr && node->Child[1] == nullptr) {
            return nullptr;
        } else {
            return node;
        }

    } else if (childEntity.HasComponent("BlendOverride")) {
        Node* node = new Node();
        node->Entity = childEntity;
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Override;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Master"], childEntity);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Slave"], childEntity);
        
        if (node->Child[0] == nullptr && node->Child[1] == nullptr) {
            return nullptr;
        } else {
            return node;
        }
    } else if (childEntity.HasComponent("BlendAdditive")) {
        Node* node = new Node();
        node->Entity = childEntity;
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Additive;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Adder"], childEntity);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Receiver"], childEntity);
        
        if(node->Child[0] == nullptr && node->Child[1] == nullptr) {
            return nullptr;
        } else {
            return node;
        }
    }
    

    return nullptr;
}


std::vector<BlendTree::Node*> BlendTree::FindNodesByName(std::string name)
{
    std::vector<Node*> Nodes;
    Node* currentNode = m_Root;

    while (currentNode->Child[0] != nullptr) {
        currentNode = currentNode->Child[0];
    }

    while (currentNode != nullptr) {
        if(currentNode->Name == name) {
            Nodes.push_back(currentNode);
        }
        currentNode = currentNode->Next();
    }
    return Nodes;
}


BlendTree::AutoBlendInfo BlendTree::AutoBlendStep(AutoBlendInfo blendInfo)
{
    std::vector<Node*> goalNodes = FindNodesByName(blendInfo.NodeName);

    if(goalNodes.size() == 0) {
        return blendInfo;
    } else if(goalNodes.size() == 1) {
        Node* currentNode = goalNodes[0]->Parent;
        Node* lastNode = goalNodes[0];

        while (currentNode != nullptr)
        {
            

            if(!currentNode->Entity.HasComponent("Blend")) {
                return blendInfo;
            }

            double startWeight;
            if(blendInfo.StartWeights.find(currentNode->Entity) != blendInfo.StartWeights.end()) {
                startWeight = blendInfo.StartWeights.at(currentNode->Entity);
            } else {
                startWeight = currentNode->Weight;
                blendInfo.StartWeights[currentNode->Entity] = startWeight;
            }

            double goalWeight;
            if(currentNode->Child[0] == lastNode) {
                goalWeight = 0.0;
            } else if (currentNode->Child[1] == lastNode) {
                goalWeight = 1.0;
            }

            double weight = ((goalWeight - startWeight) * blendInfo.progress) + startWeight;
            (double&)currentNode->Entity["Blend"]["Weight"] = weight;
            currentNode->Weight = weight;

            lastNode = currentNode;
            currentNode = currentNode->Parent;
        }


    } else if(goalNodes.size() >= 2) {

    }

    return blendInfo;
}

void BlendTree::Blend(std::map<int, Skeleton::PoseData>& pose)
{
    Node* currentNode;
    Node* start = m_Root;
    while (start->Child[0] != nullptr) {
        start = start->Child[0];
    }

    currentNode = start;

    while (m_Root->Pose.size() == 0) {
        if(currentNode->Pose.size() == 0) {
            if (currentNode->Child[0] != nullptr && currentNode->Child[1] != nullptr) {
                if (currentNode->Child[0]->Pose.size() != 0 && currentNode->Child[1]->Pose.size() != 0) {
                    switch (currentNode->Type) {
                    case BlendTree::NodeType::Additive:
                        currentNode->Pose = m_Skeleton->BlendPoseAdditive(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Blend:
                        currentNode->Pose = m_Skeleton->BlendPoses(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose, currentNode->Weight);
                        break;
                    case BlendTree::NodeType::Override:
                        currentNode->Pose = m_Skeleton->OverridePose(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Animation:
                        // do nothing
                        break;
                    } 
                } 
            } else if (currentNode->Child[0] != nullptr) {
                if (currentNode->Child[0]->Pose.size() != 0) {
                    currentNode->Pose = currentNode->Child[0]->Pose;
                }
            } else if (currentNode->Child[1] != nullptr) {
                if (currentNode->Child[1]->Pose.size() != 0) {
                    currentNode->Pose = currentNode->Child[1]->Pose;
                }
            }
        } 

        currentNode = currentNode->Next();

        if (currentNode == nullptr) {
            currentNode = start;
        }
        
    }

    pose = m_Root->Pose;
}

std::vector<glm::mat4> BlendTree::AccumulateFinalPose()
{
    std::vector<glm::mat4> finalPose;
    if (m_Skeleton == nullptr || m_Root == nullptr || (m_Root->Child[0] == nullptr && m_Root->Child[1] == nullptr)) {
        
        for (int i = 0; i < m_Skeleton->Bones.size(); i++) {
            finalPose.push_back(glm::mat4(1));
        }
        return finalPose;
    }

    std::map<int, Skeleton::PoseData> pose;
    Blend(pose);

    m_Skeleton->GetFinalPose(pose, finalPose, m_FinalBoneTransforms);

    return finalPose;
}

