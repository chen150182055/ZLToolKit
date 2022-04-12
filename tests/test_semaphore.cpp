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
#include <atomic>
#include "Util/TimeTicker.h"
#include "Util/logger.h"
#include "Thread/threadgroup.h"
#include "Thread/semaphore.h"

using namespace std;
using namespace toolkit;


#define MAX_TASK_SIZE (1000 * 10000)
semaphore g_sem;//�ź���
atomic_llong g_produced(0);
atomic_llong g_consumed(0);

//�������߳�
void onConsum() {
    while (true) {
        g_sem.wait();
        if (++g_consumed > g_produced) {
            //�����ӡ���log�������bug
            ErrorL << g_consumed << " > " << g_produced;
        }
    }
}

//�������߳�
void onProduce() {
    while (true) {
        ++g_produced;
        g_sem.post();
        if (g_produced >= MAX_TASK_SIZE) {
            break;
        }
    }
}

int main() {
    //��ʼ��log
    Logger::Instance().add(std::make_shared<ConsoleChannel>());

    Ticker ticker;
    thread_group thread_producer;
    for (size_t i = 0; i < thread::hardware_concurrency(); ++i) {
        thread_producer.create_thread([]() {
            //1���������߳�
            onProduce();
        });
    }

    thread_group thread_consumer;
    for (int i = 0; i < 4; ++i) {
        thread_consumer.create_thread([i]() {
            //4���������߳�
            onConsum();
        });
    }



    //�ȴ������������߳��˳�
    thread_producer.join_all();
    DebugL << "�������߳��˳�����ʱ:" << ticker.elapsedTime() << "ms," << "����������:" << g_produced << ",����������:" << g_consumed;

    int i = 5;
    while (--i) {
        DebugL << "�����˳�����ʱ:" << i << ",����������:" << g_consumed;
        sleep(1);
    }

    //����ǿ���˳�����core dump���ڳ����Ƴ�ʱ��������������Ӧ�ø�����������һ��
    WarnL << "ǿ�ƹر������̣߳����ܴ���core dump";
    return 0;
}
