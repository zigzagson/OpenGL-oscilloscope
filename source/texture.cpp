#include "texture.h"

Texture2D::Texture2D()
    : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR), color(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
{
}
Texture2D::Texture2D(const char *vertexPath, const char *fragmentPath)
    : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB), Wrap_S(GL_REPEAT), Wrap_T(GL_REPEAT), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR), color(glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
{
    TextureInit(vertexPath, fragmentPath);
}
void Texture2D::TextureInit(const char *vertexPath, const char *fragmentPath)
{
    this->TextureShader.Compile(vertexPath, fragmentPath);

    glGenTextures(1, &this->ID);
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Texture2D::Generate(const char *texturePath, GLuint format)
{
    this->Internal_Format = format;
    this->Image_Format = format;
    int nrChannels;
    stbi_set_flip_vertically_on_load(true); //OpenGL要求y轴0.0坐标在图片的底部，但图片y轴0.0坐标通常在顶部。stb_image能在图像加载时翻转y轴
    unsigned char *data = stbi_load(texturePath, &this->Width, &this->Height, &nrChannels, 0);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //OpenGL要求所有的纹理都是4字节对齐的,通过将纹理解压对齐参数设为1，这样才能确保不会有对齐问题。
    // Create Texture
    glBindTexture(GL_TEXTURE_2D, this->ID);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, this->Width, this->Height, 0, this->Image_Format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture2D::SetProjection(GLuint width, GLuint height)
{
    glUseProgram(this->TextureShader.ID);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
    this->TextureShader.SetMatrix4fv("projection", projection);
}
void Texture2D::SetPosition(float posX, float posY, float posW, float posH)
{
    float vertices[6][4] = {
        {posX, posY, 0.0, 0.0},
        {posX, posY + posH, 0.0, 1.0},
        {posX + posW, posY + posH, 1.0, 1.0},

        {posX, posY, 0.0, 0.0},
        {posX + posW, posY, 1.0, 0.0},
        {posX + posW, posY + posH, 1.0, 1.0}};
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Texture2D::DrawTexture(float posX, float posY, float posW, float posH, bool ifBlend)
{
    if (ifBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    SetPosition(posX, posY, posW, posH);
    this->TextureShader.Use();
    this->TextureShader.SetVector4f("TexColor", this->color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (ifBlend)
        glDisable(GL_BLEND);
}
void Texture2D::DrawTexture(bool ifBlend)
{
    if (ifBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    this->TextureShader.Use();
    this->TextureShader.SetVector4f("TexColor", this->color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(this->VAO);
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (ifBlend)
        glDisable(GL_BLEND);
}
