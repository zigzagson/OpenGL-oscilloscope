#include "udp_upper.h"

BOOL UDPconnector::EstablishConnection()
{
    // 发送询问信号
    char sendInqBuf[INQ] = {0x28, 0x00, 0x01, 0x00, 0x01};
    if (SOCKET_ERROR == sendto(this->Upper, sendInqBuf, INQ, 0, (struct sockaddr *)&this->brdctAddr, sizeof(this->brdctAddr)))
    {
        printf("错误代码：%d", WSAGetLastError());
        return false;
    }
    //接收应答
    struct sockaddr fromAddr;
    int addrLen = sizeof(fromAddr);
    char ResHead[5] = {0x29, 0, 1, 0, 1};
    char recvResBuf[RES];
    recvfrom(Upper, recvResBuf, RES, 0, &fromAddr, &addrLen);
    if (strncmp(recvResBuf, ResHead, 5) != 0)
        return false;

    struct in_addr fpgaIpAddr; //获取FPGA的IP地址
    memcpy(&fpgaIpAddr, recvResBuf + 11, 4);
    this->lower_IP = inet_ntoa(fpgaIpAddr);
    memcpy(this->lower_MAC, recvResBuf + 5, 6); //获取FPGA的MAC地址
    return true;
}
void UDPconnector::SendMsg(unsigned int size, char step)
{
    //发出控制信号
    char sendCtlBuf[CTL] = {0x28, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0};
    memcpy(sendCtlBuf + 5, this->lower_MAC, 6); //添加MAC信息
    sendCtlBuf[CTL - 1] = step;                 //添加时机信息
    for (int i = 0; i < 4; i++)                 //添加采样次数信息
        sendCtlBuf[15 + i] = ((char *)&size)[3 - i];
    sendto(this->Upper, sendCtlBuf, CTL, 0, (struct sockaddr *)&this->brdctAddr, sizeof(this->brdctAddr));
}
void UDPconnector::ReceiveData(char *buffer, int size)
{
    //接收数据
    struct sockaddr fromAddr;
    int addrLen = sizeof(fromAddr);
    char DtaHead[5] = {0x29, 0, 1, 0, 2};
    char recvDtaBuf[DTA];
    while (TRUE)
    {
        if (recvfrom(this->Upper, recvDtaBuf, DTA, 0, &fromAddr, &addrLen) > 0)
        {
            if (strncmp(recvDtaBuf, DtaHead, 5) == 0)
            {
                memcpy(buffer, recvDtaBuf + 5, size);
                char tmp;
                for (int i = 0; i < size / 2; i++) //大小端转换
                {
                    tmp = buffer[i * 2];
                    buffer[i * 2] = buffer[i * 2 + 1];
                    buffer[i * 2 + 1] = tmp;
                }
                break;
            }
        }
    }
}
void UDPconnector::ClearBuffer()
{
    //清除缓冲
    struct sockaddr fromAddr;
    int addrLen = sizeof(fromAddr);
    char recvDtaBuf[DTA];
    while (recvfrom(this->Upper, recvDtaBuf, DTA, 0, &fromAddr, &addrLen) > 0)
    {
    }
}
string UDPconnector::lowerMAC()
{
    //转成16进制的MAC地址字符串
    string MAC;
    char ch;
    for (int i = 0; i < 6; i++)
    {
        ch = (this->lower_MAC[i] & 0xf0) >> 4;
        MAC += ((ch < 10) ? ch + '0' : ch - 10 + 'A');
        ch = this->lower_MAC[i] & 0xf;
        MAC += ((ch < 10) ? ch + '0' : ch - 10 + 'A');
        ch = ':';
        MAC += ch;
    }
    MAC.erase(17); //删掉最后的冒号
    return MAC;
}
string UDPconnector::lowerIP()
{
    return this->lower_IP;
}
BOOL UDPconnector::ConnectionInit()
{
    // 初始化 Winsock 库
    WSADATA wsaData = {0};
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 创建套接字
    this->Upper = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == this->Upper)
    {
        return FALSE;
    }

    // 设置上位机地址和端口信息
    memset(&this->upperAddr, 0, sizeof(this->upperAddr));
    this->upperAddr.sin_family = AF_INET;
    this->upperAddr.sin_port = ::htons(8080);                          //指定绑定端口
    this->upperAddr.sin_addr.S_un.S_addr = ::inet_addr("192.168.0.3"); //绑定的IP

    if (bind(this->Upper, (sockaddr *)&this->upperAddr, sizeof(this->upperAddr)) != 0)
        return FALSE; //绑定失败返回

    BOOL optVal = TRUE;
    int optLen = sizeof(BOOL);
    if (setsockopt(this->Upper, SOL_SOCKET, SO_BROADCAST, (char *)&optVal, optLen) != 0) //设置广播
        return FALSE;

    struct timeval timeout;
    timeout.tv_sec = 1;                                                                            //秒
    timeout.tv_usec = 0;                                                                           //微秒
    if (setsockopt(this->Upper, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) == -1) //设置超时
        return FALSE;

    // 设置广播地址和端口信息
    memset(&this->brdctAddr, 0, sizeof(this->brdctAddr));
    this->brdctAddr.sin_family = AF_INET;
    this->brdctAddr.sin_port = ::htons(8080); //指定连接端口
    this->brdctAddr.sin_addr.S_un.S_addr = ::inet_addr("255.255.255.255");

    return TRUE;
}