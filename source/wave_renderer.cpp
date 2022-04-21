#include <iostream>

#include "wave_renderer.h"

WaveRenderer::WaveRenderer(const char *vertexPath, const char *fragmentPath)
{
    WaveRenderInit(vertexPath, fragmentPath);
}
void WaveRenderer::WaveRenderInit(const char *vertexPath, const char *fragmentPath)
{
    // Load and configure shader
    this->WaveShader.Compile(vertexPath, fragmentPath);
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
}
void WaveRenderer::SetWaveAttribute(GLuint viewSize, float dataMin, float dataMax)
{
    this->viewSize = viewSize;
    this->dataMin = dataMin;
    this->dataMax = dataMax;
}
void WaveRenderer::ResetWaveData(float *waveFormData, unsigned int size)
{
    this->dataSize = size;
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, size * 2 * sizeof(float), waveFormData, GL_STREAM_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}
void WaveRenderer::RenderWave(glm::vec2 offset, float xStep, float yScale, glm::vec3 color)
{
    float start = -(offset.x + xStep / 2) * this->viewSize + this->dataSize / 2; //缩放时中间不动，初始展示波形中间数据
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
    this->WaveShader.SetVector3f("setColor", color);
    this->WaveShader.SetMatrix4fv("projection", projection);
    glDrawArrays(GL_LINE_STRIP, start, end - start);
}
