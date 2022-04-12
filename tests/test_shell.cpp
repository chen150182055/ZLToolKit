/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <iostream>
#include "Util/CMD.h"
#include "Util/logger.h"
#include "Util/util.h"
#include "Network/TcpClient.h"
#include <csignal>

using namespace std;
using namespace toolkit;

class TestClient : public TcpClient {
public:
    using Ptr = std::shared_ptr<TestClient>;

    TestClient() : TcpClient() {}

    ~TestClient() {}

    void connect(const string &strUrl, uint16_t iPort, float fTimeoutSec) {
        startConnect(strUrl, iPort, fTimeoutSec);
    }

    void disconnect() {
        shutdown();
    }

    size_t commit(const string &method, const string &path, const string &host) {
        string strGet = StrPrinter
                << method
                << " "
                << path
                << " HTTP/1.1\r\n"
                << "Host: " << host << "\r\n"
                << "Connection: keep-alive\r\n"
                << "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_1) "
                   "AppleWebKit/537.36 (KHTML, like Gecko) "
                   "Chrome/58.0.3029.110 Safari/537.36\r\n"
                << "Accept-Encoding: gzip, deflate, sdch\r\n"
                << "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n\r\n";
        DebugL << "\r\n" << strGet;
        return SockSender::send(strGet);
    }

protected:
    virtual void onConnect(const SockException &ex) override {
        //���ӽ���¼�
        InfoL << (ex ? ex.what() : "success");
    }

    virtual void onRecv(const Buffer::Ptr &pBuf) override {
        //���������¼�
        DebugL << pBuf->data();
    }

    virtual void onFlush() override {
        //���������󣬻�������¼�
        DebugL;
    }

    virtual void onErr(const SockException &ex) override {
        //�Ͽ������¼���һ����EOF
        WarnL << ex.what();
    }
};

//����(http)
class CMD_http : public CMD {
public:
    CMD_http() {
        _client.reset(new TestClient);
        _parser.reset(new OptionParser([this](const std::shared_ptr<ostream> &stream, mINI &args) {
            //����ѡ�������Ϻ󴥷��ûص������ǿ�����������һЩȫ�ֵĲ���
            if (hasKey("connect")) {
                //�������Ӳ���
                connect(stream);
                return;
            }
            if (hasKey("commit")) {
                commit(stream);
                return;
            }
        }));

        (*_parser) << Option('T', "type", Option::ArgRequired, nullptr, true, "Ӧ�ó���ģʽ��0����ͳģʽ��1��shellģʽ", nullptr);

        (*_parser) << Option('s',/*��ѡ���ƣ������\x00��˵���޼��*/
                             "server",/*��ѡ��ȫ��,ÿ��ѡ�������ȫ�ƣ�����Ϊnull����ַ���*/
                             Option::ArgRequired,/*��ѡ���������ֵ*/
                             "www.baidu.com:80",/*��ѡ��Ĭ��ֵ*/
                             false,/*��ѡ���Ƿ���븳ֵ�����û��Ĭ��ֵ��ΪArgRequiredʱ�û������ṩ�ò����������쳣*/
                             "tcp��������ַ����ð�ŷָ��˿ں�",/*��ѡ��˵������*/
                             [this](const std::shared_ptr<ostream> &stream, const string &arg) {/*��������ѡ��Ļص�*/
                                 if (arg.find(":") == string::npos) {
                                     //�жϺ���ѡ��Ľ����Լ�������ϻص��Ȳ���
                                     throw std::runtime_error("\t��ַ����ָ���˿ں�.");
                                 }
                                 //�������false����Ժ���ѡ��Ľ���
                                 return true;
                             });

        (*_parser) << Option('d', "disconnect", Option::ArgNone, nullptr, false, "�Ƿ�Ͽ�����",
                             [this](const std::shared_ptr<ostream> &stream, const string &arg) {
                                 //�Ͽ����Ӳ��������Ժ����Ĳ������Ƕ���������
                                 disconnect(stream);
                                 return false;
                             });

        (*_parser) << Option('c', "connect", Option::ArgNone, nullptr, false, "����tcp connect����", nullptr);
        (*_parser) << Option('t', "time_out", Option::ArgRequired, "3", false, "���ӳ�ʱ��", nullptr);
        (*_parser) << Option('m', "method", Option::ArgRequired, "GET", false, "HTTP����,Ʃ��GET��POST", nullptr);
        (*_parser) << Option('p', "path", Option::ArgRequired, "/index.html", false, "HTTP url·��", nullptr);
        (*_parser) << Option('C', "commit", Option::ArgNone, nullptr, false, "�ύHTTP����", nullptr);


    }

    ~CMD_http() {}

    const char *description() const override {
        return "http���Կͻ���";
    }

private:
    void connect(const std::shared_ptr<ostream> &stream) {
        (*stream) << "connect����" << endl;
        _client->connect(splitedVal("server")[0], splitedVal("server")[1], (*this)["time_out"]);
    }

    void disconnect(const std::shared_ptr<ostream> &stream) {
        (*stream) << "disconnect����" << endl;
        _client->disconnect();
    }

    void commit(const std::shared_ptr<ostream> &stream) {
        (*stream) << "commit����" << endl;
        _client->commit((*this)["method"], (*this)["path"], (*this)["server"]);
    }

private:
    TestClient::Ptr _client;
};


int main(int argc, char *argv[]) {
    REGIST_CMD(http);
    signal(SIGINT, [](int) {
        exit(0);
    });
    try {
        CMD_DO("http", argc, argv);
    } catch (std::exception &ex) {
        cout << ex.what() << endl;
        return 0;
    }
    if (GET_CMD("http")["type"] == 0) {
        cout << "��ͳģʽ�����˳������볢��shellģʽ" << endl;
        return 0;
    }
    GET_CMD("http").delOption("type");
    //��ʼ������
    Logger::Instance().add(std::shared_ptr<ConsoleChannel>(new ConsoleChannel()));
    Logger::Instance().setWriter(std::shared_ptr<LogWriter>(new AsyncLogWriter()));

    cout << "> ��ӭ��������ģʽ�����������\"help\"�����ȡ����" << endl;
    string cmd_line;
    while (cin.good()) {
        try {
            cout << "> ";
            getline(cin, cmd_line);
            CMDRegister::Instance()(cmd_line);
        } catch (ExitException &) {
            break;
        } catch (std::exception &ex) {
            cout << ex.what() << endl;
        }
    }
    return 0;
}