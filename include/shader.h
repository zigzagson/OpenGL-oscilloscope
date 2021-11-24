#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader() {}
    Shader(const char *vertexPath, const char *fragmentPath);
    void Compile(const char *vertexPath, const char *fragmentPath);
        // activate the shader
        // ------------------------------------------------------------------------
        void Use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetBool(const std::string &name, bool value);
    void SetInteger(const std::string &name, int value);
    void SetFloat(const std::string &name, float value);
    void SetVector3u(const std::string &name, unsigned int x, unsigned int y, unsigned int z);
    void SetVector3f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z);
    void SetVector3f(const GLchar *name, const glm::vec3 &value);
    void SetVector4f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void SetVector4f(const GLchar *name, const glm::vec4 &value);
    void SetMatrix4fv(const GLchar *name, const glm::mat4 &matrix);

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type, const char *filePath);
};
#endif