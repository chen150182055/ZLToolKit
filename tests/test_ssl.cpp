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
#include "Util/logger.h"
#include "Util/util.h"
#include "Util/SSLBox.h"

using namespace std;
using namespace toolkit;

int main(int argc, char *argv[]) {
    //��ʼ��������־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    //����֤�飬֤�������Կ��˽Կ
    SSL_Initor::Instance().loadCertificate((exeDir() + "ssl.p12").data());
    SSL_Initor::Instance().trustCertificate((exeDir() + "ssl.p12").data());
    SSL_Initor::Instance().ignoreInvalidCertificate(false);

    //����ͻ��˺ͷ����
    SSL_Box client(false), server(true);

    //���ÿͻ��˽�������ص�
    client.setOnDecData([&](const Buffer::Ptr &buffer) {
        //��ӡ���Է�������ݽ��ܺ������
        InfoL << "client recv:" << buffer->toString();
    });

    //���ÿͻ��˼�������ص�
    client.setOnEncData([&](const Buffer::Ptr &buffer) {
        //�ѿͻ��˼��ܺ�����ķ��͸������
        server.onRecv(buffer);
    });

    //���÷���˽�������ص�
    server.setOnDecData([&](const Buffer::Ptr &buffer) {
        //��ӡ���Կͻ������ݽ��ܺ������
        InfoL << "server recv:" << buffer->toString();
        //�����ݻ��Ը��ͻ���
        server.onSend(buffer);
    });

    //���÷���˼�������ص�
    server.setOnEncData([&](const Buffer::Ptr &buffer) {
        //�Ѽ��ܵĻ�����Ϣ�ظ����ͻ���;
        client.onRecv(buffer);
    });

    InfoL << "�������ַ���ʼ����,����quitֹͣ����:" << endl;

    string input;
    while (true) {
        std::cin >> input;
        if (input == "quit") {
            break;
        }
        //����������������ͻ���
        client.onSend(std::make_shared<BufferString>(std::move(input)));
    }
    return 0;
}
