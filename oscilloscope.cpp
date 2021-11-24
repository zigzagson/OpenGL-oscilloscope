#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "udp_upper.h"
#include "wave_renderer.h"
#include "draw_background.h"
#include <iostream>
#include <windows.h>
#include <cmath>

#define DEBUG 1

using namespace std;

void windowInit();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_press_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void waveAutoSet();
UINT Receive_Data(LPVOID lpVoid);

#define DATA_SIZE 8192
#define VIEWADLE_DATA_SIZE 600
float scrWidth = 1200;
float scrHeight = 750;
float viewWidth = scrWidth * 0.70;
float viewHeight = scrHeight * 0.92;

float scaleState = 1;
glm::vec2 offsetState(-0.5f, 0.0f);
glm::vec2 offset(0.0f, 0.0f);
float timeStep = 1;
int timeExponent = 0;
bool wavePause = false;

GLFWwindow *window;
BackgroundRender background;
float waveFormData[DATA_SIZE * 2];

int main()
{
    windowInit();
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Receive_Data, nullptr, 0, NULL);
    //OpenGL上下文属于单个线程，在其他线程没有上下文时不可以使用OpenGL函数
    WaveRenderer wave("shader/wave.vs", "shader/wave.fs");
    wave.SetWaveAttribute(DATA_SIZE, VIEWADLE_DATA_SIZE, -5000, 5000);
    background.BackgroundRenderInit(scrWidth, scrHeight);
    background.setSize(scrWidth, scrHeight, scrWidth * 0.1, scrHeight * 0.06, viewWidth, viewHeight);
    while (!glfwWindowShouldClose(window))
    {
        if (!wavePause)
            wave.ResetWaveData(waveFormData, sizeof(waveFormData));
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        background.drawBackground(timeStep, timeExponent, scaleState, offsetState.y + offset.y);  //画网格
        wave.RenderWave(offsetState + offset, timeStep, scaleState, glm::vec3(1.0f, 1.0f, 0.0f)); //画波形
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void windowInit()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(scrWidth, scrHeight, "oscilloscope", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_press_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    scrWidth = width;
    scrHeight = height;
    viewWidth = scrWidth * 0.70;
    viewHeight = scrHeight * 0.92;
    background.setSize(scrWidth, scrHeight, scrWidth * 0.1, scrHeight * 0.06, viewWidth, viewHeight);
}
void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    double posX, posY;
    glfwGetCursorPos(window, &posX, &posY);
    if (posX < scrWidth * 0.8)
    {
        scaleState *= pow(10, yoffset / 10);
        if (scaleState > 1000.01 || scaleState < 0.5)
            scaleState *= pow(10, -yoffset / 10);
    }
    else
    {
        timeStep *= pow(10, -yoffset / 10);
        if (timeStep < 1)
        {
            if (timeExponent > 0)
            {
                timeExponent--;
                timeStep *= 10;
            }
            else
                timeStep *= pow(10, yoffset / 10);
        }
        if (timeStep >= 10)
        {
            if (timeExponent < 7)
            {
                timeExponent++;
                timeStep /= 10;
            }
            else
                timeStep *= pow(10, yoffset / 10);
        }
    }
}
void mouse_press_callback(GLFWwindow *window, int button, int action, int mods)
{
    static float duration;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        duration = glfwGetTime();
        background.iconTexture.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        duration = glfwGetTime() - duration;
        if (duration < 0.6f)
            wavePause = !wavePause;
        else
            waveAutoSet();
        background.iconTexture.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        double posX, posY;
        glfwGetCursorPos(window, &posX, &posY);
        float Rsquare = (posX - scrWidth * 0.9) * (posX - scrWidth * 0.9) + (posY - scrWidth * 0.1) * (posY - scrWidth * 0.1);
        if (Rsquare < scrWidth * 0.06 * scrWidth * 0.06)
        {
            if (action == GLFW_PRESS)
            {
                background.iconTexture.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
                waveAutoSet();
            }
        }
    }
    if (action == GLFW_RELEASE)
    {
        background.iconTexture.color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    }
}
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    static float firstX, firstY;
    static bool firstPress = true;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        offsetState += offset;
        offset.x = 0;
        offset.y = 0;
        firstPress = true;
        return;
    }
    if (firstPress)
    {
        firstX = xpos;
        firstY = ypos;
        firstPress = false;
    }
    offset.x = (xpos - firstX) / viewWidth * timeStep;
    offset.y = (ypos - firstY) / viewHeight / scaleState;
}

void waveAutoSet()
{
    float min, max;
    min = waveFormData[1];
    max = waveFormData[1];
    for (int i = 1; i < DATA_SIZE; i++)
    {
        if (waveFormData[2 * i + 1] > max)
            max = waveFormData[2 * i + 1];
        if (waveFormData[2 * i + 1] < min)
            min = waveFormData[2 * i + 1];
    }
    if (max - min == 0)
        scaleState = 1000.0;
    else
        scaleState = 6000.0f / (max - min);
    if (scaleState < 0.5f)
        scaleState = 0.5f;
    if (scaleState > 1000.01f)
        scaleState = 1000.0f;
    offsetState.y = (max + min) / 2 / 10000.0f;
}
#if DEBUG
UINT Receive_Data(LPVOID lpVoid)
{
    while (TRUE)
    {
        if (wavePause)
            continue;
        float timeValue = glfwGetTime();
        for (unsigned i = 0; i < DATA_SIZE; i++)
        {
            waveFormData[2 * i] = (float)i;
            waveFormData[2 * i + 1] = 300 * sin((float)i / 50.0f + timeValue) + 200;
        }
        Sleep(500);
    }
    return 0;
}
#else
UINT Receive_Data(LPVOID lpVoid)
{
    UDPconnector FPGA;
    // 连接设置
    if (FALSE == FPGA.ConnectionInit())
    {
        printf("未设置成功\n");
        return 1;
    }
    printf("等待下位机连接...\n");
    while (FALSE == FPGA.EstablishConnection())
    {
    }
    system("cls");
    printf("下位机连接成功！\n");
    cout << "FPGA的IP地址：\t" << FPGA.lowerIP() << endl;
    cout << "FPGA的MAC地址：\t" << FPGA.lowerMAC() << endl;
    static short dataBuf[DATA_SIZE];
    while (TRUE)
    {
        if (wavePause)
            continue;
        FPGA.SendMsg(timeExponent);
        for (int i = 0; i < DATA_SIZE / 1024; i++)
        {
            FPGA.ReceiveData((char *)(dataBuf + i * 1024), 1024);
            FPGA.ReceiveData((char *)(dataBuf + i * 1024 + 512), 1024);
        }
        for (int i = 0; i < DATA_SIZE; i++)
        {
            waveFormData[2 * i] = (float)i;
            waveFormData[2 * i + 1] = (float)dataBuf[i] * 5000.0f / 2048.0f;
        }
        Sleep(500);
    }
    return 0;
}
#endif