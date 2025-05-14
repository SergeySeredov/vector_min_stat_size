#include <array>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <iostream>

using namespace std;

template<typename T, size_t min_size>
class Vector {
    static_assert(min_size > 0, "min_size должен быть больше 0!"); 

    union Data {
        array<T, min_size> static_data;
        struct {
            T* data;
            size_t capacity;
        } dynamic_data;

        Data() : static_data() {}
        ~Data() {}
    };

    Data data_;
    size_t size_ = 0;
    bool is_dynamic = false;

    void switch_to_dynamic() {
        size_t new_capacity = max(size_ * 2, min_size * 2);
        T* new_data = static_cast<T*>(operator new(new_capacity * sizeof(T)));
        
        for (size_t i = 0; i < size_; ++i) {
            new (&new_data[i]) T(move(data_.static_data[i]));
            data_.static_data[i].~T();
        }
        
        data_.dynamic_data.data = new_data;
        data_.dynamic_data.capacity = new_capacity;
        is_dynamic = true;
        cout << "\nВектор перешёл в динамический режим\n" << endl;
    }

    void check_and_switch_to_static() {
        if (is_dynamic && size_ <= min_size) {
            array<T, min_size> new_data;
            for (size_t i = 0; i < size_; i++) {
                new (&new_data[i]) T(move(data_.dynamic_data.data[i]));
                data_.dynamic_data.data[i].~T();
            }
        
            data_.static_data = new_data;
            is_dynamic = false;
            cout << "\nВектор перешёл в статический режим\n" << endl;
        }
    }

public:
    Vector() = default;

    Vector(Vector&& other) noexcept : 
        size_(other.size_),
        is_dynamic(other.is_dynamic)
    {
        if (is_dynamic) {
            data_.dynamic_data.data = other.data_.dynamic_data.data;
            data_.dynamic_data.capacity = other.data_.dynamic_data.capacity;
            other.data_.dynamic_data.data = nullptr;
            other.data_.dynamic_data.capacity = 0;
        } else {
            for (size_t i = 0; i < size_; i++) {
                new (&data_.static_data[i]) T(move(other.data_.static_data[i]));
                other.data_.static_data[i].~T();
            }
        }
        other.size_ = 0;
    }

    ~Vector() {
        if (is_dynamic) {
            for (size_t i = 0; i < size_; ++i) {
                data_.dynamic_data.data[i].~T();
            }
            operator delete(data_.dynamic_data.data);
        } else {
            for (size_t i = 0; i < size_; ++i) {
                data_.static_data[i].~T();
            }
        }
    }

    void push_back(const T& value) {
        if (size_ == (is_dynamic ? data_.dynamic_data.capacity : min_size)) {
            if (!is_dynamic) {
                switch_to_dynamic();
            } else {
                size_t new_capacity = data_.dynamic_data.capacity * 2;
                T* new_data = static_cast<T*>(operator new(new_capacity * sizeof(T)));
                for (size_t i = 0; i < size_; ++i) {
                    new (&new_data[i]) T(move(data_.dynamic_data.data[i]));
                    data_.dynamic_data.data[i].~T();
                }
                operator delete(data_.dynamic_data.data);
                data_.dynamic_data.data = new_data;
                data_.dynamic_data.capacity = new_capacity;
            }
        }

        if (is_dynamic) {
            new (&data_.dynamic_data.data[size_]) T(value);
        } else {
            new (&data_.static_data[size_]) T(value);
        }
        size_++;
    }

    void pop_back() {
        if (size_ == 0) throw out_of_range("\nВектор пустой!\n\n");
        
        if (is_dynamic) {
            data_.dynamic_data.data[size_].~T();
        } else {
            data_.static_data[size_].~T();
        }

        size_--;
        
        check_and_switch_to_static();
    }

    T& operator[](size_t index) {
        if (index >= size_) throw out_of_range("\nНекорректный индекс!\n\n");
        return is_dynamic ? data_.dynamic_data.data[index] : data_.static_data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) throw out_of_range("\nНекорректный индекс!\n\n");
        return is_dynamic ? data_.dynamic_data.data[index] : data_.static_data[index];
    }

    size_t size() const { return size_; }
    size_t capacity() const { return is_dynamic ? data_.dynamic_data.capacity : min_size; }
    bool is_using_static_storage() const { return !is_dynamic; }

    Vector(const Vector& other) : size_(0), is_dynamic(false) {
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

    T* begin() { return is_dynamic ? data_.dynamic_data.data : data_.static_data.data(); }
    const T* begin() const { return is_dynamic ? data_.dynamic_data.data : data_.static_data.data(); }
    T* end() { return (is_dynamic ? data_.dynamic_data.data : data_.static_data.data()) + size_; }
    const T* end() const { return (is_dynamic ? data_.dynamic_data.data : data_.static_data.data()) + size_; }
};

int main()
{
    Vector<int, 2> arr;
    arr.push_back(20);
    arr.push_back(35);
    cout << arr[0] << " " << arr[1] << " " << endl;
    arr.push_back(40);
    cout << arr[0] << " " << arr[1] << " " << arr[2] << endl;
    arr.pop_back();
    cout << arr[0] << " " << arr[1] << " " << endl;
    return 0;
}