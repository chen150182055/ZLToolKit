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
#include "Poller/EventPoller.h"
#include "Poller/Pipe.h"
#include "Util/util.h"

using namespace std;
using namespace toolkit;

int main() {
    //������־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
#if defined(_WIN32)
    ErrorL << "�ò��Գ�������windows�����У���Ϊ�Ҳ���windows�µĶ���̱�̣����ǹܵ�ģ���ǿ�����windows�����������ġ�" << endl;
#else
    //��ȡ�����̵�PID
    auto parentPid = getpid();
    InfoL << "parent pid:" << parentPid << endl;

    //����һ���ܵ���lambada���͵Ĳ����ǹܵ��յ����ݵĻص�
    Pipe pipe([](int size,const char *buf) {
        //�ùܵ������ݿɶ���
        InfoL << getpid() << " recv:" << buf;
    });

    //�����ӽ���
    auto pid = fork();

    if (pid == 0) {
        //�ӽ���
        int i = 10;
        while (i--) {
            //���ӽ���ÿ��һ�������д��ܵ������Ʒ���10��
            sleep(1);
            string msg = StrPrinter << "message " << i << " form subprocess:" << getpid();
            DebugL << "�ӽ��̷���:" << msg << endl;
            pipe.send(msg.data(), msg.size());
        }
        DebugL << "�ӽ����˳�" << endl;
    } else {
        //�����������˳��źŴ�����
        static semaphore sem;
        signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
        sem.wait();

        InfoL << "�������˳�" << endl;
    }
#endif // defined(_WIN32)

    return 0;
}
