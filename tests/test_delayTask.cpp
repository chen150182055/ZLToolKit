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
#include "Util/TimeTicker.h"
#include "Util/onceToken.h"
#include "Poller/EventPoller.h"

using namespace std;
using namespace toolkit;

int main() {
    //������־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    Ticker ticker0;
    int nextDelay0 = 50;
    std::shared_ptr<onceToken> token0 = std::make_shared<onceToken>(nullptr, []() {
        TraceL << "task 0 ��ȡ�����������������ͷ�lambad���ʽ����ı���!";
    });
    auto tag0 = EventPollerPool::Instance().getPoller()->doDelayTask(nextDelay0, [&, token0]() {
        TraceL << "task 0(�̶���ʱ�ظ�����),Ԥ������ʱ�� :" << nextDelay0 << " ʵ������ʱ��" << ticker0.elapsedTime();
        ticker0.resetTime();
        return nextDelay0;
    });
    token0 = nullptr;

    Ticker ticker1;
    int nextDelay1 = 50;
    auto tag1 = EventPollerPool::Instance().getPoller()->doDelayTask(nextDelay1, [&]() {
        DebugL << "task 1(�ɱ���ʱ�ظ�����),Ԥ������ʱ�� :" << nextDelay1 << " ʵ������ʱ��" << ticker1.elapsedTime();
        ticker1.resetTime();
        nextDelay1 += 1;
        return nextDelay1;
    });

    Ticker ticker2;
    int nextDelay2 = 3000;
    auto tag2 = EventPollerPool::Instance().getPoller()->doDelayTask(nextDelay2, [&]() {
        InfoL << "task 2(������ʱ����),Ԥ������ʱ�� :" << nextDelay2 << " ʵ������ʱ��" << ticker2.elapsedTime();
        return 0;
    });

    Ticker ticker3;
    int nextDelay3 = 50;
    auto tag3 = EventPollerPool::Instance().getPoller()->doDelayTask(nextDelay3, [&]() -> uint64_t {
        throw std::runtime_error("task 2(������ʱ���������쳣,�����²��ټ�������ʱ����)");
    });


    sleep(2);
    tag0->cancel();
    tag1->cancel();
    WarnL << "ȡ��task 0��1";

    //�˳������¼�����
    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
    sem.wait();
    return 0;
}
