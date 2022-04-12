/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef UTIL_LOGGER_H_
#define UTIL_LOGGER_H_

#include <set>
#include <map>
#include <fstream>
#include <thread>
#include <memory>
#include <mutex>
#include "util.h"
#include "List.h"
#include "Thread/semaphore.h"

namespace toolkit {

    class LogContext;

    class LogChannel;

    class LogWriter;

    class Logger;

    using LogContextPtr = std::shared_ptr<LogContext>;

    typedef enum {
        LTrace = 0, LDebug, LInfo, LWarn, LError
    } LogLevel;

    Logger &getLogger();

    void setLogger(Logger *logger);

/**
* ��־��
*/
    class Logger : public std::enable_shared_from_this<Logger>, public noncopyable {
    public:
        friend class AsyncLogWriter;

        using Ptr = std::shared_ptr<Logger>;

        /**
         * ��ȡ��־����
         * @return
         */
        static Logger &Instance();

        explicit Logger(const std::string &loggerName);

        ~Logger();

        /**
         * �����־ͨ�������̰߳�ȫ��
         * @param channel logͨ��
         */
        void add(const std::shared_ptr<LogChannel> &channel);

        /**
         * ɾ����־ͨ�������̰߳�ȫ��
         * @param name logͨ����
         */
        void del(const std::string &name);

        /**
         * ��ȡ��־ͨ�������̰߳�ȫ��
         * @param name logͨ����
         * @return �߳�ͨ��
         */
        std::shared_ptr<LogChannel> get(const std::string &name);

        /**
         * ����дlog�������̰߳�ȫ��
         * @param writer дlog��
         */
        void setWriter(const std::shared_ptr<LogWriter> &writer);

        /**
         * ����������־ͨ����log�ȼ�
         * @param level log�ȼ�
         */
        void setLevel(LogLevel level);

        /**
         * ��ȡlogger��
         * @return logger��
         */
        const std::string &getName() const;

        /**
         * д��־
         * @param ctx ��־��Ϣ
         */
        void write(const LogContextPtr &ctx);

    private:
        /**
         * д��־����channel������AsyncLogWriter����
         * @param ctx ��־��Ϣ
         */
        void writeChannels(const LogContextPtr &ctx);

        void writeChannels_l(const LogContextPtr &ctx);

    private:
        LogContextPtr _last_log;
        std::string _logger_name;
        std::shared_ptr<LogWriter> _writer;
        std::map<std::string, std::shared_ptr<LogChannel> > _channels;
    };

///////////////////LogContext///////////////////
/**
* ��־������
*/
    class LogContext : public std::ostringstream {
    public:
        //_file,_function�ĳ�string���棬Ŀ������Щ����£�ָ����ܻ�ʧЧ
        //����˵��̬���д�ӡ��һ����־��Ȼ��̬��ж���ˣ���ôָ��̬��������ָ��ͻ�ʧЧ
        LogContext() = default;

        LogContext(LogLevel level, const char *file, const char *function, int line, const char *module_name);

        ~LogContext() = default;

        LogLevel _level;
        int _line;
        int _repeat = 0;
        std::string _file;
        std::string _function;
        std::string _thread_name;
        std::string _module_name;
        struct timeval _tv;

        const std::string &str();

    private:
        bool _got_content = false;
        std::string _content;
    };

/**
 * ��־�����Ĳ�����
 */
    class LogContextCapture {
    public:
        using Ptr = std::shared_ptr<LogContextCapture>;

        LogContextCapture(Logger &logger, LogLevel level, const char *file, const char *function, int line);

        LogContextCapture(const LogContextCapture &that);

        ~LogContextCapture();

        /**
         * ����std::endl(�س���)���������־
         * @param f std::endl(�س���)
         * @return ��������
         */
        LogContextCapture &operator<<(std::ostream &(*f)(std::ostream &));

        template<typename T>
        LogContextCapture &operator<<(T &&data) {
            if (!_ctx) {
                return *this;
            }
            (*_ctx) << std::forward<T>(data);
            return *this;
        }

        void clear();

    private:
        LogContextPtr _ctx;
        Logger &_logger;
    };


///////////////////LogWriter///////////////////
/**
 * д��־��
 */
    class LogWriter : public noncopyable {
    public:
        LogWriter() = default;

        virtual ~LogWriter() = default;

        virtual void write(const LogContextPtr &ctx, Logger &logger) = 0;
    };

    class AsyncLogWriter : public LogWriter {
    public:
        AsyncLogWriter();

        ~AsyncLogWriter();

    private:
        void run();

        void flushAll();

        void write(const LogContextPtr &ctx, Logger &logger) override;

    private:
        bool _exit_flag;
        semaphore _sem;
        std::mutex _mutex;
        std::shared_ptr<std::thread> _thread;
        List<std::pair<LogContextPtr, Logger *> > _pending;
    };

///////////////////LogChannel///////////////////
/**
 * ��־ͨ��
 */
    class LogChannel : public noncopyable {
    public:
        LogChannel(const std::string &name, LogLevel level = LTrace);

        virtual ~LogChannel();

        virtual void write(const Logger &logger, const LogContextPtr &ctx) = 0;

        const std::string &name() const;

        void setLevel(LogLevel level);

        static std::string printTime(const timeval &tv);

    protected:
        /**
        * ��ӡ��־�������
        * @param ost �����
        * @param enable_color �Ƿ�������ɫ
        * @param enable_detail �Ƿ��ӡϸ��(��������Դ���ļ�����Դ����)
        */
        virtual void format(const Logger &logger, std::ostream &ost, const LogContextPtr &ctx, bool enable_color = true,
                            bool enable_detail = true);

    protected:
        std::string _name;
        LogLevel _level;
    };

/**
 * ����������㲥
 */
    class EventChannel : public LogChannel {
    public:
        //�����־ʱ�Ĺ㲥��
        static const std::string kBroadcastLogEvent;
        //��־�㲥�������ͺ��б�
#define BroadcastLogEventArgs const Logger &logger, const LogContextPtr &ctx

        EventChannel(const std::string &name = "EventChannel", LogLevel level = LTrace);

        ~EventChannel() override = default;

        void write(const Logger &logger, const LogContextPtr &ctx) override;
    };

/**
 * �����־���նˣ�֧�������־��android logcat
 */
    class ConsoleChannel : public LogChannel {
    public:
        ConsoleChannel(const std::string &name = "ConsoleChannel", LogLevel level = LTrace);

        ~ConsoleChannel() override = default;

        void write(const Logger &logger, const LogContextPtr &logContext) override;
    };

/**
 * �����־���ļ�
 */
    class FileChannelBase : public LogChannel {
    public:
        FileChannelBase(const std::string &name = "FileChannelBase", const std::string &path = exePath() + ".log",
                        LogLevel level = LTrace);

        ~FileChannelBase() override;

        void write(const Logger &logger, const LogContextPtr &ctx) override;

        bool setPath(const std::string &path);

        const std::string &path() const;

    protected:
        virtual bool open();

        virtual void close();

        virtual size_t size();

    protected:
        std::string _path;
        std::ofstream _fstream;
    };

    class Ticker;

/**
 * �Զ��������־�ļ�ͨ��
 * Ĭ����ౣ��30�����־
 */
    class FileChannel : public FileChannelBase {
    public:
        FileChannel(const std::string &name = "FileChannel", const std::string &dir = exeDir() + "log/",
                    LogLevel level = LTrace);

        ~FileChannel() override = default;

        /**
         * д��־ʱ�Żᴥ���½���־�ļ�����ɾ���ϵ���־�ļ�
         * @param logger
         * @param stream
         */
        void write(const Logger &logger, const LogContextPtr &ctx) override;

        /**
         * ������־��󱣴�����
         * @param max_day ����
         */
        void setMaxDay(size_t max_day);

        /**
         * ������־��Ƭ�ļ�����С
         * @param max_size ��λMB
         */
        void setFileMaxSize(size_t max_size);

        /**
         * ������־��Ƭ�ļ�������
         * @param max_count ����
         */
        void setFileMaxCount(size_t max_count);

    private:
        /**
         * ɾ����־��Ƭ�ļ�������Ϊ������󱣴������������Ƭ����
         */
        void clean();

        /**
         * ��鵱ǰ��־��Ƭ�ļ���С������������ƣ��򴴽��µ���־��Ƭ�ļ�
         */
        void checkSize(time_t second);

        /**
         * �������л�����һ����־��Ƭ�ļ�
         */
        void changeFile(time_t second);

    private:
        bool _can_write = false;
        //Ĭ����ౣ��30�����־�ļ�
        size_t _log_max_day = 30;
        //ÿ����־��Ƭ�ļ����Ĭ��128MB
        size_t _log_max_size = 128;
        //���Ĭ�ϱ���30����־��Ƭ�ļ�
        size_t _log_max_count = 30;
        //��ǰ��־��Ƭ�ļ�����
        size_t _index = 0;
        int64_t _last_day = -1;
        time_t _last_check_time = 0;
        std::string _dir;
        std::set<std::string> _log_file_map;
    };

#if defined(__MACH__) || ((defined(__linux) || defined(__linux__)) && !defined(ANDROID))
    class SysLogChannel : public LogChannel {
    public:
        SysLogChannel(const std::string &name = "SysLogChannel", LogLevel level = LTrace);
        ~SysLogChannel() override = default;

        void write(const Logger &logger, const LogContextPtr &logContext) override;
    };

#endif//#if defined(__MACH__) || ((defined(__linux) || defined(__linux__)) &&  !defined(ANDROID))

    class LoggerWrapper {
    public:
        template<typename First, typename ...ARGS>
        static inline void
        printLogArray(Logger &logger, LogLevel level, const char *file, const char *function, int line, First &&first,
                      ARGS &&...args) {
            LogContextCapture log(logger, level, file, function, line);
            log << std::forward<First>(first);
            appendLog(log, std::forward<ARGS>(args)...);
        }

        static inline void
        printLogArray(Logger &logger, LogLevel level, const char *file, const char *function, int line) {
            LogContextCapture log(logger, level, file, function, line);
        }

        template<typename Log, typename First, typename ...ARGS>
        static inline void appendLog(Log &out, First &&first, ARGS &&...args) {
            out << std::forward<First>(first);
            appendLog(out, std::forward<ARGS>(args)...);
        }

        template<typename Log>
        static inline void appendLog(Log &out) {}

        //printf��ʽ����־��ӡ
        static void
        printLog(Logger &logger, int level, const char *file, const char *function, int line, const char *fmt, ...);

        static void
        printLogV(Logger &logger, int level, const char *file, const char *function, int line, const char *fmt,
                  va_list ap);
    };

//������Ĭ��ֵ
    extern Logger *g_defaultLogger;

//�÷�: DebugL << 1 << "+" << 2 << '=' << 3;
#define WriteL(level) ::toolkit::LogContextCapture(::toolkit::getLogger(), level, __FILE__, __FUNCTION__, __LINE__)
#define TraceL WriteL(::toolkit::LTrace)
#define DebugL WriteL(::toolkit::LDebug)
#define InfoL WriteL(::toolkit::LInfo)
#define WarnL WriteL(::toolkit::LWarn)
#define ErrorL WriteL(::toolkit::LError)

//�÷�: LogD("%d + %s = %c", 1 "2", 'c');
#define PrintLog(level, ...) ::toolkit::LoggerWrapper::printLog(::toolkit::getLogger(), level, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define PrintT(...) PrintLog(::toolkit::LTrace, ##__VA_ARGS__)
#define PrintD(...) PrintLog(::toolkit::LDebug, ##__VA_ARGS__)
#define PrintI(...) PrintLog(::toolkit::LInfo, ##__VA_ARGS__)
#define PrintW(...) PrintLog(::toolkit::LWarn, ##__VA_ARGS__)
#define PrintE(...) PrintLog(::toolkit::LError, ##__VA_ARGS__)

//�÷�: LogD(1, "+", "2", '=', 3);
//����ģ��ʵ������ԭ�����ÿ�δ�ӡ�������������Ͳ�һ�£����ܻᵼ�¶����ƴ�������
#define LogL(level, ...) ::toolkit::LoggerWrapper::printLogArray(::toolkit::getLogger(), (LogLevel)level, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LogT(...) LogL(::toolkit::LTrace, ##__VA_ARGS__)
#define LogD(...) LogL(::toolkit::LDebug, ##__VA_ARGS__)
#define LogI(...) LogL(::toolkit::LInfo, ##__VA_ARGS__)
#define LogW(...) LogL(::toolkit::LWarn, ##__VA_ARGS__)
#define LogE(...) LogL(::toolkit::LError, ##__VA_ARGS__)

} /* namespace toolkit */
#endif /* UTIL_LOGGER_H_ */
