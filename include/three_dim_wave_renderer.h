#ifndef THREE_DIM_WAVE_RENDERER_H
#define THREE_DIM_WAVE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

class ThreeDimWaveRenderer
{
public:
    Shader SingleColorWaveShader;
    Shader MultiColorWaveShader;
    ThreeDimWaveRenderer() {}
    void WaveRenderInit();
    void SetWaveAttribute(GLuint viewSize, float dataMin, float dataMax);
    void SetBackgroundColor(glm::vec3 backgroundColor);
    void SetColorGrade(int maxColorGrade);
    void SetColorAttribute(glm::vec3 beginColor, glm::vec3 endColor);
    void SetColorAttribute(glm::vec3 beginColor, glm::vec3 secondColor, glm::vec3 thirdColor,
                           glm::vec3 fourthColor, glm::vec3 endColor);
    void ResetWaveData(float *waveFormData, unsigned int size);
    void RenderWave(glm::vec2 offset, float xStep, float yScale, bool ifMultiColor);

private:
    // Render state
    GLuint VAO, VBO, EBO;
    GLuint dataSize, viewSize; //横轴分viewSize格
    float dataMin, dataMax;
};

#endif