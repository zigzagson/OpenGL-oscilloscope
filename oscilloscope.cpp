#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <json/json.h>

#include "udp_upper.h"
#include "wave_renderer.h"
#include "three_dim_wave_renderer.h"
#include "draw_background.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <chrono>
#include <cmath>
#include <random>

#define DEBUG 1

using namespace std;

void windowInit();
void configurationInit();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_press_callback(GLFWwindow *window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void drawWaveForm();
void waveParameter();
void waveAutoSet();
void dataProcessing();
void waveOverSampling();       //前后overSamplingDigit个点做平均
void dataTrig();               //平均模式触发
void dataTrig_direct();        //单次触发
void threeDimWavaDataMapper(); //三维波形数据映射
UINT Receive_Data(LPVOID lpVoid);
void threadRestart();

#define DATA_SIZE 262144
#define VIEW_DATA_SIZE 8192
//#define VIEWADLE_DATA_SIZE 600
#define TIME_DIVS 12
int dataSize = DATA_SIZE; //实际使用数据量
float scrWidth = 1200;
float scrHeight = 750;
float viewWidth = scrWidth * 0.70;
float viewHeight = scrHeight * 0.92;

glm::vec2 offset(0.0f, 0.0f);       //偏移，既图像拖动量; 横轴单位是1/TIME_DIVS个窗口内的数据量;纵轴单位是mV
glm::vec2 offsetState(-0.0f, 0.0f); //历史偏移状态记录
float scaleState = 1;               //纵轴缩放，窗口显示10000/scaleState个mV
float timeStep = 1;                 //横轴步长，1/TIME_DIVS个窗口内的时间(us)
int timeExponent = 0;               //使timeStep大于10^x的最大x值,暂时没用
float realTimeSamplingRate = 50;    //实时采样率，单位MHz
float samplingRate = 50;            //转换后的采样率，单位MHz（points/us）
int overSamplingDigit = 1;          // 过采样。1既没有过采样，16既4位过采样
float trigLevel = 0;
float trigLevelState = 100;
bool waveTrig = false;
bool trigAverage = false; //多次触发图像做平均
bool wavePause = false;
bool threeDim = false;
bool multiColor = false;

int maxColorGrade = 32; //最大颜色等级

int autoPeriodNum = 3;
int autoVoltageNum = 6;
float scrollSensitivity = 1;

HANDLE threadHandle;
unsigned threadCount = 0;
unsigned threadNum = 0;
GLFWwindow *window;
BackgroundRender background;
WaveRenderer wave;
ThreeDimWaveRenderer wave3d;

float waveFormData[VIEW_DATA_SIZE * 2];
float waveData[DATA_SIZE];
unsigned char threeDimDataBase[1024][512];
float threeDimWaveData[1024 * 512 * 3];

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
    // OpenGL上下文属于单个线程，在其他线程没有上下文时不可以使用OpenGL函数
    configurationInit();
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        drawWaveForm(); //画图像
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void drawWaveForm()
{
#if DEBUG
#else
    threadRestart();
#endif
    if (threeDim)
    {
        if (!wavePause)
            wave3d.ResetWaveData(threeDimWaveData, sizeof(threeDimWaveData));
        wave3d.RenderWave(offsetState + offset, timeStep * samplingRate, scaleState, multiColor);              //画波形
        background.drawBackground(timeStep, scaleState, offsetState.y + offset.y, trigLevel + trigLevelState); //画网格
    }
    else
    {
        if (!wavePause)
            wave.ResetWaveData(waveFormData, dataSize);
        background.drawBackground(timeStep, scaleState, offsetState.y + offset.y, trigLevel + trigLevelState); //画网格
        wave.RenderWave(offsetState + offset, timeStep * samplingRate, scaleState, waveColor);                 //画波形
    }
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
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    wave.WaveRenderInit("shader/wave.vs", "shader/wave.fs");
    wave.SetWaveAttribute(TIME_DIVS, -5000, 5000);
    wave3d.WaveRenderInit();
    wave3d.SetWaveAttribute(TIME_DIVS, -5000, 5000);
    wave3d.SetColorGrade(maxColorGrade);
    wave3d.SetBackgroundColor(glm::vec3(0.0f, 0.0f, 0.0f));
    wave3d.SetColorAttribute(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
    wave3d.SetColorAttribute(glm::vec3(0.0f, 0.0f, 1.0f),
                             glm::vec3(0.0f, 1.0f, 1.0f),
                             glm::vec3(0.0f, 1.0f, 0.0f),
                             glm::vec3(1.0f, 1.0f, 0.0f),
                             glm::vec3(1.0f, 0.0f, 0.0f));
    background.BackgroundRenderInit(scrWidth, scrHeight);
    background.setSize(scrWidth, scrHeight, scrWidth * 0.1, scrHeight * 0.06, viewWidth, viewHeight);
    background.maxColorGrade = maxColorGrade;
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
        Json::Value wave_3d;
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
        color["wave"].append(1.0f);
        color["wave"].append(1.0f);
        color["wave"].append(0.0f);
        color["trig"].append(0.9f);
        color["trig"].append(0.3f);
        color["trig"].append(0.1f);
        wave_3d["begin"].append(0.0f);
        wave_3d["begin"].append(0.0f);
        wave_3d["begin"].append(1.0f);
        wave_3d["second"].append(0.0f);
        wave_3d["second"].append(1.0f);
        wave_3d["second"].append(1.0f);
        wave_3d["third"].append(0.0f);
        wave_3d["third"].append(1.0f);
        wave_3d["third"].append(0.0f);
        wave_3d["fourth"].append(1.0f);
        wave_3d["fourth"].append(1.0f);
        wave_3d["fourth"].append(0.0f);
        wave_3d["end"].append(1.0f);
        wave_3d["end"].append(0.0f);
        wave_3d["end"].append(0.0f);
        color["wave3d"] = wave_3d;
        root["color"] = color;
        root["rate"] = realTimeSamplingRate;
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
    realTimeSamplingRate = root["rate"].asFloat();
    scrollSensitivity = root["scroll"].asFloat();
    autoVoltageNum = root["auto"][0].asInt();
    autoPeriodNum = root["auto"][1].asInt();
    Json::Value color = root["color"];
    backgroundColor = glm::vec3(
        color["background"][0].asFloat(),
        color["background"][1].asFloat(),
        color["background"][2].asFloat());
    waveColor = glm::vec3(
        color["wave"][0].asFloat(),
        color["wave"][1].asFloat(),
        color["wave"][2].asFloat());
    iconColor = glm::vec4(
        color["icon"][0].asFloat(),
        color["icon"][1].asFloat(),
        color["icon"][2].asFloat(),
        color["icon"][3].asFloat());
    iconClickColor = glm::vec4(
        color["iconClick"][0].asFloat(),
        color["iconClick"][1].asFloat(),
        color["iconClick"][2].asFloat(),
        color["iconClick"][3].asFloat());
    background.borderColor = glm::vec3(
        color["bordor"][0].asFloat(),
        color["bordor"][1].asFloat(),
        color["bordor"][2].asFloat());
    background.gridColor = glm::vec3(
        color["grid"][0].asFloat(),
        color["grid"][1].asFloat(),
        color["grid"][2].asFloat());
    background.textColor = glm::vec3(
        color["text"][0].asFloat(),
        color["text"][1].asFloat(),
        color["text"][2].asFloat());
    background.trigLineColor = glm::vec3(
        color["trig"][0].asFloat(),
        color["trig"][1].asFloat(),
        color["trig"][2].asFloat());
    background.iconTexture.color = iconColor;
    Json::Value wave_3d = color["wave3d"];
    wave3d.SetBackgroundColor(backgroundColor);
    wave3d.SetColorAttribute(
        glm::vec3(wave_3d["begin"][0].asFloat(), wave_3d["begin"][1].asFloat(), wave_3d["begin"][2].asFloat()),
        glm::vec3(wave_3d["end"][0].asFloat(), wave_3d["end"][1].asFloat(), wave_3d["end"][2].asFloat()));
    wave3d.SetColorAttribute(
        glm::vec3(wave_3d["begin"][0].asFloat(), wave_3d["begin"][1].asFloat(), wave_3d["begin"][2].asFloat()),
        glm::vec3(wave_3d["second"][0].asFloat(), wave_3d["second"][1].asFloat(), wave_3d["second"][2].asFloat()),
        glm::vec3(wave_3d["third"][0].asFloat(), wave_3d["third"][1].asFloat(), wave_3d["third"][2].asFloat()),
        glm::vec3(wave_3d["fourth"][0].asFloat(), wave_3d["fourth"][1].asFloat(), wave_3d["fourth"][2].asFloat()),
        glm::vec3(wave_3d["end"][0].asFloat(), wave_3d["end"][1].asFloat(), wave_3d["end"][2].asFloat()));
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
        if (timeStep < 1 || timeStep >= 1000000)
            timeStep *= pow(10, scrollSensitivity * yoffset / 10);
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
        if (posX > scrWidth * 0.82 && posX < scrWidth * 0.88 && posY > scrHeight * 0.32 && posY < scrHeight * 0.35) //是否过采样
        {
            background.ifOverSampling = !background.ifOverSampling;
            if (background.ifOverSampling)
                overSamplingDigit = 16; //过采样4位
            else
                overSamplingDigit = 1;
        }
        if (posX > scrWidth * 0.92 && posX < scrWidth * 0.98 && posY > scrHeight * 0.32 && posY < scrHeight * 0.35) //触发是否平均模式
        {
            trigAverage = !trigAverage;
            background.ifTrigAverage = trigAverage;
        }
        if (posX > scrWidth * 0.82 && posX < scrWidth * 0.9 && posY > scrHeight * 0.36 && posY < scrHeight * 0.39) //是否三维映射
        {
            threeDim = !threeDim;
            background.ifThreeDim = threeDim;
        }
        if (posX > scrWidth * 0.82 && posX < scrWidth * 0.88 && posY > scrHeight * 0.41 && posY < scrHeight * 0.44) //单色系或多色系
        {
            if (threeDim)
            {
                multiColor = !multiColor;
                background.ifMultiColor = multiColor;
            }
        }
        if (posX > scrWidth * 0.9 && posX < scrWidth * 0.95 && posY > scrHeight * 0.46 && posY < scrHeight * 0.5) //灰度等级输入
        {
            if (threeDim)
            {
                background.gradeBox.ifInputBoxShow = true;
            }
        }
        else
        {
            background.gradeBox.exitInputBox();
        }
        if (Rsquare < scrWidth * 0.06 * scrWidth * 0.06)
        {
            background.iconTexture.color = iconClickColor;
            waveAutoSet(); //左键点图标autoset
        }
        if (posX > scrWidth * 0.18 && posX < scrWidth * 0.24 && posY > scrHeight * 0.95 && posY < scrHeight * 0.99) //电压参数输入
        {
            background.volBox.ifInputBoxShow = true;
        }
        else
        {
            background.volBox.exitInputBox();
        }
        if (posX > scrWidth * 0.38 && posX < scrWidth * 0.44 && posY > scrHeight * 0.95 && posY < scrHeight * 0.99) //时基参数输入
        {
            background.timeBox.ifInputBoxShow = true;
        }
        else
        {
            background.timeBox.exitInputBox();
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
    //左键拖动
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) //左键松开后
    {
        offsetState += offset;
        if (waveTrig)
            offsetState.x = 0;
        trigLevelState += trigLevel;
        offset.x = 0;
        offset.y = 0;
        trigLevel = 0;
        firstPress = true;
        return;
    }
    if (firstPress) //左键按下时记录初始坐标
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
        offset.x = (xpos - firstX) / viewWidth * timeStep * samplingRate;
        offset.y = (ypos - firstY) / viewHeight / scaleState;
        if (waveTrig)
            offset.x = 0;
    }
    if (type && waveTrig)
        trigLevel = -(ypos - firstY) * 10000 / viewHeight / scaleState;
}
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE)
    {
        background.volBox.TypeIn(key);
        background.timeBox.TypeIn(key);
        background.gradeBox.TypeIn(key);
        if (key == GLFW_KEY_ENTER)
        {
            if (background.volBox.ifInputBoxShow)
            {
                scaleState = 1000.0f / background.volBox.getInputValue();
                background.volBox.exitInputBox();
                if (scaleState > 1000.0)
                    scaleState = 1000.0f;
                if (scaleState < 0.5)
                    scaleState = 0.5;
            }
            if (background.timeBox.ifInputBoxShow)
            {
                timeStep = background.timeBox.getInputValue();
                background.timeBox.exitInputBox();
            }
            if (background.gradeBox.ifInputBoxShow)
            {
                maxColorGrade = background.gradeBox.getInputValue();
                background.maxColorGrade = maxColorGrade;
                wave3d.SetColorGrade(maxColorGrade);
                background.gradeBox.exitInputBox();
            }
        }
    }
}

void waveParameter()
{
    float min, max, range, average;
    min = waveFormData[1];
    max = waveFormData[1];
    // if (!(min == min && max == max))
    // {
    //     min = 0;
    //     max = 0;
    // }
    for (int i = 1; i < dataSize; i++)
    {
        if (waveFormData[2 * i + 1] > max)
            max = waveFormData[2 * i + 1];
        if (waveFormData[2 * i + 1] < min)
            min = waveFormData[2 * i + 1];
    }
    range = max - min;
    int min_index, max_index;     //上升时间最小值下标，最大值下标
    int rise_points, freq_points; //上升时间，一周期 点数
    for (int i = 1; i < dataSize; i++)
    {
        if ((waveFormData[2 * i - 1] < (0.1 * range + min)) && (waveFormData[2 * i + 1] > (0.1 * range + min)))
        {
            min_index = i;
            for (int j = i; j < dataSize; j++)
            {
                if ((waveFormData[2 * j - 1] < (0.9 * range + min)) && (waveFormData[2 * j + 1] > (0.9 * range + min)))
                {
                    max_index = j;
                    break;
                }
            }
            rise_points = (max_index - min_index);
            break;
        }
    }
    for (int i = 1; i < dataSize; i++)
    {
        if ((waveFormData[2 * i - 1] < (0.5 * range + min)) && (waveFormData[2 * i + 1] > (0.5 * range + min)))
        {
            min_index = i;
            for (int j = i + 1; j < dataSize; j++)
            {
                if ((waveFormData[2 * j - 1] < (0.5 * range + min)) && (waveFormData[2 * j + 1] > (0.5 * range + min)))
                {
                    max_index = j;
                    if ((max_index - min_index) > rise_points)
                        break;
                }
            }
            freq_points = (max_index - min_index);
            break;
        }
    }
    average = 0;
    for (int i = min_index; i < max_index; i++)
        average += waveFormData[2 * i + 1];
    average = average / freq_points;
    float Vac, Vac1, Vac2;
    Vac1 = abs(max) / sqrt(2);
    Vac2 = abs(min) / sqrt(2);
    Vac = (Vac1 > Vac2) ? Vac1 : Vac2;
    background.measuredValue[0].value = max;
    background.measuredValue[1].value = min;
    background.measuredValue[2].value = average;
    background.measuredValue[3].value = Vac;
    background.measuredValue[4].value = rise_points / samplingRate;
    background.measuredValue[5].value = freq_points / samplingRate;
}
void waveAutoSet()
{
    float max = background.measuredValue[0].value;
    float min = background.measuredValue[1].value;
    float period = background.measuredValue[5].value;
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
    timeStep = autoPeriodNum * period / TIME_DIVS;
    if (timeStep == 0)
        timeStep = 100;
    offsetState.x = 0;
}

void waveOverSampling()
{
    if (overSamplingDigit != 1)
    {
        float tmp = 0;
        for (int i = 0; i < DATA_SIZE / overSamplingDigit; i++)
        {
            for (int j = 0; j < overSamplingDigit; j++)
            {
                tmp += waveData[overSamplingDigit * i + j];
            }
            waveData[i] = tmp / overSamplingDigit;
            tmp = 0;
        }
    }
    dataSize = DATA_SIZE / overSamplingDigit;
    samplingRate = realTimeSamplingRate / overSamplingDigit;
}
void dataTrig()
{
    static int lastTrigNum = 0;
    float level = trigLevelState + trigLevel;
    // int preTrigDepth = TIME_DIVS / 2 * timeStep * samplingRate; //暂时没有用，预触发深度始终在中间
    int trigNum = 0;
    int waveForm[VIEW_DATA_SIZE];
    for (int i = 0; i < VIEW_DATA_SIZE; i++)
    {
        waveForm[i] = waveFormData[2 * i + 1] * lastTrigNum;
    }
    for (int i = VIEW_DATA_SIZE / 2; i < dataSize - VIEW_DATA_SIZE / 2; i++)
    {
        if ((waveData[i + 2] > level) && (waveData[i + 1] > level) && (waveData[i] > level) &&
            (waveData[i - 1] < level) && (waveData[i - 2] < level) && (waveData[i - 3] < level))
        {
            for (int j = 0; j < VIEW_DATA_SIZE; j++) //触发点在波形数据中间
            {
                waveForm[j] += waveData[i + j - VIEW_DATA_SIZE / 2];
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
            waveFormData[2 * i + 1] = waveData[i];
        }
        return;
    }
    for (int i = 0; i < VIEW_DATA_SIZE; i++)
    {
        waveFormData[2 * i] = (float)i;
        waveFormData[2 * i + 1] = waveForm[i] / (trigNum + lastTrigNum);
    }
    lastTrigNum = trigNum;
}

void dataTrig_direct()
{
    float level = trigLevelState + trigLevel;
    // int preTrigDepth = TIME_DIVS / 2 * timeStep * samplingRate; //暂时没有用，预触发深度始终在中间
    int trigNum = 0;
    for (int i = VIEW_DATA_SIZE / 2; i < dataSize - VIEW_DATA_SIZE / 2; i++)
    {
        if ((waveData[i + 2] > level) && (waveData[i + 1] > level) && (waveData[i] > level) &&
            (waveData[i - 1] < level) && (waveData[i - 2] < level) && (waveData[i - 3] < level))
        {
            for (int j = 0; j < VIEW_DATA_SIZE; j++) //触发点在波形数据中间
            {
                waveFormData[2 * j] = (float)j;
                waveFormData[2 * j + 1] = waveData[i + j - VIEW_DATA_SIZE / 2];
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
            waveFormData[2 * i + 1] = waveData[i];
        }
        return;
    }
}
void threeDimWavaDataMapper()
{
    float level = trigLevelState + trigLevel;
    // int preTrigDepth = TIME_DIVS / 2 * timeStep * samplingRate; //暂时没有用，预触发深度始终在中间
    int trigNum = 0;
    memset(threeDimDataBase, 0, sizeof(threeDimDataBase));
    for (int i = 1024 / 2; i < dataSize - 1024 / 2; i++)
    {
        if ((waveData[i + 2] > level) && (waveData[i + 1] > level) && (waveData[i] > level) &&
            (waveData[i - 1] < level) && (waveData[i - 2] < level) && (waveData[i - 3] < level))
        {
            for (int j = 0; j < 1024; j++) //触发点在波形数据中间
            {
                int yDim = (int)(waveData[i + j - 1024 / 2] / 5000 * 256 + 256);
                yDim = (yDim > 511) ? 511 : ((yDim < 0) ? 0 : yDim);
                if (threeDimDataBase[j][yDim] < 255)
                    threeDimDataBase[j][yDim]++;
            }
            i += 1024;
            trigNum++;
        }
    }
    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 512; j++)
        {
            threeDimWaveData[(i * 512 + j) * 3] = i;
            threeDimWaveData[(i * 512 + j) * 3 + 1] = (j - 256) * 5000 / 256;
            threeDimWaveData[(i * 512 + j) * 3 + 2] = threeDimDataBase[i][j];
        }
    }
    dataSize = 1024;
}
void dataProcessing()
{
    dataSize = DATA_SIZE;
    samplingRate = realTimeSamplingRate;
    timeExponent = 0;
    for (int temp = timeStep; temp >= 10; timeExponent++)
        temp /= 10;

    if (threeDim)
    {
        waveTrig = true;
        background.ifTrig = true;
        for (int temp = timeStep, i = 0; temp >= 2 && i < 5; temp /= 2, i++)
        {
            dataSize /= 2;
            samplingRate /= 2;
            for (int i = 0; i < dataSize; i++)
            {
                waveData[i] = waveData[2 * i];
            }
        }
        threeDimWavaDataMapper();
    }
    else
    {
        waveOverSampling();
        if (waveTrig)
        {
            if (overSamplingDigit == 1)
            {
                if (timeStep >= 8 && timeStep < 16)
                {
                    dataSize /= 8;
                    samplingRate /= 8;
                    for (int i = 0; i < dataSize; i++)
                    {
                        waveData[i] = waveData[8 * i];
                    }
                }
                else if (timeStep >= 16)
                {
                    dataSize /= 16;
                    samplingRate /= 16;
                    for (int i = 0; i < dataSize; i++)
                    {
                        waveData[i] = waveData[16 * i];
                    }
                }
            }
            if (trigAverage)
                dataTrig();
            else
                dataTrig_direct();
        }
        else
        {
            if (timeStep < 8)
            {
                for (int i = 0; i < VIEW_DATA_SIZE; i++)
                {
                    waveFormData[2 * i] = (float)i;
                    waveFormData[2 * i + 1] = waveData[i];
                }
            }
            else if (timeStep < 32)
            {
                dataSize /= 8;
                samplingRate /= 8;
                for (int i = 0; i < VIEW_DATA_SIZE && i < dataSize; i++)
                {
                    waveFormData[2 * i] = (float)i;
                    waveFormData[2 * i + 1] = waveData[8 * i];
                }
            }
            else
            {
                dataSize /= 32;
                samplingRate /= 32;
                for (int i = 0; i < VIEW_DATA_SIZE && i < dataSize; i++)
                {
                    waveFormData[2 * i] = (float)i;
                    waveFormData[2 * i + 1] = waveData[32 * i];
                }
            }
        }
        dataSize = (dataSize < VIEW_DATA_SIZE) ? dataSize : VIEW_DATA_SIZE;
        waveParameter();
    }
}

void threadRestart()
{
    //子线程不知道为什么总是卡死，此函数判断子线程是否卡死，若是则杀掉子线程并重启
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
double Gaussian_noise(double mean, double stddev)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::normal_distribution<double> dist(mean, stddev);
    return dist(generator);
}
#if DEBUG
UINT Receive_Data(LPVOID lpVoid)
{
    while (TRUE)
    {
        if (wavePause)
            continue;
        float timeValue = glfwGetTime();
        float period = debugWavePeriod * realTimeSamplingRate;
        for (unsigned i = 0; i < DATA_SIZE; i++)
        {
            float noise = 1;
            if (threeDim)
                noise = Gaussian_noise(1, 0.2);
            waveData[i] = noise * debugWaveRange / 2 * sin((float)i * 6.2832f / period + timeValue) + debugWaveOffset;
        }
        dataProcessing();
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
            waveData[i] = (float)dataBuf[i] * 5000.0f / 2048.0f + 183; // AD采样板偏差纠正
        }
        dataProcessing();
        threadCount++;
        Sleep(500);
    }
    return 0;
}
#endif
