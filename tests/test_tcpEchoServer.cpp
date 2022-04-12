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

#ifndef _WIN32
#include <unistd.h>
#endif

#include "Util/logger.h"
#include "Util/TimeTicker.h"
#include "Network/TcpServer.h"
#include "Network/TcpSession.h"

using namespace std;
using namespace toolkit;

class EchoSession : public TcpSession {
public:
    EchoSession(const Socket::Ptr &sock) :
            TcpSession(sock) {
        DebugL;
    }

    ~EchoSession() {
        DebugL;
    }

    virtual void onRecv(const Buffer::Ptr &buf) override {
        //����ͻ��˷��͹���������
        TraceL << buf->data() << " from port:" << get_local_port();
        send(buf);
    }

    virtual void onError(const SockException &err) override {
        //�ͻ��˶Ͽ����ӻ�����ԭ���¸ö�������TCPServer����
        WarnL << err.what();
    }

    virtual void onManager() override {
        //��ʱ����ö���Ʃ��Ự��ʱ���
        DebugL;
    }

private:
    Ticker _ticker;
};


int main() {
    //��ʼ����־ģ��
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    //����֤�飬֤�������Կ��˽Կ
    SSL_Initor::Instance().loadCertificate((exeDir() + "ssl.p12").data());
    SSL_Initor::Instance().trustCertificate((exeDir() + "ssl.p12").data());
    SSL_Initor::Instance().ignoreInvalidCertificate(false);

    TcpServer::Ptr server(new TcpServer());
    server->start<EchoSession>(9000);//����9000�˿�

    TcpServer::Ptr serverSSL(new TcpServer());
    serverSSL->start<TcpSessionWithSSL<EchoSession> >(9001);//����9001�˿�

    //�˳������¼�����
    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
    sem.wait();
    return 0;
}