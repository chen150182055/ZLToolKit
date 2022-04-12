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
#include "Util/util.h"
#include "Util/logger.h"
#include "Util/NoticeCenter.h"

using namespace std;
using namespace toolkit;

//�㲥����1
#define NOTICE_NAME1 "NOTICE_NAME1"
//�㲥����2
#define NOTICE_NAME2 "NOTICE_NAME2"

//�����˳����
bool g_bExitFlag = false;


int main() {
    //���ó����˳��źŴ�����
    signal(SIGINT, [](int) { g_bExitFlag = true; });
    //������־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());

    //���¼�NOTICE_NAME1����һ������
    //addListener������һ�������Ǳ�ǩ������ɾ������ʱʹ��
    //��Ҫע����Ǽ����ص��Ĳ����б����������Ҫ��emitEvent�㲥ʱ����ȫһ�£���������޷�Ԥ֪�Ĵ���
    NoticeCenter::Instance().addListener(0, NOTICE_NAME1,
                                         [](int &a, const char *&b, double &c, string &d) {
                                             DebugL << a << " " << b << " " << c << " " << d;
                                             NoticeCenter::Instance().delListener(0, NOTICE_NAME1);

                                             NoticeCenter::Instance().addListener(0, NOTICE_NAME1,
                                                                                  [](int &a, const char *&b, double &c,
                                                                                     string &d) {
                                                                                      InfoL << a << " " << b << " " << c
                                                                                            << " " << d;
                                                                                  });
                                         });

    //����NOTICE_NAME2�¼�
    NoticeCenter::Instance().addListener(0, NOTICE_NAME2, [](string &d, double &c, const char *&b, int &a) {
        DebugL << a << " " << b << " " << c << " " << d;
        NoticeCenter::Instance().delListener(0, NOTICE_NAME2);

        NoticeCenter::Instance().addListener(0, NOTICE_NAME2,
                                             [](string &d, double &c,
                                                const char *&b, int &a) {
                                                 WarnL << a << " " << b << " " << c
                                                       << " " << d;
                                             });

    });
    int a = 0;
    while (!g_bExitFlag) {
        const char *b = "b";
        double c = 3.14;
        string d("d");
        //ÿ��1��㲥һ���¼�������޷�ȷ���������ͣ��ɼ�ǿ��ת��
        NoticeCenter::Instance().emitEvent(NOTICE_NAME1, ++a, (const char *) "b", c, d);
        NoticeCenter::Instance().emitEvent(NOTICE_NAME2, d, c, b, a);
        sleep(1); // sleep 1 second
    }
    return 0;
}
