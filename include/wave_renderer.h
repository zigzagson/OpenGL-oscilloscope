#ifndef WAVE_RENDERER_H
#define WAVE_RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

class WaveRenderer
{
public:
    Shader WaveShader;
    WaveRenderer() {}
    WaveRenderer(const char *vertexPath, const char *fragmentPath);
    void WaveRenderInit(const char *vertexPath, const char *fragmentPath);
    void SetWaveAttribute(GLuint viewSize, float dataMin, float dataMax);
    void ResetWaveData(float *waveFormData, unsigned int size);
    void RenderWave(glm::vec2 offset, float xStep, float yScale, glm::vec3 color);

private:
    // Render state
    GLuint VAO, VBO;
    GLuint dataSize, viewSize; //横轴分viewSize格
    float dataMin, dataMax;
};

#endif