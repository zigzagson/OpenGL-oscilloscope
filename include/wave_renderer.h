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
    WaveRenderer(Shader shader);
    WaveRenderer(const char *vertexPath, const char *fragmentPath);
    void SetWaveAttribute(GLuint dataSize, GLuint viewSize, float dataMin, float dataMax);
    void ResetWaveData(float *waveFormData, unsigned int size);
    void RenderWave(glm::vec2 offset, float xStep, float yScale, glm::vec3 color);

private:
    // Render state
    GLuint VAO, VBO;
    GLuint dataSize, viewSize;
    float dataMin, dataMax;
};

#endif