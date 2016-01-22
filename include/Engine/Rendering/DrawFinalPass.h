#ifndef DrawFinalPass_h__
#define DrawFinalPass_h__

#include "IRenderer.h"
#include "DrawFinalPassState.h"
#include "LightCullingPass.h"
#include "FrameBuffer.h"
#include "ShaderProgram.h"
#include "Util/UnorderedMapVec2.h"
#include "Texture.h"

class DrawFinalPass
{
public:
    DrawFinalPass(IRenderer* renderer, LightCullingPass* lightCullingPass);
    ~DrawFinalPass() { }
    void InitializeTextures();
    void InitializeFrameBuffers();
    void InitializeShaderPrograms();
    void Draw(RenderScene& scene);

    //Todo: Should not be public
    FrameBuffer m_BloomFrameBuffer;
    GLuint m_BloomTexture;
    GLuint m_SceneTexture;
    GLuint m_DepthBuffer;

private:
    void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type) const;
    void GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint format, GLenum type, GLint numMipMaps) const;

    Texture* m_WhiteTexture;
    Texture* m_BlackTexture;
    Texture* TEMP_glowTestTexture;

    const IRenderer* m_Renderer;
    const LightCullingPass* m_LightCullingPass;

    ShaderProgram* m_ForwardPlusProgram;
};

#endif 