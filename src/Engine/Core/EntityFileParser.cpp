#include "Core/EntityFileParser.h"

EntityFileParser::EntityFileParser(const EntityFile* entityFile) 
    : m_EntityFile(entityFile)
{
    m_Handler.SetStartEntityCallback(std::bind(&EntityFileParser::onStartEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_Handler.SetStartComponentCallback(std::bind(&EntityFileParser::onStartComponent, this, std::placeholders::_1, std::placeholders::_2));
    m_Handler.SetStartFieldCallback(std::bind(&EntityFileParser::onStartComponentField, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_Handler.SetStartFieldDataCallback(std::bind(&EntityFileParser::onFieldData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

EntityID EntityFileParser::MergeEntities(World* world, EntityID baseParent /*= EntityID_Invalid */)
{
    m_World = world;
    m_EntityIDMapper[0] = baseParent;
    m_EntityFile->Parse(&m_Handler);
    return m_FirstEntity;
}

void EntityFileParser::onStartEntity(EntityID entity, EntityID parent, const std::string& name)
{
    EntityID realParent = m_EntityIDMapper.at(parent);
    EntityID realEntity = m_World->CreateEntity(realParent);
    if (m_FirstEntity == EntityID_Invalid) {
        m_FirstEntity = realEntity;
    }
    if (!name.empty()) {
        m_World->SetName(realEntity, name);
    }
    m_EntityIDMapper[entity] = realEntity;
    //LOG_DEBUG("Created entity #%i (%i) with parent %i (%i)", entity, realEntity, parent, realParent);
}

void EntityFileParser::onStartComponent(EntityID entity, const std::string& component)
{
    EntityID realEntity = m_EntityIDMapper.at(entity);
    m_World->AttachComponent(realEntity, component);
    //LOG_DEBUG("Attached component of type \"%s\" to entity #%i (%i)", component.c_str(), entity, realEntity);
}

void EntityFileParser::onStartComponentField(EntityID entity, const std::string& componentType, const std::string& fieldName, const std::map<std::string, std::string>& attributes)
{
    EntityID realEntity = m_EntityIDMapper.at(entity);
    ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
    auto fieldIt = component.Info.Fields.find(fieldName);
    if (fieldIt == component.Info.Fields.end()) {
        LOG_ERROR("Tried to set unknown field \"%s\" of component type \"%s\"! Ignoring.", fieldName.c_str(), componentType.c_str());
        return;
    }
    auto& field = fieldIt->second;

    //LOG_DEBUG("Field \"%s\" type \"%s\"", fieldName.c_str(), field.Type.c_str());
    //LOG_DEBUG("Attributes:");
    //for (auto& kv : attributes) {
    //    LOG_DEBUG("\t%s = %s", kv.first.c_str(), kv.second.c_str());
    //}

    char* data = component.Data + field.Offset;
    EntityFile::WriteAttributeData(data, field, attributes);
}

void EntityFileParser::onFieldData(EntityID entity, const std::string& componentType, const std::string& fieldName, const char* fieldData)
{
    EntityID realEntity = m_EntityIDMapper.at(entity);
    ComponentWrapper component = m_World->GetComponent(realEntity, componentType);
    auto fieldIt = component.Info.Fields.find(fieldName);
    if (fieldIt == component.Info.Fields.end()) {
        return;
    }
    auto& field = fieldIt->second;

    char* data = component.Data + field.Offset;
    EntityFile::WriteValueData(data, field, fieldData);
}