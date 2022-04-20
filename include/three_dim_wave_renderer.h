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
    Shader WaveShader;
    ThreeDimWaveRenderer() {}
    ThreeDimWaveRenderer(const char *vertexPath, const char *fragmentPath);
    void WaveRenderInit(const char *vertexPath, const char *fragmentPath);
    void SetWaveAttribute(GLuint viewSize, float dataMin, float dataMax);
    void SetColorAttribute(glm::vec3 beginColor, glm::vec3 endColor, int maxGrade);
    void ResetWaveData(float *waveFormData, unsigned int size);
    void RenderWave(glm::vec2 offset, float xStep, float yScale);

private:
    // Render state
    GLuint VAO, VBO, EBO;
    GLuint dataSize, viewSize; //横轴分viewSize格
    float dataMin, dataMax;
};

#endif