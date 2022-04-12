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
#include "Poller/Timer.h"

using namespace std;
using namespace toolkit;

int main() {
    //������־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());


    Ticker ticker0;
    Timer::Ptr timer0 = std::make_shared<Timer>(0.5f, [&]() {
        TraceL << "timer0�ظ�:" << ticker0.elapsedTime();
        ticker0.resetTime();
        return true;
    }, nullptr);

    Timer::Ptr timer1 = std::make_shared<Timer>(1.0f, []() {
        DebugL << "timer1�����ظ�";
        return false;
    }, nullptr);

    Ticker ticker2;
    Timer::Ptr timer2 = std::make_shared<Timer>(2.0f, [&]() -> bool {
        InfoL << "timer2,�������������쳣" << ticker2.elapsedTime();
        ticker2.resetTime();
        throw std::runtime_error("timer2,�������������쳣");
    }, nullptr);

    //�˳������¼�����
    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); });// �����˳��ź�
    sem.wait();
    return 0;
}
