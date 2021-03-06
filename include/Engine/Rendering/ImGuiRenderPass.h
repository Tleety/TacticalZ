#include <imgui/imgui.h>
#include "../OpenGL.h"
#include "IRenderer.h"
#include "RenderState.h"
#include "../Core/EventBroker.h"
#include "../Core/EMousePress.h"
#include "../Core/EMouseRelease.h"
#include "../Core/EMouseMove.h"
#include "../Core/EMouseScroll.h"
#include "../Core/EKeyDown.h"
#include "../Core/EKeyUp.h"
#include "../Core/EKeyboardChar.h"

class ImGuiRenderState : public RenderState
{
public:
    ImGuiRenderState()
        : RenderState()
    {
        BindFramebuffer(0);
        Enable(GL_BLEND);
        BlendEquation(GL_FUNC_ADD);
        BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Disable(GL_CULL_FACE);
        Disable(GL_DEPTH_TEST);
        Enable(GL_SCISSOR_TEST);
    }
};

class ImGuiRenderPass
{
public:
    ImGuiRenderPass(IRenderer* renderer, EventBroker* eventBroker);

    void Update(double dt);
    void Draw();

private:
    IRenderer* m_Renderer;
    EventBroker* m_EventBroker;

    GLFWwindow* g_Window;
    double g_DeltaTime = 0.0;
    float g_MouseWheel = 0.f;
    GLuint g_FontTexture;
    int g_ShaderHandle;
    int g_VertHandle;
    int g_FragHandle;
    int g_AttribLocationTex;
    int g_AttribLocationProjMtx;
    int g_AttribLocationPosition;
    int g_AttribLocationUV;
    int g_AttribLocationColor;
    GLuint g_VboHandle;
    GLuint g_VaoHandle;
    GLuint g_ElementsHandle;

    EventRelay<ImGuiRenderPass, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<ImGuiRenderPass, Events::MouseRelease> m_EMouseRelease;
    bool OnMouseRelease(const Events::MouseRelease& e);
    EventRelay<ImGuiRenderPass, Events::MouseMove> m_EMouseMove;
    bool OnMouseMove(const Events::MouseMove& e);
    EventRelay<ImGuiRenderPass, Events::MouseScroll> m_EMouseScroll;
    bool OnMouseScroll(const Events::MouseScroll& e);
    EventRelay<ImGuiRenderPass, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown& e);
    EventRelay<ImGuiRenderPass, Events::KeyUp> m_EKeyUp;
    bool OnKeyUp(const Events::KeyUp& e);
    EventRelay<ImGuiRenderPass, Events::KeyboardChar> m_EKeyboardChar;
    bool OnKeyboardChar(const Events::KeyboardChar& e);

    bool createDeviceObjects();
    bool createFontsTexture();

    void newFrame();
};