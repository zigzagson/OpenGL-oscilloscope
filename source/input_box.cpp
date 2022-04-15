#include "input_box.h"

void InputBox::InputBoxInit()
{
    this->boxShader.Compile("shader/wave.vs", "shader/wave.fs");
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &this->VBO);
    this->boxColor = glm::vec3(1.0f, 1.0f, 1.0f);
}
void InputBox::setPosition(float scrW, float scrH, float posX, float posY, float posW, float posH)
{
    this->scrWidth = scrW;
    this->scrHeight = scrH;

    float vertices[] = {
        posX, posY,               // top right
        posX, posY + posH,        // bottom right
        posX + posW, posY + posH, // bottom left
        posX + posW, posY,        // top left
        posX, posY,               // top right
        posX + posW, posY + posH, // bottom left
    };

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}
void InputBox::RenderInputBox()
{
    glm::mat4 projection = glm::ortho(0.0f, this->scrWidth, 0.0f, this->scrHeight);
    this->boxShader.Use();
    glBindVertexArray(this->VAO);
    this->boxShader.SetVector3f("setColor", this->boxColor);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    this->boxShader.SetVector3f("setColor", glm::vec3(0.2f, 0.2f, 0.2f));
    this->boxShader.SetMatrix4fv("projection", projection);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}
void InputBox::TypeInCharacter(char key)
{
    this->valueString += key;
}
float InputBox::getInputValue()
{
    return stof(this->valueString);
}
void InputBox::exitInputBox()
{
    this->valueString.clear();
    this->ifInputBoxShow = false;
}