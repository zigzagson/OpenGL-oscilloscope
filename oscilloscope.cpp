#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <json/json.h>

#include "udp_upper.h"
#include "wave_renderer.h"
#include "draw_background.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>

#define DEBUG 1

using namespace std;

void windowInit();
void configurationInit();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_press_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void waveParameter();
void waveAutoSet();
void dataTrig();
void dataTrig_without_oversampling();
UINT Receive_Data(LPVOID lpVoid);
void threadRestart();

#define DATA_SIZE 262144
#define VIEW_DATA_SIZE 8192
#define VIEWADLE_DATA_SIZE 600
float scrWidth = 1200;
float scrHeight = 750;
float viewWidth = scrWidth * 0.70;
float viewHeight = scrHeight * 0.92;

float scaleState = 1;
glm::vec2 offsetState(-0.5f, 0.0f);
glm::vec2 offset(0.0f, 0.0f);
float trigLevel = 0;
float trigLevelState = 100;
float timeStep = 1;
int timeExponent = 0;
bool overSampling = false;
bool wavePause = false;
bool waveTrig = false;
int autoPeriodNum = 3;
int autoVoltageNum = 6;
float scrollSensitivity = 1;

unsigned samplingRate = 50; //单位MHz

HANDLE threadHandle;
unsigned threadCount = 0;
unsigned threadNum = 0;
GLFWwindow *window;
BackgroundRender background;
float waveFormData[VIEW_DATA_SIZE * 2];
float waveData[DATA_SIZE * 2];

glm::vec3 backgroundColor(0.1f, 0.1f, 0.1f);
glm::vec3 waveColor(1.0f, 1.0f, 0.0f);
glm::vec4 iconColor(0.8f, 0.8f, 0.8f, 1.0f);
glm::vec4 iconClickColor(1.0f, 1.0f, 1.0f, 0.0f);

#if DEBUG
int debugWaveRange = 600;
int debugWaveOffset = 200;
float debugWavePeriod = 6;
#endif

int main()
{
    windowInit();
    threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Receive_Data, nullptr, 0, NULL);
    //OpenGL上下文属于单个线程，在其他线程没有上下文时不可以使用OpenGL函数
    configurationInit();
    WaveRenderer wave("shader/wave.vs", "shader/wave.fs");
    wave.SetWaveAttribute(VIEW_DATA_SIZE, VIEWADLE_DATA_SIZE, -5000, 5000);
    while (!glfwWindowShouldClose(window))
    {
#if DEBUG
#else
        threadRestart();
#endif
        if (!wavePause)
            wave.ResetWaveData(waveFormData, sizeof(waveFormData));
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        background.drawBackground(timeStep, timeExponent, scaleState, offsetState.y + offset.y, trigLevel + trigLevelState); //画网格
        wave.RenderWave(offsetState + offset, timeStep, scaleState, waveColor);                                              //画波形
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
    background.BackgroundRenderInit(scrWidth, scrHeight);
    background.setSize(scrWidth, scrHeight, scrWidth * 0.1, scrHeight * 0.06, viewWidth, viewHeight);
}
void configurationInit()
{
    std::ifstream ifs;
    ifs.open("init.json", std::ios::in);
    if (false == ifs.is_open())
    {
        ifs.close();
        std::ofstream ofs;
        ofs.open("init.json", std::ios::out);
        Json::StreamWriterBuilder writerBuilder;
        const std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
        Json::Value root;
        Json::Value color;
        for (int i = 0; i < 3; i++)
        {
            color["background"].append(0.1f);
            color["bordor"].append(0.9f);
            color["grid"].append(0.6f);
            color["text"].append(0.9f);
            color["icon"].append(0.8f);
            color["iconClick"].append(1.0f);
        }
        color["icon"].append(1.0f);
        color["iconClick"].append(0.0f);
        color["wave"].append({1.0f});
        color["wave"].append(1.0f);
        color["wave"].append(0.0f);
        color["trig"].append(0.9f);
        color["trig"].append(0.3f);
        color["trig"].append(0.1f);
        root["color"] = color;
        root["rate"] = samplingRate;
        root["auto"].append(autoVoltageNum);
        root["auto"].append(autoPeriodNum);
        root["scroll"] = scrollSensitivity;
#if DEBUG
        root["debug"].append(debugWaveRange);
        root["debug"].append(debugWaveOffset);
        root["debug"].append(debugWavePeriod);
#endif
        writer->write(root, &ofs);
        ofs.close();
        return;
    }
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    JSONCPP_STRING errs;
    if (!parseFromStream(readerBuilder, ifs, &root, &errs))
    {
        std::cout << errs << std::endl;
        system("pause");
        return;
    }
#if DEBUG
    debugWaveRange = root["debug"][0].asInt();
    debugWaveOffset = root["debug"][1].asInt();
    debugWavePeriod = root["debug"][2].asFloat();
#endif
    samplingRate = root["rate"].asUInt();
    scrollSensitivity = root["scroll"].asFloat();
    autoVoltageNum = root["auto"][0].asInt();
    autoPeriodNum = root["auto"][1].asInt();
    Json::Value color = root["color"];
    backgroundColor = glm::vec3(color["background"][0].asFloat(), color["background"][1].asFloat(), color["background"][2].asFloat());
    waveColor = glm::vec3(color["wave"][0].asFloat(), color["wave"][1].asFloat(), color["wave"][2].asFloat());
    iconColor = glm::vec4(color["icon"][0].asFloat(), color["icon"][1].asFloat(), color["icon"][2].asFloat(), color["icon"][3].asFloat());
    iconClickColor = glm::vec4(color["iconClick"][0].asFloat(), color["iconClick"][1].asFloat(), color["iconClick"][2].asFloat(), color["iconClick"][3].asFloat());
    background.borderColor = glm::vec3(color["bordor"][0].asFloat(), color["bordor"][1].asFloat(), color["bordor"][2].asFloat());
    background.gridColor = glm::vec3(color["grid"][0].asFloat(), color["grid"][1].asFloat(), color["grid"][2].asFloat());
    background.textColor = glm::vec3(color["text"][0].asFloat(), color["text"][1].asFloat(), color["text"][2].asFloat());
    background.trigLineColor = glm::vec3(color["trig"][0].asFloat(), color["trig"][1].asFloat(), color["trig"][2].asFloat());
    background.iconTexture.color = iconColor;
    background.samplingRate = samplingRate;
    ifs.close();
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
    if (posY < scrHeight * 0.9)
    {
        scaleState *= pow(10, scrollSensitivity * yoffset / 10);
        if (scaleState > 1000.01 || scaleState < 0.5)
            scaleState *= pow(10, -scrollSensitivity * yoffset / 10);
    }
    if (posX < scrWidth * 0.8)
    {
        timeStep *= pow(10, -scrollSensitivity * yoffset / 10);
        if (timeStep < 1)
        {
            if (timeExponent > 0)
            {
                timeExponent--;
                timeStep *= 10;
            }
            else
                timeStep *= pow(10, scrollSensitivity * yoffset / 10);
        }
        if (timeStep >= 10)
        {
            if (timeExponent < 7)
            {
                timeExponent++;
                timeStep /= 10;
            }
            else
                timeStep *= pow(10, scrollSensitivity * yoffset / 10);
        }
    }
}
void mouse_press_callback(GLFWwindow *window, int button, int action, int mods)
{
    static float duration;
    double posX, posY;
    glfwGetCursorPos(window, &posX, &posY);
    float Rsquare = (posX - scrWidth * 0.9) * (posX - scrWidth * 0.9) + (posY - scrWidth * 0.1) * (posY - scrWidth * 0.1);
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        duration = glfwGetTime();
        background.iconTexture.color = iconClickColor;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        duration = glfwGetTime() - duration;
        if (duration < 0.6f)
        {
            if (Rsquare < scrWidth * 0.06 * scrWidth * 0.06)
                waveTrig = !waveTrig; //右键点击图标开启触发
            else
                wavePause = !wavePause; //右键点击图标外暂停
            if (waveTrig)
                wavePause = false;
            background.ifPause = wavePause;
            background.ifTrig = waveTrig;
        }
        else
            waveAutoSet(); //右键长按autoset
        background.iconTexture.color = iconColor;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (posX > scrWidth * 0.82 && posX < scrWidth * 0.88 && posY > scrHeight * 0.32 && posY < scrHeight * 0.35)
        {
            overSampling = !overSampling;
            background.ifOverSampling = overSampling;
        }
        if (Rsquare < scrWidth * 0.06 * scrWidth * 0.06)
        {
            background.iconTexture.color = iconClickColor;
            waveAutoSet(); //左键点图标autoset
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        configurationInit();
    }
    if (action == GLFW_RELEASE)
    {
        background.iconTexture.color = iconColor;
    }
}
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    static float firstX, firstY;
    static bool firstPress = true;
    static int type;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) //左键拖动
    {
        offsetState += offset;
        if (waveTrig)
            offsetState.x = -0.5 * timeStep;
        trigLevelState += trigLevel;
        offset.x = 0;
        offset.y = 0;
        trigLevel = 0;
        firstPress = true;
        return;
    }
    if (firstPress)
    {
        firstX = xpos;
        firstY = ypos;
        if (xpos < scrWidth * 0.8)
            type = 0;
        else
            type = 1;
        firstPress = false;
    }
    if (type == 0)
    {
        if (waveTrig == 0)
            offset.x = (xpos - firstX) / viewWidth * timeStep;
        offset.y = (ypos - firstY) / viewHeight / scaleState;
    }
    if (type && waveTrig)
        trigLevel = -(ypos - firstY) * 10000 / viewHeight / scaleState;
}

void waveParameter()
{
    float min, max, range, average;
    min = waveFormData[1];
    max = waveFormData[1];
    if (!(min == min && max == max))
    {
        min = 0;
        max = 0;
    }
    for (int i = 1; i < VIEW_DATA_SIZE; i++)
    {
        average += waveFormData[2 * i + 1];
        if (waveFormData[2 * i + 1] > max)
            max = waveFormData[2 * i + 1];
        if (waveFormData[2 * i + 1] < min)
            min = waveFormData[2 * i + 1];
    }
    range = max - min;
    average = average / VIEW_DATA_SIZE;
    int min_i, max_i;
    float rise_i, freq_i; //上升时间最小值坐标，最大值坐标
    for (int i = 3000; i < VIEW_DATA_SIZE; i++)
    {
        if ((waveFormData[2 * i - 1] < (0.293 * range + min)) && (waveFormData[2 * i + 1] > (0.293 * range + min)))
        {
            min_i = i;
            for (int j = i; j < VIEW_DATA_SIZE; j++)
            {
                if ((waveFormData[2 * j - 1] < (0.707 * range + min)) && (waveFormData[2 * j + 1] > (0.707 * range + min)))
                {
                    max_i = j;
                    break;
                }
            }
            rise_i = (max_i - min_i);
            for (int j = i; j < VIEW_DATA_SIZE; j++)
            {
                if ((waveFormData[2 * j - 1] < (0.293 * range + min)) && (waveFormData[2 * j + 1] > (0.293 * range + min)))
                {
                    max_i = j;
                    if ((max_i - min_i) > rise_i)
                        break;
                }
            }
            freq_i = (max_i - min_i);
            break;
        }
    }
    float Vac, Vac1, Vac2;
    Vac1 = abs(max) / sqrt(2);
    Vac2 = abs(min) / sqrt(2);
    Vac = (Vac1 > Vac2) ? Vac1 : Vac2;
    background.measuredValue[0].value = max;
    background.measuredValue[1].value = min;
    background.measuredValue[2].value = average;
    background.measuredValue[3].value = Vac;
    background.measuredValue[4].value = rise_i / samplingRate * pow(10, timeExponent);
    background.measuredValue[5].value = freq_i / samplingRate * pow(10, timeExponent);
}
void waveAutoSet()
{
    float max = background.measuredValue[0].value;
    float min = background.measuredValue[1].value;
    float period = background.measuredValue[5].value * samplingRate / pow(10, timeExponent);
    if (max - min == 0)
        scaleState = 1000.0;
    else
        scaleState = autoVoltageNum * 1000.0f / (max - min);
    if (scaleState < 0.5f)
        scaleState = 0.5f;
    if (scaleState > 1000.01f)
        scaleState = 1000.0f;
    offsetState.y = (max + min) / 2 / 10000.0f;
    trigLevelState = (max + min) / 2;
    timeStep = autoPeriodNum * period / VIEWADLE_DATA_SIZE;
    if (timeStep == 0)
        timeStep = 1000;
    while (timeStep >= 10)
    {
        if (timeExponent < 7)
            timeExponent++;
        timeStep /= 10;
    }
    while (timeStep < 1)
    {
        if (timeExponent > 0)
            timeExponent--;
        timeStep *= 10;
    }
    offsetState.x = -0.5 * timeStep;
}

void dataTrig()
{
    if (waveTrig)
    {
        float level = trigLevelState + trigLevel;
        int preTrigDepth = VIEWADLE_DATA_SIZE * timeStep / 2;
        //int preTrigDepth = 0;
        int trigNum = 0;
        for (int i = 3000 + preTrigDepth; i < DATA_SIZE - VIEW_DATA_SIZE; i++)
        {
            if ((waveData[2 * i + 1] > level) && (waveData[2 * i - 1] < level))
            {
                for (int j = 0; j < VIEW_DATA_SIZE; j++)
                {
                    waveFormData[2 * j + 1] = waveData[2 * (i - preTrigDepth + j) + 1] + waveFormData[2 * j + 1];
                }
                i += VIEW_DATA_SIZE;
                trigNum++;
            }
        }
        if (trigNum == 0)
        {
            for (int i = 0; i < VIEW_DATA_SIZE; i++)
            {
                waveFormData[2 * i] = (float)i;
                waveFormData[2 * i + 1] = waveData[2 * (i + 3000) + 1];
            }
            return;
        }
        for (int i = 0; i < VIEW_DATA_SIZE; i++)
        {
            waveFormData[2 * i] = (float)i;
            waveFormData[2 * i + 1] /= trigNum;
        }
    }
    else
    {
        for (int i = 0; i < VIEW_DATA_SIZE; i++)
        {
            waveFormData[2 * i] = (float)i;
            waveFormData[2 * i + 1] = waveData[2 * (i + 3000) + 1];
        }
    }
}

void dataTrig_without_oversampling()
{
    if (waveTrig)
    {
        float level = trigLevelState + trigLevel;
        int preTrigDepth = VIEWADLE_DATA_SIZE * timeStep / 2;
        //int preTrigDepth = 0;
        int trigNum = 0;
        for (int i = 3000 + preTrigDepth; i < DATA_SIZE - VIEW_DATA_SIZE; i++)
        {
            if ((waveData[2 * i + 1] > level) && (waveData[2 * i - 1] < level))
            {
                for (int j = 0; j < VIEW_DATA_SIZE; j++)
                {
                    waveFormData[2 * j] = (float)j;
                    waveFormData[2 * j + 1] = waveData[2 * (i - preTrigDepth + j) + 1] /*+ waveFormData[2 * j + 1]*/;
                }
                i += VIEW_DATA_SIZE;
                trigNum++;
            }
            if (trigNum == 1)
                break;
        }
        if (trigNum == 0)
        {
            for (int i = 0; i < VIEW_DATA_SIZE; i++)
            {
                waveFormData[2 * i] = (float)i;
                waveFormData[2 * i + 1] = waveData[2 * (i + 3000) + 1];
            }
            return;
        }
    }
    else
    {
        for (int i = 0; i < VIEW_DATA_SIZE; i++)
        {
            waveFormData[2 * i] = (float)i;
            waveFormData[2 * i + 1] = waveData[2 * (i + 3000) + 1];
        }
    }
}

void threadRestart()
{
    //子线程不知道为什么总是卡死，此函数判断子线程卡死则杀掉子线程并重启
    static float duration = glfwGetTime();
    static unsigned times = 0;
    if (times != threadCount)
    {
        times = threadCount;
        duration = glfwGetTime();
    }
    else
    {
        if ((glfwGetTime() - duration) > 3.0f) //卡住3秒则重启
        {
            threadNum++;
            cout << "第" << threadNum << "次重新连接" << endl;
            cout << "时间：" << glfwGetTime() << endl;
            TerminateThread(threadHandle, EXIT_FAILURE);
            CloseHandle(threadHandle);
            threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Receive_Data, nullptr, 0, NULL);
            duration = glfwGetTime();
        }
    }
}

#if DEBUG
UINT Receive_Data(LPVOID lpVoid)
{
    while (TRUE)
    {
        if (wavePause)
            continue;
        float timeValue = glfwGetTime();
        float period = debugWavePeriod * samplingRate / pow(10, timeExponent);
        for (unsigned i = 0; i < DATA_SIZE; i++)
        {
            waveData[2 * i] = (float)i;
            waveData[2 * i + 1] = debugWaveRange / 2 * sin((float)i * 6.2832f / period + timeValue) + debugWaveOffset;
        }
        if (overSampling)
            dataTrig();
        else
            dataTrig_without_oversampling();
        waveParameter();
        Sleep(500);
    }
    return 0;
}
#else
UDPconnector FPGA;
UINT Receive_Data(LPVOID lpVoid)
{
    if (threadNum == 0)
    {
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
    }
    short dataBuf[DATA_SIZE];
    while (TRUE)
    {
        if (wavePause)
            continue;
        FPGA.ClearBuffer();
        FPGA.SendMsg(DATA_SIZE, timeExponent);
        for (int i = 0; i < DATA_SIZE / 1024; i++)
        {
            FPGA.ReceiveData((char *)(dataBuf + i * 1024), 1024);
            FPGA.ReceiveData((char *)(dataBuf + i * 1024 + 512), 1024);
        }
        for (int i = 0; i < DATA_SIZE; i++)
        {
            waveData[2 * i] = (float)i;
            waveData[2 * i + 1] = (float)dataBuf[i] * 5000.0f / 2048.0f + 183; //AD采样板偏差纠正
        }
        if (overSampling)
            dataTrig();
        else
            dataTrig_without_oversampling();
        waveParameter();
        threadCount++;
        Sleep(500);
    }
    return 0;
}
#endif
