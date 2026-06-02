/**
 * @file ObjectPool.hpp
 * @brief High-performance memory pool to prevent heap fragmentation.
 */
#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <utility>

namespace pluma {
namespace optimization {

/**
 * @class ObjectPool
 * @brief Pre-allocates and recycles objects of type T.
 * 
 * Uses a thread-safe freelist. Useful for high-frequency objects
 * like RunBox or DOMNode.
 */
template <typename T>
class ObjectPool {
public:
    ObjectPool() = default;

    /**
     * @brief Acquires an object from the pool or allocates a new one if empty.
     */
    template <typename... Args>
    std::unique_ptr<T, void(*)(T*)> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!freelist_.empty()) {
            T* obj = freelist_.back();
            freelist_.pop_back();
            // Re-initialize the object in-place
            obj->~T();
            new (obj) T(std::forward<Args>(args)...);
            return std::unique_ptr<T, void(*)(T*)>(obj, [](T* ptr) {
                ObjectPool<T>::getInstance().release(ptr);
            });
        }
        
        T* new_obj = new T(std::forward<Args>(args)...);
        return std::unique_ptr<T, void(*)(T*)>(new_obj, [](T* ptr) {
            ObjectPool<T>::getInstance().release(ptr);
        });
    }

    static ObjectPool<T>& getInstance() {
        static ObjectPool<T> instance;
        return instance;
    }

private:
    void release(T* ptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        freelist_.push_back(ptr);
    }

    std::vector<T*> freelist_;
    std::mutex mutex_;
};

} // namespace optimization
} // namespace pluma
