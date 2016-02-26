#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"
#include "Util/UnorderedMapVec2.h"
#include "FrameBuffer.h"
#include "../Core/World.h"
#include "PickingPass.h"
#include "LightCullingPass.h"
#include "DrawFinalPass.h"
#include "DrawScreenQuadPass.h"
#include "DrawBloomPass.h"
#include "DrawColorCorrectionPass.h"
#include "SSAOPass.h"
#include "CubeMapPass.h"
#include "../Core/EventBroker.h"
#include "ImGuiRenderPass.h"
#include "Camera.h"
#include "../Core/Transform.h"
#include "imgui/imgui.h"
#include "TextPass.h"
#include "Util/CommonFunctions.h"
#include "Core/PerformanceTimer.h"

class Renderer : public IRenderer
{
    static void glfwFrameBufferCallback(GLFWwindow* window, int width, int height);

public:
	Renderer(EventBroker* eventBroker, ConfigFile* config)
		: m_EventBroker(eventBroker)
		, m_Config(config)
    { }

    virtual void Initialize() override;
    virtual void Update(double dt) override;
    virtual void Draw(RenderFrame& frame) override;

    virtual PickData Pick(glm::vec2 screenCoord) override;


private:
    //----------------------Variables----------------------//

    static std::unordered_map <GLFWwindow*, Renderer*> m_WindowToRenderer;
	ConfigFile* m_Config;

    EventBroker* m_EventBroker;
    TextPass* m_TextPass;

    Texture* m_ErrorTexture;
    Texture* m_WhiteTexture;

    Model* m_ScreenQuad;
    Model* m_UnitQuad;
    Model* m_UnitSphere;

    int m_DebugTextureToDraw = 0;
    int m_CubeMapTexture = 0;
    bool m_ResizeWindow = false;
	float m_SSAO_Radius = 1.0f;
	float m_SSAO_Bias = 0.05f;
	float m_SSAO_Contrast = 1.5f;
	float m_SSAO_IntensityScale = 1.0f;
	int m_SSAO_NumOfSamples = 24;
	int m_SSAO_NumOfTurns = 7;
	int m_SSAO_iterations = 9;
	int m_SSAO_TextureQuality = 0;
	int m_SSAO_Quality = 0;

    PickingPass* m_PickingPass;
    LightCullingPass* m_LightCullingPass;
    ImGuiRenderPass* m_ImGuiRenderPass;
    DrawFinalPass* m_DrawFinalPass;
    DrawScreenQuadPass* m_DrawScreenQuadPass;
    DrawBloomPass* m_DrawBloomPass;
    DrawColorCorrectionPass* m_DrawColorCorrectionPass;
	SSAOPass* m_SSAOPass;
    CubeMapPass* m_CubeMapPass;

    //----------------------Functions----------------------//
    void InitializeWindow();
    void InitializeShaders();
    void InitializeTextures();
    void InitializeRenderPasses();
    //TODO: Renderer: Get InputUpdate out of renderer
    void InputUpdate(double dt);
    //void PickingPass(RenderQueueCollection& rq);
    //void DrawScreenQuad(GLuint textureToDraw);

    static bool DepthSort(const std::shared_ptr<RenderJob> &i, const std::shared_ptr<RenderJob> &j) { return (i->Depth < j->Depth); }
    void SortRenderJobsByDepth(RenderScene &scene);
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
	//--------------------ShaderPrograms-------------------//
    ShaderProgram* m_BasicForwardProgram;
    ShaderProgram* m_ExplosionEffectProgram;

};

#endif