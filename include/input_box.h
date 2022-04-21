#ifndef INPUT_BOX_H
#define INPUT_BOX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include "shader.h"
#include "text_renderer.h"

class InputBox
{
public:
    Shader boxShader;
    // Constructor
    InputBox(){};
    void InputBoxInit();
    void setPosition(float scrW, float scrH, float posX, float posY, float posW, float posH);
    void RenderInputBox();
    void TypeIn(int key);
    float getInputValue();
    void exitInputBox();

    std::string valueString;
    glm::vec3 boxColor;
    bool ifInputBoxShow;

private:
    GLuint VAO, VBO;
    float scrWidth, scrHeight;
};

#endif