/*
 * Copyright (c) 2016 The ZLToolKit project authors. All Rights Reserved.
 *
 * This file is part of ZLToolKit(https://github.com/ZLMediaKit/ZLToolKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLTOOLKIT_BUFFER_H
#define ZLTOOLKIT_BUFFER_H

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>
#include <functional>
#include "Util/util.h"
#include "Util/List.h"
#include "Util/ResourcePool.h"
#include "sockutil.h"

namespace toolkit {

    template<typename T>
    struct is_pointer : public std::false_type {
    };
    template<typename T>
    struct is_pointer<std::shared_ptr<T> > : public std::true_type {
    };
    template<typename T>
    struct is_pointer<std::shared_ptr<T const> > : public std::true_type {
    };
    template<typename T>
    struct is_pointer<T *> : public std::true_type {
    };
    template<typename T>
    struct is_pointer<const T *> : public std::true_type {
    };

//�������
    class Buffer : public noncopyable {
    public:
        using Ptr = std::shared_ptr<Buffer>;

        Buffer() = default;

        virtual ~Buffer() = default;

        //�������ݳ���
        virtual char *data() const = 0;

        virtual size_t size() const = 0;

        virtual std::string toString() const {
            return std::string(data(), size());
        }

        virtual size_t getCapacity() const {
            return size();
        }

    private:
        //�������ͳ��
        ObjectStatistic<Buffer> _statistic;
    };

    template<typename C>
    class BufferOffset : public Buffer {
    public:
        using Ptr = std::shared_ptr<BufferOffset>;

        BufferOffset(C data, size_t offset = 0, size_t len = 0) : _data(std::move(data)) {
            setup(offset, len);
        }

        ~BufferOffset() override = default;

        char *data() const override {
            return const_cast<char *>(getPointer<C>(_data)->data()) + _offset;
        }

        size_t size() const override {
            return _size;
        }

        std::string toString() const override {
            return std::string(data(), size());
        }

    private:
        void setup(size_t offset = 0, size_t size = 0) {
            auto max_size = getPointer<C>(_data)->size();
            assert(offset + size <= max_size);
            if (!size) {
                size = max_size - offset;
            }
            _size = size;
            _offset = offset;
        }

        template<typename T>
        static typename std::enable_if<::toolkit::is_pointer<T>::value, const T &>::type
        getPointer(const T &data) {
            return data;
        }

        template<typename T>
        static typename std::enable_if<!::toolkit::is_pointer<T>::value, const T *>::type
        getPointer(const T &data) {
            return &data;
        }

    private:
        C _data;
        size_t _size;
        size_t _offset;
    };

    using BufferString = BufferOffset<std::string>;

//ָ��ʽ�������
    class BufferRaw : public Buffer {
    public:
        using Ptr = std::shared_ptr<BufferRaw>;

        static Ptr create();

        ~BufferRaw() override {
            if (_data) {
                delete[] _data;
            }
        }

        //��д������ʱ��ȷ���ڴ��Ƿ�Խ��
        char *data() const override {
            return _data;
        }

        //��Ч���ݴ�С
        size_t size() const override {
            return _size;
        }

        //�����ڴ��С
        void setCapacity(size_t capacity) {
            if (_data) {
                do {
                    if (capacity > _capacity) {
                        //������ڴ���ڵ�ǰ�ڴ棬��ô���·���
                        break;
                    }

                    if (_capacity < 2 * 1024) {
                        //2K���£����ظ������ڴ棬ֱ�Ӹ���
                        return;
                    }

                    if (2 * capacity > _capacity) {
                        //���������ڴ���ڵ�ǰ�ڴ��һ�룬��ôҲ����
                        return;
                    }
                } while (false);

                delete[] _data;
            }
            _data = new char[capacity];
            _capacity = capacity;
        }

        //������Ч���ݴ�С
        virtual void setSize(size_t size) {
            if (size > _capacity) {
                throw std::invalid_argument("Buffer::setSize out of range");
            }
            _size = size;
        }

        //��ֵ����
        void assign(const char *data, size_t size = 0) {
            if (size <= 0) {
                size = strlen(data);
            }
            setCapacity(size + 1);
            memcpy(_data, data, size);
            _data[size] = '\0';
            setSize(size);
        }

        size_t getCapacity() const override {
            return _capacity;
        }

    protected:
        friend class ResourcePool_l<BufferRaw>;

        BufferRaw(size_t capacity = 0) {
            if (capacity) {
                setCapacity(capacity);
            }
        }

        BufferRaw(const char *data, size_t size = 0) {
            assign(data, size);
        }

    private:
        size_t _size = 0;
        size_t _capacity = 0;
        char *_data = nullptr;
        //�������ͳ��
        ObjectStatistic<BufferRaw> _statistic;
    };

    class BufferLikeString : public Buffer {
    public:
        ~BufferLikeString() override = default;

        BufferLikeString() {
            _erase_head = 0;
            _erase_tail = 0;
        }

        BufferLikeString(std::string str) {
            _str = std::move(str);
            _erase_head = 0;
            _erase_tail = 0;
        }

        BufferLikeString &operator=(std::string str) {
            _str = std::move(str);
            _erase_head = 0;
            _erase_tail = 0;
            return *this;
        }

        BufferLikeString(const char *str) {
            _str = str;
            _erase_head = 0;
            _erase_tail = 0;
        }

        BufferLikeString &operator=(const char *str) {
            _str = str;
            _erase_head = 0;
            _erase_tail = 0;
            return *this;
        }

        BufferLikeString(BufferLikeString &&that) {
            _str = std::move(that._str);
            _erase_head = that._erase_head;
            _erase_tail = that._erase_tail;
            that._erase_head = 0;
            that._erase_tail = 0;
        }

        BufferLikeString &operator=(BufferLikeString &&that) {
            _str = std::move(that._str);
            _erase_head = that._erase_head;
            _erase_tail = that._erase_tail;
            that._erase_head = 0;
            that._erase_tail = 0;
            return *this;
        }

        BufferLikeString(const BufferLikeString &that) {
            _str = that._str;
            _erase_head = that._erase_head;
            _erase_tail = that._erase_tail;
        }

        BufferLikeString &operator=(const BufferLikeString &that) {
            _str = that._str;
            _erase_head = that._erase_head;
            _erase_tail = that._erase_tail;
            return *this;
        }

        char *data() const override {
            return (char *) _str.data() + _erase_head;
        }

        size_t size() const override {
            return _str.size() - _erase_tail - _erase_head;
        }

        BufferLikeString &erase(size_t pos = 0, size_t n = std::string::npos) {
            if (pos == 0) {
                //�Ƴ�ǰ�������
                if (n != std::string::npos) {
                    //�Ƴ�����
                    if (n > size()) {
                        //�Ƴ�̫��������
                        throw std::out_of_range("BufferLikeString::erase out_of_range in head");
                    }
                    //������ʼ������
                    _erase_head += n;
                    data()[size()] = '\0';
                    return *this;
                }
                //�Ƴ�ȫ������
                _erase_head = 0;
                _erase_tail = _str.size();
                data()[0] = '\0';
                return *this;
            }

            if (n == std::string::npos || pos + n >= size()) {
                //�Ƴ�ĩβ��������
                if (pos >= size()) {
                    //�Ƴ�̫������
                    throw std::out_of_range("BufferLikeString::erase out_of_range in tail");
                }
                _erase_tail += size() - pos;
                data()[size()] = '\0';
                return *this;
            }

            //�Ƴ��м��
            if (pos + n > size()) {
                //������������
                throw std::out_of_range("BufferLikeString::erase out_of_range in middle");
            }
            _str.erase(_erase_head + pos, n);
            return *this;
        }

        BufferLikeString &append(const BufferLikeString &str) {
            return append(str.data(), str.size());
        }

        BufferLikeString &append(const std::string &str) {
            return append(str.data(), str.size());
        }

        BufferLikeString &append(const char *data) {
            return append(data, strlen(data));
        }

        BufferLikeString &append(const char *data, size_t len) {
            if (len <= 0) {
                return *this;
            }
            if (_erase_head > _str.capacity() / 2) {
                moveData();
            }
            if (_erase_tail == 0) {
                _str.append(data, len);
                return *this;
            }
            _str.insert(_erase_head + size(), data, len);
            return *this;
        }

        void push_back(char c) {
            if (_erase_tail == 0) {
                _str.push_back(c);
                return;
            }
            data()[size()] = c;
            --_erase_tail;
            data()[size()] = '\0';
        }

        BufferLikeString &insert(size_t pos, const char *s, size_t n) {
            _str.insert(_erase_head + pos, s, n);
            return *this;
        }

        BufferLikeString &assign(const char *data) {
            return assign(data, strlen(data));
        }

        BufferLikeString &assign(const char *data, size_t len) {
            if (len <= 0) {
                return *this;
            }
            if (data >= _str.data() && data < _str.data() + _str.size()) {
                _erase_head = data - _str.data();
                if (data + len > _str.data() + _str.size()) {
                    throw std::out_of_range("BufferLikeString::assign out_of_range");
                }
                _erase_tail = _str.data() + _str.size() - (data + len);
                return *this;
            }
            _str.assign(data, len);
            _erase_head = 0;
            _erase_tail = 0;
            return *this;
        }

        void clear() {
            _erase_head = 0;
            _erase_tail = 0;
            _str.clear();
        }

        char &operator[](size_t pos) {
            if (pos >= size()) {
                throw std::out_of_range("BufferLikeString::operator[] out_of_range");
            }
            return data()[pos];
        }

        const char &operator[](size_t pos) const {
            return (*const_cast<BufferLikeString *>(this))[pos];
        }

        size_t capacity() const {
            return _str.capacity();
        }

        void reserve(size_t size) {
            _str.reserve(size);
        }

        void resize(size_t size, char c = '\0') {
            _str.resize(size, c);
            _erase_head = 0;
            _erase_tail = 0;
        }

        bool empty() const {
            return size() <= 0;
        }

        std::string substr(size_t pos, size_t n = std::string::npos) const {
            if (n == std::string::npos) {
                //��ȡĩβ���е�
                if (pos >= size()) {
                    throw std::out_of_range("BufferLikeString::substr out_of_range");
                }
                return _str.substr(_erase_head + pos, size() - pos);
            }

            //��ȡ����
            if (pos + n > size()) {
                throw std::out_of_range("BufferLikeString::substr out_of_range");
            }
            return _str.substr(_erase_head + pos, n);
        }

    private:
        void moveData() {
            if (_erase_head) {
                _str.erase(0, _erase_head);
                _erase_head = 0;
            }
        }

    private:
        size_t _erase_head;
        size_t _erase_tail;
        std::string _str;
        //�������ͳ��
        ObjectStatistic<BufferLikeString> _statistic;
    };

#if defined(_WIN32)
    struct iovec {
        void *iov_base;    /* [XSI] Base address of I/O memory region */
        size_t iov_len;    /* [XSI] Size of region iov_base points to */
    };
    struct msghdr {
        void *msg_name;    /* [XSI] optional address */
        size_t msg_namelen;    /* [XSI] size of address */
        struct iovec *msg_iov;    /* [XSI] scatter/gather array */
        size_t msg_iovlen;    /* [XSI] # elements in msg_iov */
        void *msg_control;    /* [XSI] ancillary data, see below */
        int msg_controllen;    /* [XSI] ancillary data buffer len */
        int msg_flags;    /* [XSI] flags on received message */
    };
#else
#include <sys/uio.h>
#include <limits.h>
#endif

#if !defined(IOV_MAX)
#define IOV_MAX 1024
#endif

    class BufferList;

    class BufferSock : public Buffer {
    public:
        using Ptr = std::shared_ptr<BufferSock>;

        friend class BufferList;

        BufferSock(Buffer::Ptr ptr, struct sockaddr *addr = nullptr, int addr_len = 0);

        ~BufferSock();

        char *data() const override;

        size_t size() const override;

    private:
        int _addr_len = 0;
        struct sockaddr *_addr = nullptr;
        Buffer::Ptr _buffer;
    };

    class BufferList : public noncopyable {
    public:
        using Ptr = std::shared_ptr<BufferList>;
        using SendResult = std::function<void(const Buffer::Ptr &buffer, bool send_success)>;

        BufferList(List<std::pair<Buffer::Ptr, bool> > &list, SendResult cb = nullptr);

        ~BufferList();

        bool empty();

        size_t count();

        ssize_t send(int fd, int flags, bool udp);

    private:
        void reOffset(size_t n);

        ssize_t send_l(int fd, int flags, bool udp);

    private:
        size_t _iovec_off = 0;
        size_t _remain_size = 0;
        std::vector<struct iovec> _iovec;
        List<std::pair<Buffer::Ptr, bool> > _pkt_list;
        SendResult _cb;
        //�������ͳ��
        ObjectStatistic<BufferList> _statistic;
    };

}//namespace toolkit
#endif //ZLTOOLKIT_BUFFER_H