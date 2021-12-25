#include "button_renderer.h"

void ButtonRender::ButtonRenderInit(GLuint width, GLuint height)
{
    // Load and configure shader
    this->frameShader.Compile("shader/text.vs", "shader/text.vs");
    this->buttonText.TextRendererInit("shader/text.vs", "shader/text.fs", width, width);
    glUseProgram(this->frameShader.ID); //使用glUniform**相关函数前要先用这个函数，不然会报1282:GL_INVALID_OPERATION错误。在主程序里不用，在这里要，我也不知道为什么。
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void ButtonRender::SetProjection(GLuint width, GLuint height)
{
    glUseProgram(this->buttonText.TextShader.ID);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
    this->buttonText.TextShader.SetMatrix4fv("projection", projection);
}