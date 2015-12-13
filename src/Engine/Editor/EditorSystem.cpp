#include "Editor/EditorSystem.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

EditorSystem::EditorSystem(EventBroker* eventBroker) 
    : ImpureSystem(eventBroker)
{
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &EditorSystem::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EPicking, &EditorSystem::OnPicking);
}

void EditorSystem::Update(World* world, double dt)
{
    if (m_Widget == 0) {
        m_Widget = world->CreateEntity();
        world->AttachComponent(m_Widget, "Transform");
        auto& model = world->AttachComponent(m_Widget, "Model");
        model["Resource"] = "Models/TranslationWidget.obj";
    }

    if (m_Selection != m_LastSelection) {
        
    }

    if (m_Selection != 0) {
        auto selectionTransform = world->GetComponent(m_Selection, "Transform");
        auto widgetTransform = world->GetComponent(m_Widget, "Transform");

        widgetTransform["Position"] = (glm::vec3)selectionTransform["Position"];
    }
    
    drawUI(world, dt);
}

bool EditorSystem::OnMousePress(const Events::MousePress& e)
{
    if (e.Button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_PickingQueue.push_back(glm::vec2(e.X, e.Y));
    }
    return true;
}

bool EditorSystem::OnPicking(const Events::Picking& e)
{
    for (auto& pos : m_PickingQueue) {
        auto result = e.Pick(pos);
        LOG_INFO("Selected %i", result.Entity);
        m_Selection = result.Entity;
    }
    m_PickingQueue.clear();
    return true;
};

void EditorSystem::drawUI(World* world, double dt)
{
    //ImGui::ShowTestWindow();
    //ImGui::ShowStyleEditor();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("New")) { }
            if (ImGui::MenuItem("Open", "Ctrl+O")) { }
            if (ImGui::MenuItem("Save", "Ctrl+S")) { }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { }
            ImGui::Separator();
            if (ImGui::MenuItem("Close Editor", "F1")) { }

            ImGui::EndMenu();
        }

        ImGui::SameLine();
        if (ImGui::Button("Move")) {
            auto& model = world->GetComponent(m_Widget, "Model");
            model["Resource"] = "Models/TranslationWidget.obj";
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate")) {
            auto& model = world->GetComponent(m_Widget, "Model");
            model["Resource"] = "Models/RotationWidget.obj";
        }
        ImGui::SameLine();
        if (ImGui::Button("Scale")) {
            auto& model = world->GetComponent(m_Widget, "Model");
            model["Resource"] = "Models/ScaleWidget.obj";
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Properties")) {
        if (m_Selection != 0) {
            auto& pools = world->GetComponentPools();

            std::vector<const char*> componentTypes;
            for (auto& pair : pools) {
                // Only add components the entity doesn't already have
                if (!pair.second->KnowsEntity(m_Selection)) {
                    componentTypes.push_back(pair.first.c_str());
                }
            }
            int item = -1;
            ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() - 5.f);
            if (ImGui::Combo("", &item, componentTypes.data(), componentTypes.size())) {
                if (item != -1) {
                    std::string chosenType = std::string(componentTypes.at(item));
                    world->AttachComponent(m_Selection, chosenType);
                }
            }
            ImGui::PopItemWidth();

            for (auto& pair : pools) {
                const std::string& componentType = pair.first;
                auto pool = pair.second;
                if (!pool->KnowsEntity(m_Selection)) {
                    continue;
                }
                auto& ci = pool->ComponentInfo();

                bool deletePressed = createDeleteButton(componentType);
                if (deletePressed) {
                    world->DeleteComponent(m_Selection, componentType);
                    continue;
                }

                if (ImGui::CollapsingHeader(componentType.c_str())) {
                    if (!ci.Meta.Annotation.empty()) {
                        ImGui::Text(ci.Meta.Annotation.c_str());
                    }

                    auto& component = world->GetComponent(m_Selection, componentType);
                    for (auto& pair : ci.FieldTypes) {
                        const std::string& field = pair.first;
                        const std::string& type = pair.second;
                        
                        if (type == "Vector") {
                            auto& val = component.Property<glm::vec3>(field);
                            if (field == "Scale") {
                                ImGui::DragFloat3(field.c_str(), glm::value_ptr(val), 0.1f, 0.f, 9999.f);
                            } else if (field == "Orientation") {
                                ImGui::SliderFloat3(field.c_str(), glm::value_ptr(val), 0.f, glm::pi<float>());
                            } else {
                                ImGui::InputFloat3(field.c_str(), glm::value_ptr(val));
                            }
                        } else if (type == "Color") {
                            auto& val = component.Property<glm::vec4>(field);
                            ImGui::ColorEdit4(field.c_str(), glm::value_ptr(val), true);
                        } else if (type == "string") {
                            std::string& val = component.Property<std::string>(field);
                            char tempString[1024];
                            memcpy(tempString, val.c_str(), std::min(val.length() + 1, sizeof(tempString)));
                            if (ImGui::InputText(field.c_str(), tempString, sizeof(tempString))) {
                                val = std::string(tempString);
                                LOG_DEBUG("%s::%s changed!", componentType.c_str(), field.c_str());
                            }
                        } else if (type == "double") {
                            float tempVal = static_cast<float>(component.Property<double>(field));
                            if (ImGui::InputFloat(field.c_str(), &tempVal, 0.01f, 1.f)) {
                                component.SetProperty(field, static_cast<double>(tempVal));
                            }
                        }
                    }
                }
            }
        }

    }
    ImGui::End();
}

bool EditorSystem::createDeleteButton(std::string componentType)
{
    float width = ImGui::GetContentRegionAvailWidth();
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    auto pos = ImGui::GetCursorScreenPos() + ImVec2(width - 14.f, 1);
    ImRect bb = ImRect(pos, pos + ImVec2(14.f, 14.f));
    std::string idString = "#DELETE";
    idString += componentType;
    ImGuiID id = window->GetID(idString.c_str());
    bool hovered;
    bool held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    //ImU32 col = window->Color((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_CloseButtonHovered : ImGuiCol_CloseButton);
    ImU32 col = window->Color((held && hovered) ? ImGuiCol_CloseButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    window->DrawList->AddCircleFilled(bb.GetCenter(), 7.f, col, 16);
    return pressed;
}
