#include "Rendering/RenderState.h"

bool RenderState::Enable(GLenum cap)
{
    if (glIsEnabled(cap)) {
        return false;
    }
    m_ResetFunctions.push_back(std::bind(glDisable, cap));
    glEnable(cap);
    return !GLERROR("RenderState::Enable");
}

bool RenderState::Disable(GLenum cap)
{
    if (!glIsEnabled(cap)) {
        return false;
    }
    m_ResetFunctions.push_back(std::bind(glEnable, cap));
    glDisable(cap);
    return !GLERROR("RenderState::Disable");
}

bool RenderState::CullFace(GLenum mode)
{
    if (!glIsEnabled(GL_CULL_FACE)) {
        LOG_ERROR("Setting GL_CULL_FACE without enabling it.");
        return false;
    }
    
    GLint original;
    glGetIntegerv(GL_CULL_FACE_MODE, &original);
    m_ResetFunctions.push_back(std::bind(glCullFace, original));
    glCullFace(mode);
    return !GLERROR("RenderState::CullFace");
}

bool RenderState::ClearColor(glm::vec4 color)
{
    GLfloat original[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &original[0]);
    m_ResetFunctions.push_back(std::bind(glClearColor, original[0], original[1], original[2], original[3]));
    glClearColor(color.r, color.g, color.b, color.a);
    return !GLERROR("RenderState::ClearColor");
}

bool RenderState::Clear(GLbitfield mask)
{
    glClear(mask);
    return !GLERROR("RenderState::Clear");
}

bool RenderState::BindFramebuffer(GLint framebuffer)
{
    GLint originalRead;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &originalRead);
    GLint originalDraw;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalDraw);
    m_ResetFunctions.push_back([originalRead, originalDraw]() {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, originalRead); 
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalDraw); 
    });
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    return !GLERROR("RenderState::BindBuffer");
}


bool RenderState::BlendEquation(GLenum mode)
{
    GLint originalRGB;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &originalRGB);
    GLint originalAlpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &originalAlpha);
    m_ResetFunctions.push_back(std::bind(glBlendEquationSeparate, originalRGB, originalAlpha));
    glBlendEquation(mode);
    return !GLERROR("RenderState::BlendEquation");
}

bool RenderState::BlendFunc(GLenum sfactor, GLenum dfactor)
{
    GLint originalSrcRGB;
    glGetIntegerv(GL_BLEND_SRC_RGB, &originalSrcRGB);
    GLint originalSrcAlpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &originalSrcAlpha);
    GLint originalDestRGB;
    glGetIntegerv(GL_BLEND_DST_RGB, &originalDestRGB);
    GLint originalDestAlpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, &originalDestAlpha);
    m_ResetFunctions.push_back(std::bind(glBlendFuncSeparate, originalSrcRGB, originalSrcAlpha, originalDestRGB, originalDestAlpha));
    glBlendFunc(sfactor, dfactor);
    return !GLERROR("RenderState::BlendFunc");
}

RenderState::~RenderState()
{
    for (auto& f : m_ResetFunctions) {
        f();
    }
}
