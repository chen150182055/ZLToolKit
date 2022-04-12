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
#include "Util/logger.h"
#include "Network/TcpClient.h"

using namespace std;
using namespace toolkit;

class TestClient : public TcpClient {
public:
    using Ptr = std::shared_ptr<TestClient>;

    TestClient() : TcpClient() {
        DebugL;
    }

    ~TestClient() {
        DebugL;
    }

protected:
    virtual void onConnect(const SockException &ex) override {
        //���ӽ���¼�
        InfoL << (ex ? ex.what() : "success");
    }

    virtual void onRecv(const Buffer::Ptr &pBuf) override {
        //���������¼�
        DebugL << pBuf->data() << " from port:" << get_peer_port();
    }

    virtual void onFlush() override {
        //���������󣬻�������¼�
        DebugL;
    }

    virtual void onErr(const SockException &ex) override {
        //�Ͽ������¼���һ����EOF
        WarnL << ex.what();
    }

    virtual void onManager() override {
        //��ʱ�������ݵ�������
        auto buf = BufferRaw::create();
        if (buf) {
            buf->assign("[BufferRaw]\0");
            (*this) << _nTick++ << " "
                    << 3.14 << " "
                    << string("string") << " "
                    << (Buffer::Ptr &) buf;
        }
    }

private:
    int _nTick = 0;
};


int main() {
    // ������־ϵͳ
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    TestClient::Ptr client(new TestClient());//����ʹ������ָ��
    client->startConnect("127.0.0.1", 9000);//���ӷ�����

    TcpClientWithSSL<TestClient>::Ptr clientSSL(new TcpClientWithSSL<TestClient>());//����ʹ������ָ��
    clientSSL->startConnect("127.0.0.1", 9001);//���ӷ�����

    //�˳������¼�����
    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
    sem.wait();
    return 0;
}