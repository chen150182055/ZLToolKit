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
#include "Util/CMD.h"
#include "Util/logger.h"
#include "Util/util.h"
#include "Network/TcpSession.h"
#include "Network/TcpServer.h"

using namespace std;
using namespace toolkit;

/**
* ���ԻỰ
*/
class EchoSession : public TcpSession {
public:
    EchoSession(const Socket::Ptr &pSock) : TcpSession(pSock) {
        DebugL;
    }

    virtual ~EchoSession() {
        DebugL;
    }

    void onRecv(const Buffer::Ptr &buffer) override {
        send(buffer);
    }

    void onError(const SockException &err) override {
        WarnL << err.what();
    }

    void onManager() override {}
};

//����(http)
class CMD_pingpong : public CMD {
public:
    CMD_pingpong() {
        _parser.reset(new OptionParser(nullptr));
        (*_parser) << Option('l', "listen", Option::ArgRequired, "10000", false, "������ģʽ�������˿�", nullptr);
        //���Կͻ��˸�����Ĭ��10��
        (*_parser) << Option('c', "count", Option::ArgRequired, to_string(10).data(), false, "�ͻ���ģʽ�����Կͻ��˸���", nullptr);
        //Ĭ��ÿ�η���1MB������
        (*_parser) << Option('b', "block", Option::ArgRequired, to_string(1024 * 1024).data(), false, "�ͻ���ģʽ���������ݿ��С",
                             nullptr);
        //Ĭ��1�뷢��10�Σ����ٶ���Ϊ1MB/s * 10 * 10 = 100MB/s
        (*_parser) << Option('i', "interval", Option::ArgRequired, to_string(100).data(), false, "�ͻ���ģʽ���������ݷ��ͼ������λ����",
                             nullptr);
        //�ͻ����������ʱ��
        (*_parser) << Option('d', "delay", Option::ArgRequired, "50", false, "������ģʽ���ͻ����������ʱ��", nullptr);

        //ָ����������ַ
        (*_parser) << Option('s', "server", Option::ArgRequired, "127.0.0.1:10000", false, "�ͻ���ģʽ�����Է�������ַ", []
                (const std::shared_ptr<ostream> &stream, const string &arg) {
            if (arg.find(":") == string::npos) {
                //�жϺ���ѡ��Ľ����Լ�������ϻص��Ȳ���
                throw std::runtime_error("\t��ַ����ָ���˿ں�.");
            }
            //�������false����Ժ���ѡ��Ľ���
            return true;
        });
    }

    ~CMD_pingpong() {}

    const char *description() const override {
        return "tcp�������ܲ���";
    }
};


EventPoller::Ptr nextPoller() {
    static vector<EventPoller::Ptr> s_poller_vec;
    static int s_poller_index = 0;
    if (s_poller_vec.empty()) {
        EventPollerPool::Instance().for_each([&](const TaskExecutor::Ptr &executor) {
            s_poller_vec.emplace_back(dynamic_pointer_cast<EventPoller>(executor));
        });
    }
    auto ret = s_poller_vec[s_poller_index++];
    if (s_poller_index == s_poller_vec.size()) {
        s_poller_index = 0;
    }
    return ret;
}

int main(int argc, char *argv[]) {
    CMD_pingpong cmd;
    try {
        cmd(argc, argv);
    } catch (std::exception &ex) {
        cout << ex.what() << endl;
        return 0;
    }

    //��ʼ������
    Logger::Instance().add(std::shared_ptr<ConsoleChannel>(new ConsoleChannel()));
    Logger::Instance().setWriter(std::shared_ptr<LogWriter>(new AsyncLogWriter()));

    {
        int interval = cmd["interval"];
        int block = cmd["block"];
        auto ip = cmd.splitedVal("server")[0];
        int port = cmd.splitedVal("server")[1];
        int delay = cmd["delay"];
        auto buffer = BufferRaw::create();
        buffer->setCapacity(block);
        buffer->setSize(block);

        TcpServer::Ptr server(new TcpServer);
        server->start<EchoSession>(cmd["listen"]);
        for (auto i = 0; i < cmd["count"].as<int>(); ++i) {
            auto poller = nextPoller();
            auto socket = Socket::createSocket(poller, false);

            socket->connect(ip, port, [socket, poller, interval, buffer](const SockException &err) {
                if (err) {
                    WarnL << err.what();
                    return;
                }
                socket->setOnErr([](const SockException &err) {
                    WarnL << err.what();
                });
                socket->setOnRead([interval, socket](const Buffer::Ptr &buffer, struct sockaddr *addr, int addr_len) {
                    if (!interval) {
                        socket->send(buffer);
                    }
                });

                if (interval) {
                    poller->doDelayTask(interval, [socket, interval, buffer]() {
                        socket->send(buffer);
                        return interval;
                    });
                } else {
                    socket->send(buffer);
                }
            });
            usleep(delay * 1000);
        }

        //�����˳��źŴ�����
        static semaphore sem;
        signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
        sem.wait();
    }
    return 0;
}