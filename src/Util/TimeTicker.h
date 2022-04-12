/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef UTIL_TIMETICKER_H_
#define UTIL_TIMETICKER_H_

#include <cassert>
#include "logger.h"

namespace toolkit {

    class Ticker {
    public:
        /**
         * �˶���������ڴ���ִ��ʱ��ͳ�ƣ��Կ�������һ���ʱ
         * @param min_ms ������ִ��ʱ��ͳ��ʱ���������ִ�к�ʱ�����ò��������ӡ������־
         * @param ctx ��־�����Ĳ������ڲ���ǰ��־��������λ��
         * @param print_log �Ƿ��ӡ����ִ��ʱ��
         */
        Ticker(uint64_t min_ms = 0,LogContextCapture ctx = LogContextCapture(Logger::Instance(), LWarn, __FILE__, "", __LINE__),bool print_log = false) : _ctx(std::move(ctx)) {    //���캯��
            if (!print_log) {
                _ctx.clear();
            }
            _created = _begin = getCurrentMillisecond();
            _min_ms = min_ms;
        }

        ~Ticker() {     //��������
            uint64_t tm = createdTime();
            if (tm > _min_ms) {
                _ctx << "take time:" << tm << "ms" << ", thread may be overloaded";
            } else {
                _ctx.clear();
            }
        }

        /**
         * ��ȡ����ʱ�䣬��λ����
         */
        uint64_t elapsedTime() const {
            return getCurrentMillisecond() - _begin;
        }

        /**
         * ��ȡ�ϴ�resetTime�������ʱ�䣬��λ����
         */
        uint64_t createdTime() const {
            return getCurrentMillisecond() - _created;
        }

        /**
         * ���ü�ʱ��
         */
        void resetTime() {
            _begin = getCurrentMillisecond();
        }

    private:
        uint64_t _min_ms;
        uint64_t _begin;
        uint64_t _created;
        LogContextCapture _ctx;
    };

    class SmoothTicker {
    public:
        /**
         * �˶�����������ƽ����ʱ���
         * @param reset_ms ʱ������ü����û���reset_ms����, ���ɵ�ʱ�����ͬ��һ��ϵͳʱ���
         */
        SmoothTicker(uint64_t reset_ms = 10000) {     //���캯��
            _reset_ms = reset_ms;
            _ticker.resetTime();
        }

        ~SmoothTicker() {}          //��������

        /**
         * ����ƽ����ʱ�������ֹ�������綶������ʱ�����ƽ��
         */
        uint64_t elapsedTime() {
            auto now_time = _ticker.elapsedTime();
            if (_first_time == 0) {
                if (now_time < _last_time) {
                    auto last_time = _last_time - _time_inc;
                    double elapse_time = (now_time - last_time);
                    _time_inc += (elapse_time / ++_pkt_count) / 3;
                    auto ret_time = last_time + _time_inc;
                    _last_time = (uint64_t) ret_time;
                    return (uint64_t) ret_time;
                }
                _first_time = now_time;
                _last_time = now_time;
                _pkt_count = 0;
                _time_inc = 0;
                return now_time;
            }

            auto elapse_time = (now_time - _first_time);
            _time_inc += elapse_time / ++_pkt_count;
            auto ret_time = _first_time + _time_inc;
            if (elapse_time > _reset_ms) {
                _first_time = 0;
            }
            _last_time = (uint64_t) ret_time;
            return (uint64_t) ret_time;
        }

        /**
         * ʱ�������Ϊ0��ʼ
         */
        void resetTime() {
            _first_time = 0;
            _pkt_count = 0;
            _ticker.resetTime();
        }

    private:
        double _time_inc = 0;
        uint64_t _first_time = 0;
        uint64_t _last_time = 0;
        uint64_t _pkt_count = 0;
        uint64_t _reset_ms;
        Ticker _ticker;
    };

#if !defined(NDEBUG)
#define TimeTicker() Ticker __ticker(5,WarnL,true)
#define TimeTicker1(tm) Ticker __ticker1(tm,WarnL,true)
#define TimeTicker2(tm, log) Ticker __ticker2(tm,log,true)
#else
#define TimeTicker()
#define TimeTicker1(tm)
#define TimeTicker2(tm,log)
#endif

} /* namespace toolkit */
#endif /* UTIL_TIMETICKER_H_ */
