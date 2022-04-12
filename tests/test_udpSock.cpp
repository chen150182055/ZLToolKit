/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <csignal>
#include <iostream>
#include "Util/util.h"
#include "Util/logger.h"
#include "Network/Socket.h"

using namespace std;
using namespace toolkit;

//���߳��˳���־
bool exitProgram = false;

//��ֵstruct sockaddr
void makeAddr(struct sockaddr *out, const char *ip, uint16_t port) {
    struct sockaddr_in &servaddr = *((struct sockaddr_in *) out);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    bzero(&(servaddr.sin_zero), sizeof servaddr.sin_zero);
}

//��ȡstruct sockaddr��IP�ַ���
string getIP(struct sockaddr *addr) {
    return SockUtil::inet_ntoa(((struct sockaddr_in *) addr)->sin_addr);
}

int main() {
    //���ó����˳��źŴ�����
    signal(SIGINT, [](int) { exitProgram = true; });
    //������־ϵͳ
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    Socket::Ptr sockRecv = Socket::createSocket();//����һ��UDP���ݽ��ն˿�
    Socket::Ptr sockSend = Socket::createSocket();//����һ��UDP���ݷ��Ͷ˿�
    sockRecv->bindUdpSock(9001);//����UDP��9001�˿�
    sockSend->bindUdpSock(0);//����UDP����˿�

    sockRecv->setOnRead([](const Buffer::Ptr &buf, struct sockaddr *addr, int) {
        //���յ����ݻص�
        DebugL << "recv data form " << getIP(addr) << ":" << buf->data();
    });

    struct sockaddr addrDst;
    makeAddr(&addrDst, "127.0.0.1", 9001);//UDP���ݷ��͵�ַ
//	sockSend->bindPeerAddr(&addrDst);
    int i = 0;
    while (!exitProgram) {
        //ÿ��һ�����Է���������
        sockSend->send(to_string(i++), &addrDst, sizeof(struct sockaddr_in));
        sleep(1);
    }
    return 0;
}





