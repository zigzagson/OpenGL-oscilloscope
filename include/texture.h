#ifndef TEXTURE_H
#define TEXTURE_H

#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "shader.h"

// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D
{
public:
    Shader TextureShader;
    // Texture Format
    int Width, Height;      // Width and height of loaded image in pixels
    GLuint Internal_Format; // Format of texture object
    GLuint Image_Format;    // Format of loaded image
    // Texture configuration
    GLuint Wrap_S;     // Wrapping mode on S axis
    GLuint Wrap_T;     // Wrapping mode on T axis
    GLuint Filter_Min; // Filtering mode if texture pixels < screen pixels
    GLuint Filter_Max; // Filtering mode if texture pixels > screen pixels
    glm::vec4 color;
    // Constructor (sets default texture modes)
    Texture2D();
    Texture2D(const char *vertexPath, const char *fragmentPath);
    void TextureInit(const char *vertexPath, const char *fragmentPath);
    // Generates texture from image data
    void Generate(const char *texturePath, GLuint format);
    void SetProjection(GLuint width, GLuint height);
    void SetPosition(float posX, float posY, float posW, float posH);
    void DrawTexture(float posX, float posY, float posW, float posH, bool ifBlend);
    void DrawTexture(bool ifBlend);

private:
    // Holds the ID of the texture object, used for all texture operations to reference to this particlar texture
    GLuint ID;
    GLuint VAO, VBO;
};

#endif