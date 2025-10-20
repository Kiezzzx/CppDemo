#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <stdexcept>

template<typename T>
class TSQueue {
public:
    explicit TSQueue(std::size_t cap) : buf_(cap) {
        if (cap == 0) {
            throw std::invalid_argument("Queue capacity must be greater than 0");
        }
    }

    // blocking push
    void push(const T& v) {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_full_.wait(lk, [&]{ return count_ < buf_.size(); });
        buf_[head_] = v;
        head_ = (head_ + 1) % buf_.size();  
        ++count_;
        cv_not_empty_.notify_one();
    }

    // blocking pop
    T pop() {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_empty_.wait(lk, [&]{ return count_ > 0; });
        T v = *buf_[tail_];
        buf_[tail_].reset();
        tail_ = (tail_ + 1) % buf_.size();  
        --count_;
        cv_not_full_.notify_one();
        return v;
    }

private:
    std::vector<std::optional<T>> buf_;
    std::size_t head_ = 0, tail_ = 0, count_ = 0;
    std::mutex m_;
    std::condition_variable cv_not_full_, cv_not_empty_;
};
