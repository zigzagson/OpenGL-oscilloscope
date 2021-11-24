#ifndef DRAW_BACKGROUND_H
#define DRAW_BACKGROUND_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include "shader.h"
#include "texture.h"
#include "text_renderer.h"

class BackgroundRender
{
public:
    Shader dashedShader;
    TextRenderer valueText;
    Texture2D iconTexture;
    BackgroundRender() {}
    void BackgroundRenderInit(float scrWidth, float scrHeight);
    void drawBackground(float xStep, int xExponent, float yScale, float offset);
    void setSize(float scrWidth, float scrHeight, float viewportX, float viewportY, float viewportW, float viewportH);

private:
    void BackgroundBorderInit();
    void BackgroundGriddingInit();
    GLuint borderVAO, borderVBO, borderEBO;
    GLuint gridVAO, gridVBO, gridEBO;
    float scrWidth, scrHeight;
    float viewportX, viewportY, viewportW, viewportH;
    unsigned int fontSize;
};
#endif