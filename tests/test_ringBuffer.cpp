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
#include "Util/util.h"
#include "Util/RingBuffer.h"
#include "Thread/threadgroup.h"
#include <list>

using namespace std;
using namespace toolkit;

//���λ���д�߳��˳����
bool g_bExitWrite = false;

//һ��30��string����Ļ��λ���
RingBuffer<string>::Ptr g_ringBuf(new RingBuffer<string>(30));

//д�¼��ص�����
void onReadEvent(const string &str) {
    //���¼�ģʽ��
    DebugL << str;
}

//���λ��������¼�
void onDetachEvent() {
    WarnL;
}

//д���λ�������
void doWrite() {
    int i = 0;
    while (!g_bExitWrite) {
        //ÿ��100msдһ�����ݵ����λ���
        g_ringBuf->write(to_string(++i), true);
        usleep(100 * 1000);
    }

}

int main() {
    //��ʼ����־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    auto poller = EventPollerPool::Instance().getPoller();
    RingBuffer<string>::RingReader::Ptr ringReader;
    poller->sync([&]() {
        //�ӻ��λ����ȡһ����ȡ��
        ringReader = g_ringBuf->attach(poller);

        //���ö�ȡ�¼�
        ringReader->setReadCB([](const string &pkt) {
            onReadEvent(pkt);
        });

        //���û��λ��������¼�
        ringReader->setDetachCB([]() {
            onDetachEvent();
        });
    });


    thread_group group;
    //д�߳�
    group.create_thread([]() {
        doWrite();
    });

    //����3����
    sleep(3);

    //֪ͨд�߳��˳�
    g_bExitWrite = true;
    //�ȴ�д�߳��˳�
    group.join_all();

    //�ͷŻ��λ��壬��ʱ�첽����Detach�¼�
    g_ringBuf.reset();
    //�ȴ��첽����Detach�¼�
    sleep(1);
    //������EventPoller���������
    ringReader.reset();
    sleep(1);
    return 0;
}











