#include <iostream>

#include "three_dim_wave_renderer.h"

unsigned int indices[511 * 1023 * 6];
ThreeDimWaveRenderer::ThreeDimWaveRenderer(const char *vertexPath, const char *fragmentPath)
{
    WaveRenderInit(vertexPath, fragmentPath);
}
void ThreeDimWaveRenderer::WaveRenderInit(const char *vertexPath, const char *fragmentPath)
{
    // Load and configure shader
    this->WaveShader.Compile(vertexPath, fragmentPath);
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &this->EBO);
    memset(indices, 0, sizeof(indices));
    for (int i = 0; i < 1023; i++)
    {
        for (int j = 0; j < 511; j++)
        {
            indices[6 * (511 * i + j)] = 512 * i + j;           //左上
            indices[6 * (511 * i + j) + 1] = 512 * i + j + 1;   //左下
            indices[6 * (511 * i + j) + 2] = 512 * i + j + 512; //右上
            indices[6 * (511 * i + j) + 3] = 512 * i + j + 1;   //左下
            indices[6 * (511 * i + j) + 4] = 512 * i + j + 512; //右上
            indices[6 * (511 * i + j) + 5] = 512 * i + j + 513; //右下
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}
void ThreeDimWaveRenderer::SetWaveAttribute(GLuint viewSize, float dataMin, float dataMax)
{
    this->viewSize = viewSize;
    this->dataMin = dataMin;
    this->dataMax = dataMax;
}
void ThreeDimWaveRenderer::SetColorAttribute(glm::vec3 beginColor, glm::vec3 endColor, int maxGrade)
{
    this->WaveShader.Use();
    glBindVertexArray(this->VAO);
    this->WaveShader.SetVector3f("beginColor", beginColor);
    this->WaveShader.SetVector3f("endColor", endColor);
    this->WaveShader.SetInteger("maxGrade", maxGrade);
}
void ThreeDimWaveRenderer::ResetWaveData(float *waveFormData, unsigned int size)
{
    this->dataSize = 1024;
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, size, waveFormData, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(0);
}
void ThreeDimWaveRenderer::RenderWave(glm::vec2 offset, float xStep, float yScale)
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
    this->WaveShader.SetMatrix4fv("projection", projection);
    // glDrawArrays(GL_POINTS, 0, 1024 * 512);
    glDrawElements(GL_TRIANGLES, 6 * 511 * 1023, GL_UNSIGNED_INT, 0);
}
