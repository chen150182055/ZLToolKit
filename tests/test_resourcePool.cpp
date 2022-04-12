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
#include <random>
#include "Util/util.h"
#include "Util/logger.h"
#include "Util/ResourcePool.h"
#include "Thread/threadgroup.h"
#include <list>

using namespace std;
using namespace toolkit;

//�����˳���־
bool g_bExitFlag = false;


class string_imp : public string {
public:
    template<typename ...ArgTypes>
    string_imp(ArgTypes &&...args) : string(std::forward<ArgTypes>(args)...) {
        DebugL << "����string����:" << this << " " << *this;
    };

    ~string_imp() {
        WarnL << "����string����:" << this << " " << *this;
    }
};


//��̨�߳�����
void onRun(ResourcePool<string_imp> &pool, int threadNum) {
    std::random_device rd;
    while (!g_bExitFlag) {
        //��ѭ���ػ�ȡһ�����õĶ���
        auto obj_ptr = pool.obtain();
        if (obj_ptr->empty()) {
            //���������ȫ��δʹ�õ�
            InfoL << "��̨�߳� " << threadNum << ":" << "obtain a emptry object!";
        } else {
            //���������ѭ��ʹ�õ�
            InfoL << "��̨�߳� " << threadNum << ":" << *obj_ptr;
        }
        //��Ǹö��󱻱��߳�ʹ��
        obj_ptr->assign(StrPrinter << "keeped by thread:" << threadNum);

        //������ߣ�����ѭ��ʹ��˳��
        usleep(1000 * (rd() % 10));
        obj_ptr.reset();//�ֶ��ͷţ�Ҳ����ע�������롣����RAII��ԭ���ö���ᱻ�Զ��ͷŲ����½���ѭ���ж�
        usleep(1000 * (rd() % 1000));
    }
}

int main() {
    //��ʼ����־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    //��СΪ50��ѭ����
    ResourcePool<string_imp> pool;
    pool.setSize(50);

    //��ȡһ������,�ö��󽫱����̳߳��У����Ҳ��ᱻ��̨�̻߳�ȡ����ֵ
    auto reservedObj = pool.obtain();
    //�����̸߳�ֵ�ö���
    reservedObj->assign("This is a reserved object , and will never be used!");

    thread_group group;
    //����4����̨�̣߳���4���߳�ģ��ѭ���ص�ʹ�ó�����
    //������4���߳���ͬһʱ�����ͬʱ�ܹ�ռ��4������


    WarnL << "���̴߳�ӡ:" << "��ʼ���ԣ����߳��Ѿ���ȡ���Ķ���Ӧ�ò��ᱻ��̨�̻߳�ȡ��:" << *reservedObj;

    for (int i = 0; i < 4; ++i) {
        group.create_thread([i, &pool]() {
            onRun(pool, i);
        });
    }

    //�ȴ�3���ӣ���ʱѭ����������õĶ�����������ٶ���ʹ�ù�һ����
    sleep(3);

    //��������reservedObj���ѱ����̳߳��У���̨�߳��ǻ�ȡ�����ö����
    //������ֵӦ����δ������
    WarnL << "���̴߳�ӡ: �ö����ڱ����̳߳��У���ֵӦ�ñ��ֲ���:" << *reservedObj;

    //��ȡ�ö��������
    auto &objref = *reservedObj;

    //��ʽ�ͷŶ���,�ö������½���ѭ���жӣ���ʱ�ö���Ӧ�ûᱻ��̨�̳߳��в���ֵ
    reservedObj.reset();

    WarnL << "���̴߳�ӡ: �Ѿ��ͷŸö���,��Ӧ�ûᱻ��̨�̻߳�ȡ����������ֵ";

    //������3�룬��reservedObj����̨�߳�ѭ��ʹ��
    sleep(3);

    //��ʱ��reservedObj����ѭ�����ڣ�����Ӧ�û�����Ч�ģ�����ֵӦ�ñ�������
    WarnL << "���̴߳�ӡ:�����ѱ���̨�̸߳�ֵΪ:" << objref << endl;

    {
        WarnL << "���̴߳�ӡ:��ʼ������������ѭ��ʹ�ù���";

        List<decltype(pool)::ValuePtr> objlist;
        for (int i = 0; i < 8; ++i) {
            reservedObj = pool.obtain();
            string str = StrPrinter << i << " " << (i % 2 == 0 ? "�˶�������ѭ���ع���" : "�˶��󽫻ص�ѭ����");
            reservedObj->assign(str);
            reservedObj.quit(i % 2 == 0);
            objlist.emplace_back(reservedObj);
        }
    }
    sleep(3);

    //֪ͨ��̨�߳��˳�
    g_bExitFlag = true;
    //�ȴ���̨�߳��˳�
    group.join_all();
    return 0;
}











