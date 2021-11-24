#include "draw_background.h"

void BackgroundRender::BackgroundRenderInit(float scrWidth, float scrHeight)
{
    this->dashedShader.Compile("shader/dashed.vs", "shader/dashed.fs");
    this->valueText.TextRendererInit("shader/text.vs", "shader/text.fs", scrWidth, scrHeight);
    this->iconTexture.TextureInit("shader/texture.vs", "shader/texture.fs");
    this->iconTexture.Generate("img/uestc_icon.png", GL_RGBA);
    this->iconTexture.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    this->fontSize = (GLuint)(scrWidth / 240) * 4;
    this->valueText.Load("fonts/arial.ttf", this->fontSize);
    BackgroundBorderInit();
    BackgroundGriddingInit();
}

void BackgroundRender::BackgroundBorderInit()
{
    float vertices[16] = {
        1.0f, 0.9999f, 1.0f, -1.0f, -0.9999f, -1.0f, -0.9999f, 0.9999f,
        1.0f, 0.0f, 0.0f, -1.0f, -0.9999f, 0.0f, 0.0f, 0.9999f};
    unsigned int indices[12] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 6, 5, 7};
    glGenVertexArrays(1, &this->borderVAO);
    glGenBuffers(1, &this->borderVBO);
    glGenBuffers(1, &this->borderEBO);

    glBindVertexArray(this->borderVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->borderVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->borderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void BackgroundRender::BackgroundGriddingInit()
{
    int i = 0, j = 0;
    float vertices[472] = {0};
    for (i = 0; i < 11; i++)
    {
        vertices[i * 4] = -1.0f;
        vertices[i * 4 + 1] = 1.0f - 0.2f * i;
        vertices[i * 4 + 2] = 1.0f;
        vertices[i * 4 + 3] = 1.0f - 0.2f * i;
    }
    for (i = 0; i < 12; i++)
    {
        for (j = 0; j < 4; j++)
        {
            vertices[i * 16 + j * 4 + 44] = -1.0f;
            vertices[i * 16 + j * 4 + 45] = 1.16f - 0.2f * i - 0.04f * j;
            vertices[i * 16 + j * 4 + 46] = 1.0f;
            vertices[i * 16 + j * 4 + 47] = 1.16f - 0.2f * i - 0.04f * j;
        }
    }
    for (i = 0; i < 11; i++)
    {
        vertices[i * 4 + 236] = 1.0f - 1.0f / 6 * (i + 1);
        vertices[i * 4 + 237] = 1.2f;
        vertices[i * 4 + 238] = 1.0f - 1.0f / 6 * (i + 1);
        vertices[i * 4 + 239] = -1.2f;
    }
    for (i = 0; i < 12; i++)
    {
        for (j = 0; j < 4; j++)
        {
            vertices[i * 16 + j * 4 + 280] = 1.0f - 1.0f / 6 * i - 1.0f / 30 * (j + 1);
            vertices[i * 16 + j * 4 + 281] = 1.2f;
            vertices[i * 16 + j * 4 + 282] = 1.0f - 1.0f / 6 * i - 1.0f / 30 * (j + 1);
            vertices[i * 16 + j * 4 + 283] = -1.2f;
        }
    }
    unsigned int indices[236] = {0};
    for (i = 0; i < 236; i++)
    {
        indices[i] = i;
    }
    glGenVertexArrays(1, &this->gridVAO);
    glGenBuffers(1, &this->gridVBO);
    glGenBuffers(1, &this->gridEBO);

    glBindVertexArray(this->gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->gridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->gridEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void BackgroundRender::setSize(float scrWidth, float scrHeight, float viewportX, float viewportY, float viewportW, float viewportH)
{
    this->scrWidth = scrWidth;
    this->scrHeight = scrHeight;
    this->viewportX = viewportX;
    this->viewportY = viewportY;
    this->viewportW = viewportW;
    this->viewportH = viewportH;
    this->valueText.SetProjection(scrWidth, scrHeight);
    this->fontSize = (GLuint)(scrWidth / 240) * 4;
    this->valueText.Load("fonts/arial.ttf", this->fontSize);
    this->iconTexture.SetProjection(scrWidth, scrHeight);
    this->iconTexture.SetPosition(scrWidth * 0.84f, scrHeight - scrWidth * 0.16f, scrWidth * 0.12f, scrWidth * 0.12f);
}
void BackgroundRender::drawBackground(float xStep, int xExponent, float yScale, float offset)
{
    float xZoom = 10 / xStep;
    if (xZoom >= 1.0 && xZoom < 2.0)
    {
        xZoom = xZoom;
        xStep = 10;
    }
    else if (xZoom >= 2.0 && xZoom < 4.0)
    {
        xZoom = xZoom / 2.0;
        xStep = 5;
    }
    else if (xZoom >= 4.0 && xZoom < 5.0)
    {
        xZoom = xZoom * 3.0 / 10.0;
        xStep = 3;
    }
    else if (xZoom >= 5.0 && xZoom < 9.9999)
    {
        xZoom = xZoom / 5.0;
        xStep = 2;
    }
    else
    {
        xZoom = xZoom / 10.0;
        xStep = 1;
    }
    int yExponent;
    float yZoom, yStep;
    yZoom = yScale * 2;
    if (yZoom >= 1.0 && yZoom < 10.0)
    {
        yExponent = 3;
    }
    else if (yZoom >= 10.0 && yZoom < 100.0)
    {
        yExponent = 2;
        yZoom /= 10.0;
    }
    else if (yZoom >= 100.0 && yZoom < 1000.0)
    {
        yExponent = 1;
        yZoom /= 100.0;
    }
    else
    {
        yExponent = 0;
        yZoom /= 1000.0;
    }
    if (yZoom >= 1.0 && yZoom < (5.0 / 3.0))
    {
        yZoom = yZoom;
        yStep = 2;
    }
    else if (yZoom >= (5.0 / 3.0) && yZoom < (10.0 / 3.0))
    {
        yZoom = yZoom / 2.0;
        yStep = 1;
    }
    else if (yZoom >= (10.0 / 3.0) && yZoom < (50.0 / 6.0))
    {
        yZoom = yZoom / 4.0;
        yStep = 0.5;
    }
    else
    {
        yZoom = yZoom / 10.0;
        yStep = 0.2;
    }
    glViewport(0, 0, this->scrWidth, this->scrHeight);

    this->iconTexture.DrawTexture(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < 11; i++) //画刻度值
    {
        int value = (int)(((int)(offset * 10000 / pow(10, yExponent) / yStep) + i - 5) * yStep * 10) * pow(10, yExponent - 1); //因为浮点数精度多了点麻烦
        int x = this->viewportX - this->fontSize * 1.25;
        int y = this->viewportY + this->viewportH / 2 + ((float)i - 5.0 - fmod(offset * yScale / yZoom, 0.1) * 10.0) * this->viewportH * 0.1 * yZoom - this->fontSize / 2;
        this->valueText.RenderTextAlignRight(std::to_string(value), x, y, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }
    std::string parameter = "TIME/DIV: ";
    if ((xExponent == 2 || xExponent == 5) && xStep == 10)
    {
        xExponent++;
        xStep = 1;
    }
    parameter += std::to_string((int)(xStep * pow(10, xExponent % 3)));
    if (xExponent < 3)
        parameter += "us";
    else if (xExponent >= 3 && xExponent < 6)
        parameter += "ms";
    else
        parameter += "s";
    this->valueText.RenderText(parameter, this->viewportX * 2, this->viewportY * 0.5 - this->fontSize / 2, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    glDisable(GL_BLEND);
    glViewport(this->viewportX, this->viewportY, this->viewportW, this->viewportH);
    glm::mat4 transform;
    this->dashedShader.Use();
    dashedShader.SetMatrix4fv("transform", transform);
    glBindVertexArray(this->borderVAO);
    this->dashedShader.SetVector3u("type", 0U, 0U, 1U);
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, 0); //边框
    transform = glm::scale(transform, glm::vec3(1.0f, yScale, 1.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -offset * 2, 0.0f));
    dashedShader.SetMatrix4fv("transform", transform);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void *)(8 * sizeof(unsigned int))); //0mv线
    glBindVertexArray(this->gridVAO);
    offset = offset * yScale / yZoom;
    offset = fmod(offset, 0.2);
    transform = glm::mat4(1.0f);
    transform = glm::scale(transform, glm::vec3(xZoom, yZoom, 1.0f));
    transform = glm::translate(transform, glm::vec3(0.0f, -offset * 2, 0.0f));
    dashedShader.SetMatrix4fv("transform", transform);
    this->dashedShader.SetVector3u("type", 0U, 0U, 0U);
    glDrawElements(GL_LINES, 22, GL_UNSIGNED_INT, (void *)(0 * sizeof(unsigned int))); //横虚线
    this->dashedShader.SetVector3u("type", 0U, 1U, 0U);
    glDrawElements(GL_LINES, 96, GL_UNSIGNED_INT, (void *)(22 * sizeof(unsigned int))); //竖刻度
    this->dashedShader.SetVector3u("type", 1U, 0U, 0U);
    glDrawElements(GL_LINES, 22, GL_UNSIGNED_INT, (void *)(118 * sizeof(unsigned int))); //竖虚线
    this->dashedShader.SetVector3u("type", 1U, 1U, 0U);
    glDrawElements(GL_LINES, 96, GL_UNSIGNED_INT, (void *)(140 * sizeof(unsigned int))); //横刻度
}