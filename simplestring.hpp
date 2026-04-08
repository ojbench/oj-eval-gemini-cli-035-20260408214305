#ifndef SIMPLE_STRING_SIMPLESTRING_HPP
#define SIMPLE_STRING_SIMPLESTRING_HPP

#include <stdexcept>
#include <cstring>
#include <iostream>

class MyString {
private:
    union {
        char* heap_ptr;
        char small_buffer[16];
    };
    size_t _size;
    size_t _capacity;

    bool is_sso() const {
        return _capacity == 15;
    }

    char* get_ptr() {
        return is_sso() ? small_buffer : heap_ptr;
    }

    const char* get_ptr() const {
        return is_sso() ? small_buffer : heap_ptr;
    }

public:
    MyString() : _size(0), _capacity(15) {
        small_buffer[0] = '\0';
    }

    MyString(const char* s) {
        if (!s) s = "";
        _size = std::strlen(s);
        if (_size <= 15) {
            _capacity = 15;
            std::memcpy(small_buffer, s, _size + 1);
        } else {
            _capacity = _size;
            heap_ptr = new char[_capacity + 1];
            std::memcpy(heap_ptr, s, _size + 1);
        }
    }

    MyString(const MyString& other) {
        _size = other._size;
        _capacity = other._capacity;
        if (is_sso()) {
            std::memcpy(small_buffer, other.small_buffer, 16);
        } else {
            heap_ptr = new char[_capacity + 1];
            std::memcpy(heap_ptr, other.heap_ptr, _size + 1);
        }
    }

    MyString(MyString&& other) noexcept {
        _size = other._size;
        _capacity = other._capacity;
        if (is_sso()) {
            std::memcpy(small_buffer, other.small_buffer, 16);
        } else {
            heap_ptr = other.heap_ptr;
            other.heap_ptr = nullptr;
        }
        other._size = 0;
        other._capacity = 15;
        other.small_buffer[0] = '\0';
    }

    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            if (!is_sso()) {
                delete[] heap_ptr;
            }
            _size = other._size;
            _capacity = other._capacity;
            if (is_sso()) {
                std::memcpy(small_buffer, other.small_buffer, 16);
            } else {
                heap_ptr = other.heap_ptr;
                other.heap_ptr = nullptr;
            }
            other._size = 0;
            other._capacity = 15;
            other.small_buffer[0] = '\0';
        }
        return *this;
    }

    MyString& operator=(const MyString& other) {
        if (this != &other) {
            if (other.is_sso()) {
                if (!is_sso()) {
                    delete[] heap_ptr;
                }
                _size = other._size;
                _capacity = other._capacity;
                std::memcpy(small_buffer, other.small_buffer, 16);
            } else {
                if (_capacity < other._size || (!is_sso() && other._size < _capacity / 4)) {
                    if (!is_sso()) {
                        delete[] heap_ptr;
                    }
                    _capacity = other._capacity;
                    heap_ptr = new char[_capacity + 1];
                }
                _size = other._size;
                std::memcpy(get_ptr(), other.get_ptr(), _size + 1);
            }
        }
        return *this;
    }

    ~MyString() {
        if (!is_sso()) {
            delete[] heap_ptr;
        }
    }

    const char* c_str() const {
        return get_ptr();
    }

    size_t size() const {
        return _size;
    }

    size_t capacity() const {
        return _capacity;
    }

    void reserve(size_t new_capacity) {
        if (new_capacity <= _capacity) return;
        if (new_capacity <= 15) return;
        
        char* new_ptr = new char[new_capacity + 1];
        std::memcpy(new_ptr, get_ptr(), _size + 1);
        
        if (!is_sso()) {
            delete[] heap_ptr;
        }
        
        heap_ptr = new_ptr;
        _capacity = new_capacity;
    }

    void resize(size_t new_size) {
        if (new_size > _size) {
            if (new_size > _capacity) {
                reserve(new_size);
            }
            std::memset(get_ptr() + _size, '\0', new_size - _size);
            get_ptr()[new_size] = '\0';
        } else if (new_size < _size) {
            get_ptr()[new_size] = '\0';
            if (!is_sso() && new_size <= 15) {
                char temp[16];
                std::memcpy(temp, heap_ptr, new_size + 1);
                delete[] heap_ptr;
                _capacity = 15;
                std::memcpy(small_buffer, temp, 16);
            } else if (!is_sso() && new_size < _capacity / 4) {
                size_t new_cap = new_size * 2;
                char* new_ptr = new char[new_cap + 1];
                std::memcpy(new_ptr, heap_ptr, new_size + 1);
                delete[] heap_ptr;
                heap_ptr = new_ptr;
                _capacity = new_cap;
            }
        }
        _size = new_size;
    }

    char& operator[](size_t index) {
        if (index >= _size) {
            throw std::out_of_range("Index out of range");
        }
        return get_ptr()[index];
    }

    MyString operator+(const MyString& rhs) const {
        MyString res;
        res.reserve(_size + rhs._size);
        res._size = _size + rhs._size;
        std::memcpy(res.get_ptr(), get_ptr(), _size);
        std::memcpy(res.get_ptr() + _size, rhs.get_ptr(), rhs._size + 1);
        return res;
    }

    void append(const char* str) {
        if (!str) return;
        size_t len = std::strlen(str);
        if (len == 0) return;
        if (_size + len > _capacity) {
            size_t new_cap = _capacity * 2;
            if (new_cap < _size + len) new_cap = _size + len;
            
            char* new_ptr = new char[new_cap + 1];
            std::memcpy(new_ptr, get_ptr(), _size);
            std::memcpy(new_ptr + _size, str, len + 1);
            
            if (!is_sso()) {
                delete[] heap_ptr;
            }
            
            heap_ptr = new_ptr;
            _capacity = new_cap;
        } else {
            std::memmove(get_ptr() + _size, str, len + 1);
        }
        _size += len;
    }

    const char& at(size_t pos) const {
        if (pos >= _size) {
            throw std::out_of_range("Index out of range");
        }
        return get_ptr()[pos];
    }

    class const_iterator;

    class iterator {
    private:
        char* ptr;
    public:
        iterator(char* p) : ptr(p) {}

        iterator& operator++() {
            ++ptr;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++ptr;
            return temp;
        }

        iterator& operator--() {
            --ptr;
            return *this;
        }

        iterator operator--(int) {
            iterator temp = *this;
            --ptr;
            return temp;
        }

        char& operator*() const {
            return *ptr;
        }

        bool operator==(const iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const iterator& other) const {
            return ptr != other.ptr;
        }

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
        
        friend class const_iterator;
    };

    class const_iterator {
    private:
        const char* ptr;
    public:
        const_iterator(const char* p) : ptr(p) {}
        const_iterator(const iterator& it) : ptr(it.ptr) {}

        const_iterator& operator++() {
            ++ptr;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++ptr;
            return temp;
        }

        const_iterator& operator--() {
            --ptr;
            return *this;
        }

        const_iterator operator--(int) {
            const_iterator temp = *this;
            --ptr;
            return temp;
        }

        const char& operator*() const {
            return *ptr;
        }

        bool operator==(const const_iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const const_iterator& other) const {
            return ptr != other.ptr;
        }

        bool operator==(const iterator& other) const {
            return ptr == other.ptr;
        }

        bool operator!=(const iterator& other) const {
            return ptr != other.ptr;
        }
        
        friend class iterator;
    };

    iterator begin() {
        return iterator(get_ptr());
    }

    iterator end() {
        return iterator(get_ptr() + _size);
    }

    const_iterator cbegin() const {
        return const_iterator(get_ptr());
    }

    const_iterator cend() const {
        return const_iterator(get_ptr() + _size);
    }
};

inline bool MyString::iterator::operator==(const const_iterator& other) const {
    return ptr == other.ptr;
}

inline bool MyString::iterator::operator!=(const const_iterator& other) const {
    return ptr != other.ptr;
}

#endif