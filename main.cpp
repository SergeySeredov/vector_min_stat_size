#include <array>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <iostream>

using namespace std;

template<typename T, size_t min_size>

class Vector {
    static_assert(min_size > 0, "min_size должен быть больше 0!"); 

    array<T, min_size> static_data;
    T* dynamic_data = nullptr;
    size_t size_ = 0;
    size_t capacity_ = min_size;
    bool is_dynamic = false;

    void switch_to_dynamic() {
        capacity_ = max(size_ * 2, min_size * 2);
        dynamic_data = static_cast<T*>(operator new(capacity_ * sizeof(T)));
        
        for (size_t i = 0; i < size_; ++i) {
            new (&dynamic_data[i]) T(move(static_data[i]));
            static_data[i].~T();
        }
        
        is_dynamic = true;
        cout << "\nВектор перешёл в динамический режим\n" << endl;
    }

    void check_and_switch_to_static() {
        if (is_dynamic && size_ <= min_size) {
            for (size_t i = 0; i < size_; ++i) {
                new (&static_data[i]) T(move(dynamic_data[i]));
                dynamic_data[i].~T();
            }
            operator delete(dynamic_data);
            dynamic_data = nullptr;
            capacity_ = min_size;
            is_dynamic = false;
            cout << "\nВектор перешёл в статический режим\n" << endl;
        }
    }

public:
    Vector() = default;

    Vector(Vector&& other) noexcept : static_data(move(other.static_data)),
    dynamic_data(other.dynamic_data), 
    size_(other.size_), 
    capacity_(other.capacity_), 
    is_dynamic(other.is_dynamic)
    {
        other.dynamic_data = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        other.is_dynamic = false;
    }

    ~Vector() {
        if (is_dynamic) {
            for (size_t i = 0; i < size_; ++i) {
                dynamic_data[i].~T();
            }
            operator delete(dynamic_data);
        } else {
            for (size_t i = 0; i < size_; ++i) {
                static_data[i].~T();
            }
        }
    }

    void push_back(const T& value) {
        if (size_ == capacity_) {
            if (!is_dynamic) {
                switch_to_dynamic();
            } else {
                capacity_ *= 2;
                T* new_data = static_cast<T*>(operator new(capacity_ * sizeof(T)));
                for (size_t i = 0; i < size_; ++i) {
                    new (&new_data[i]) T(move(dynamic_data[i]));
                    dynamic_data[i].~T();
                }
                operator delete(dynamic_data);
                dynamic_data = new_data;
            }
        }

        if (is_dynamic) {
            new (&dynamic_data[size_]) T(value);
        } else {
            new (&static_data[size_]) T(value);
        }
        size_++;
    }

    void pop_back() {
        if (size_ == 0) throw out_of_range("\nВектор пустой!\n\n");
        size_--;
        
        if (is_dynamic) {
            dynamic_data[size_].~T();
        } else {
            static_data[size_].~T();
        }
        
        check_and_switch_to_static();
    }

    T& operator[](size_t index) {
        if (index >= size_) throw out_of_range("\nНекорректный индекс!\n\n");
        return is_dynamic ? dynamic_data[index] : static_data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) throw out_of_range("\nНекорректный индекс!\n\n");
        return is_dynamic ? dynamic_data[index] : static_data[index];
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool is_using_static_storage() const { return !is_dynamic; }

    Vector(const Vector& other) {
        for (const auto& item : other) {
            push_back(item);
        }
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            while (size_ > 0) pop_back();
            
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    T* begin() { return is_dynamic ? dynamic_data : static_data.data(); }
    const T* begin() const { return is_dynamic ? dynamic_data : static_data.data(); }
    T* end() { return (is_dynamic ? dynamic_data : static_data.data()) + size_; }
    const T* end() const { return (is_dynamic ? dynamic_data : static_data.data()) + size_; }
};

int main()
{
    Vector<int, 2> arr;
    arr.push_back(20);
    arr.push_back(35);
    arr.push_back(40);
    arr.pop_back();


}