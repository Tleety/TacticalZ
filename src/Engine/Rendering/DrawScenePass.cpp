#include "Rendering/DrawScenePass.h"

DrawScenePass::DrawScenePass(IRenderer* renderer)
{
    m_Renderer = renderer;
    InitializeTextures();
    InitializeShaderPrograms();
}

void DrawScenePass::InitializeTextures()
{
    m_WhiteTexture = ResourceManager::Load<Texture>("Textures/Core/Blank.png");
}

void DrawScenePass::InitializeShaderPrograms()
{
    m_BasicForwardProgram = ResourceManager::Load<ShaderProgram>("#BasicForwardProgram");

    m_BasicForwardProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/BasicForward.vert.glsl")));
    m_BasicForwardProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/BasicForward.frag.glsl")));
    m_BasicForwardProgram->Compile();
    m_BasicForwardProgram->Link();
}

void DrawScenePass::Draw(RenderScene& scene)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLERROR("DrawScenePass::Draw: Pre");

    DrawScenePassState state = DrawScenePassState();
    m_BasicForwardProgram->Bind();

    for (auto &job : scene.ForwardJobs) {
        auto modelJob = std::dynamic_pointer_cast<ModelJob>(job);
        if (modelJob) {
            GLuint ShaderHandle = m_BasicForwardProgram->GetHandle();

            //TODO: Kolla upp "header/include/common" shader saken s� man slipper skicka in asmycket uniforms
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "M"), 1, GL_FALSE, glm::value_ptr(modelJob->Matrix));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "V"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(ShaderHandle, "P"), 1, GL_FALSE, glm::value_ptr(scene.Camera->ProjectionMatrix()));            
            glUniform4fv(glGetUniformLocation(ShaderHandle, "Color"), 1, glm::value_ptr(modelJob->Color));

            //TODO: Renderer: b�ttre textur felhantering samt fler texturer st�d
            if (modelJob->DiffuseTexture != nullptr) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, modelJob->DiffuseTexture->m_Texture);
            } else {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_WhiteTexture->m_Texture);
            }

            glBindVertexArray(modelJob->Model->VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelJob->Model->ElementBuffer);
            glDrawElementsBaseVertex(GL_TRIANGLES, modelJob->EndIndex - modelJob->StartIndex + 1, GL_UNSIGNED_INT, 0, modelJob->StartIndex);

            //continue;
        }
    
    }
    GLERROR("DrawScenePass::Draw: End");
}
