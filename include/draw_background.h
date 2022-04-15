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
#include "input_box.h"

typedef struct
{
    std::string name;
    float value;
    std::string unit;
} Parameter;

class BackgroundRender
{
public:
    Shader dashedShader;
    TextRenderer valueText;
    Texture2D iconTexture;
    InputBox volBox, timeBox;
    BackgroundRender() {}
    void BackgroundRenderInit(float scrWidth, float scrHeight);
    void drawBackground(float xStep, float yScale, float offset, float trigLevel);
    void setSize(float scrWidth, float scrHeight, float viewportX, float viewportY, float viewportW, float viewportH);
    bool ifPause;
    bool ifTrig;
    bool ifOverSampling;
    bool ifTrigAverage;
    glm::vec3 borderColor;
    glm::vec3 gridColor;
    glm::vec3 trigLineColor;
    glm::vec3 textColor;
    Parameter measuredValue[6];
    std::string valueFontPath;

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