#include <iostream>

#include "wave_renderer.h"

WaveRenderer::WaveRenderer(Shader shader)
{
    // Load and configure shader
    this->WaveShader = shader;
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
}
WaveRenderer::WaveRenderer(const char *vertexPath, const char *fragmentPath)
{
    // Load and configure shader
    this->WaveShader.Compile(vertexPath, fragmentPath);
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
}
void WaveRenderer::SetWaveAttribute(GLuint dataSize, GLuint viewSize, float dataMin, float dataMax)
{
    this->dataSize = dataSize;
    this->viewSize = viewSize;
    this->dataMin = dataMin;
    this->dataMax = dataMax;
}
void WaveRenderer::ResetWaveData(float *waveFormData, unsigned int size)
{
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, size, waveFormData, GL_STREAM_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}
void WaveRenderer::RenderWave(glm::vec2 offset, float xStep, float yScale, glm::vec3 color)
{
    float start = -(offset.x + xStep / 2) * this->viewSize;
    float end = start + this->viewSize * xStep;
    float range = this->dataMax - this->dataMin;
    float min = this->dataMin / yScale + offset.y * range;
    float max = this->dataMax / yScale + offset.y * range;
    glm::mat4 projection = glm::ortho(start, end, min, max);
    if (start < 0)
        start = 0;
    if (end > this->dataSize)
        end = this->dataSize;
    this->WaveShader.Use();
    glBindVertexArray(this->VAO);
    this->WaveShader.SetVector4f("setColor", color.x, color.y, color.z, 1.0f);
    this->WaveShader.SetMatrix4fv("projection", projection);
    glDrawArrays(GL_LINE_STRIP, start, end - start);
}
