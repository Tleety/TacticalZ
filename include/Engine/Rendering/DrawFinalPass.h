#ifndef DrawFinalPass_h__
#define DrawFinalPass_h__

#include "IRenderer.h"
#include "DrawFinalPassState.h"
#include "LightCullingPass.h"
#include "CubeMapPass.h"
#include "SSAOPass.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Util/CommonFunctions.h"
#include "Texture.h"
#include "ShadowPass.h"
#include "BlurHUD.h"

class DrawFinalPass
{
public:
    DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass, CubeMapPass* cubeMapPass, SSAOPass* ssaoPass, ShadowPass* shadowPass, ConfigFile* config);
	~DrawFinalPass();
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void Draw(RenderScene& scene, BlurHUD* blurHUDPass);
    void ClearBuffer();
    void OnWindowResize();
	void setMSAA(unsigned int numberOfSamples);

    //Return the texture that is used in later stages to apply the bloom effect
	GLuint BloomTexture() {
		if (m_MSAA){
			m_AntiAliasedFrameBuffer->Bind();
			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_AntiAliasedFrameBuffer->Unbind();
			m_FinalPassFrameBuffer->Read();
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			m_AntiAliasedFrameBuffer->Draw();
			glBlitFramebuffer(
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			return m_AntiAliasedTexture;
		}
		return m_BloomTexture; }
    //Return the texture with diffuse and lighting of the scene.
	GLuint DrawFinalPass::SceneTexture() {
		if (m_MSAA) {
			m_AntiAliasedFrameBuffer->Bind();
			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_AntiAliasedFrameBuffer->Unbind();
			m_FinalPassFrameBuffer->Read();
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			m_AntiAliasedFrameBuffer->Draw();
			glBlitFramebuffer(
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			return m_AntiAliasedTexture;
		}
		return m_SceneTexture;
	}
    //Return the SceneTexture with the blurred HUD bits.
    GLuint CombinedSceneTexture() {
		if (m_MSAA) {
			m_AntiAliasedFrameBuffer->Bind();
			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_AntiAliasedFrameBuffer->Unbind();
			m_FinalPassFrameBuffer->Read();
			m_AntiAliasedFrameBuffer->Draw();
			glBlitFramebuffer(
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			return m_AntiAliasedTexture;
		}
		return m_CombinedTexture; }
    //Return the blurred scene texture.
    GLuint FullBlurredTexture() { 
		if (m_MSAA) {
			m_AntiAliasedFrameBuffer->Bind();
			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT);
			m_AntiAliasedFrameBuffer->Unbind();
			m_FinalPassFrameBuffer->Read();
			m_AntiAliasedFrameBuffer->Draw();
			glBlitFramebuffer(
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				0, 0, m_Renderer->GetViewportSize().Width, m_Renderer->GetViewportSize().Height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			return m_AntiAliasedTexture;
		}
		return m_FullBlurredTexture; }
    //Return the framebuffer used in the scene rendering stage.
    FrameBuffer* FinalPassFrameBuffer() { return m_FinalPassFrameBuffer; }

private:
    void DrawSprites(std::list<std::shared_ptr<RenderJob>>&jobs, RenderScene& scene);
    void DrawModelRenderQueues(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
	void DrawModelRenderQueuesWithShieldCheck(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawShieldedModelRenderQueue(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);
    void DrawToDepthStencilBuffer(std::list<std::shared_ptr<RenderJob>>& jobs, RenderScene& scene);

    void BindExplosionUniforms(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job, RenderScene& scene);
    void BindModelUniforms(GLuint shaderHandle, std::shared_ptr<ModelJob>& job, RenderScene& scene);

    void BindExplosionTextures(GLuint shaderHandle, std::shared_ptr<ExplosionEffectJob>& job);
    void BindModelTextures(GLuint shaderHandle, std::shared_ptr<ModelJob>& job);

    Texture* m_WhiteTexture;
    Texture* m_BlackTexture;
    Texture* m_NeutralNormalTexture;
    Texture* m_GreyTexture;
    Texture* m_ErrorTexture;

    FrameBuffer* m_FinalPassFrameBuffer = nullptr;
	FrameBuffer* m_ShieldDepthFrameBuffer = nullptr;
	FrameBuffer* m_AntiAliasedFrameBuffer = nullptr;
    GLuint m_BloomTexture = 0;
    GLuint m_SceneTexture = 0;
    GLuint m_DepthBuffer = 0;
	GLuint m_ShieldBuffer = 0;
    GLuint m_CubeMapTexture = 0;
    GLuint m_FullBlurredTexture = 0;
    GLuint m_CombinedTexture = 0; //This can be removed for less memory usage, just set m_sceneTexture to the return from m_BlurHUDPass.CombineTextures
	GLuint m_AntiAliasedTexture = 0; //Is only used when MSAA is active;

    //maqke this component based i guess?
    GLuint m_ShieldPixelRate = 16;
	unsigned int m_MSAA = 0;

    const IRenderer* m_Renderer;
    const LightCullingPass* m_LightCullingPass;
    const CubeMapPass* m_CubeMapPass;
	const SSAOPass* m_SSAOPass;
	const ShadowPass* m_ShadowPass;

    ShaderProgram* m_ForwardPlusProgram;
    ShaderProgram* m_ExplosionEffectProgram;
	ShaderProgram* m_ExplosionEffectSplatMapProgram;
    ShaderProgram* m_SpriteProgram;
	ShaderProgram* m_ForwardPlusSplatMapProgram;
    ShaderProgram* m_FillDepthStencilBufferProgram;

	ShaderProgram* m_ForwardPlusShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSplatMapShieldCheckProgram;
	ShaderProgram* m_SpriteShieldCheckProgram;
	ShaderProgram* m_ForwardPlusSplatMapShieldCheckProgram;



	ShaderProgram* m_ForwardPlusSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSkinnedProgram;
	ShaderProgram* m_ExplosionEffectSplatMapSkinnedProgram;
	ShaderProgram* m_ForwardPlusSplatMapSkinnedProgram;
    ShaderProgram* m_FillDepthStencilBufferSkinnedProgram;

	ShaderProgram* m_ForwardPlusSkinnedShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSkinnedShieldCheckProgram;
	ShaderProgram* m_ExplosionEffectSplatMapSkinnedShieldCheckProgram;
	ShaderProgram* m_ForwardPlusSplatMapSkinnedShieldCheckProgram;
	ShaderProgram* m_FillDepthBufferSkinnedShieldCheckProgram;
};

#endif 