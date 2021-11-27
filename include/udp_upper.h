#ifndef UDP_UPPER_H
#define UDP_UPPER_H

#include <iostream>
#include <string>
#include "winsock2.h"
//#pragma comment(lib, "Ws2_32.lib")

#define INQ 5    //询问信号字节数
#define RES 27   //应答信号字节数
#define CTL 20   //控制信号字节数
#define DTA 1029 //数据信号字节数

using namespace std;

class UDPconnector
{
public:
    UDPconnector() {}
    BOOL ConnectionInit();      //连接初始化
    BOOL EstablishConnection(); //连接建立
    void SendMsg(unsigned int size, char step);
    void ReceiveData(char *buffer, int size);
    void ClearBuffer();
    string lowerMAC();
    string lowerIP();

private:
    SOCKET Upper = INVALID_SOCKET;    // 上位机套接字
    sockaddr_in brdctAddr, upperAddr; // 广播地址，上位机地址
    char lower_MAC[6] = {0};          //下位机MAC
    char *lower_IP;                   //下位机IP
};
#endif