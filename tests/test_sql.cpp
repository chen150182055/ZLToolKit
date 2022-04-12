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

#if defined(ENABLE_MYSQL)

#include "Util/SqlPool.h"

#endif
using namespace std;
using namespace toolkit;

int main() {
    //��ʼ����־
    Logger::Instance().add(std::make_shared<ConsoleChannel>());

#if defined(ENABLE_MYSQL)
    /*
     * ���Է���:
     * �밴��ʵ�����ݿ�����޸�Դ��Ȼ�����ִ�в���
     */
    string sql_ip = "127.0.0.1";
    unsigned short sql_port = 3306;
    string user = "root";
    string password = "5201314";
    string character = "utf8mb4";

#if defined(SUPPORT_DYNAMIC_TEMPLATE)
    //��ʼ������
    SqlPool::Instance().Init(sql_ip, sql_port, "", user, password/*,character*/);
#else
    //������Ҫ�������Կɱ����ģ���֧�֣�����gcc5.0����һ�㶼��֧�֣�������뱨��
    ErrorL << "your compiler does not support variable parameter templates!" << endl;
    return -1;
#endif //defined(SUPPORT_DYNAMIC_TEMPLATE)

    //�������ݿ����ӳش�С���ø�CPU����һ��(��һ��Ϊ��)
    SqlPool::Instance().setSize(3 + thread::hardware_concurrency());

    vector<vector<string> > sqlRet;
    SqlWriter("create database test_db;", false) << sqlRet;
    SqlWriter(
            "create table test_db.test_table(user_name  varchar(128),user_id int auto_increment primary key,user_pwd varchar(128));",
            false) << sqlRet;

    //ͬ������
    SqlWriter insertSql("insert into test_db.test_table(user_name,user_pwd) values('?','?');");
    insertSql << "zltoolkit" << "123456" << sqlRet;
    //���ǿ���֪�������˼������ݣ����ҿ��Ի�ȡ�²���(��һ��)���ݵ�rowID
    DebugL << "AffectedRows:" << insertSql.getAffectedRows() << ",RowID:" << insertSql.getRowID();

    //ͬ����ѯ
    SqlWriter sqlSelect("select user_id , user_pwd from test_db.test_table where user_name='?' limit 1;");
    sqlSelect << "zltoolkit";

    vector<vector<string> > sqlRet0;
    vector<list<string> > sqlRet1;
    vector<deque<string> > sqlRet2;
    vector<map<string, string> > sqlRet3;
    vector<unordered_map<string, string> > sqlRet4;
    sqlSelect << sqlRet0;
    sqlSelect << sqlRet1;
    sqlSelect << sqlRet2;
    sqlSelect << sqlRet3;
    sqlSelect << sqlRet4;

    for (auto &line: sqlRet0) {
        DebugL << "vector<string> user_id:" << line[0] << ",user_pwd:" << line[1];
    }
    for (auto &line: sqlRet1) {
        DebugL << "list<string> user_id:" << line.front() << ",user_pwd:" << line.back();
    }
    for (auto &line: sqlRet2) {
        DebugL << "deque<string> user_id:" << line[0] << ",user_pwd:" << line[1];
    }

    for (auto &line: sqlRet3) {
        DebugL << "map<string,string> user_id:" << line["user_id"] << ",user_pwd:" << line["user_pwd"];
    }

    for (auto &line: sqlRet4) {
        DebugL << "unordered_map<string,string> user_id:" << line["user_id"] << ",user_pwd:" << line["user_pwd"];
    }

    //�첽ɾ��
    SqlWriter insertDel("delete from test_db.test_table where user_name='?';");
    insertDel << "zltoolkit" << endl;

    //ע��!
    //���SqlWriter �� "<<" �������������SqlPool::SqlRetType���ͣ���˵����ͬ���������ȴ����
    //�������std::endl ,�����첽�������ں�̨�߳����sql������
#else
    ErrorL << "ENABLE_MYSQL not defined!" << endl;
    return -1;
#endif//ENABLE_MYSQL

    return 0;
}
