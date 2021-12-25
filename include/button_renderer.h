#ifndef BUTTON_RENDERER_H
#define BUTTON_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include "shader.h"
#include "texture.h"
#include "text_renderer.h"

class ButtonRender
{
public:
    Shader frameShader;
    TextRenderer buttonText;
    ButtonRender() {}
    void ButtonRenderInit(GLuint width, GLuint height);
    void SetProjection(GLuint width, GLuint height);
    void drawButton(float xStep, int xExponent, float yScale, float offset, float trigLevel);

private:
    GLuint VAO, VBO;
};
#endif