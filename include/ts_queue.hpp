#pragma once
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class TSQueue {
public:
    explicit TSQueue(std::size_t cap) : buf_(cap) {}

    // 阻塞 push（满则等）
    void push(const T& v) {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_full_.wait(lk, [&]{ return count_ < buf_.size(); });
        buf_[(head_) % buf_.size()] = v;
        ++head_; ++count_;
        cv_not_empty_.notify_one();
    }

    // 阻塞 pop（空则等）
    T pop() {
        std::unique_lock<std::mutex> lk(m_);
        cv_not_empty_.wait(lk, [&]{ return count_ > 0; });
        T v = *buf_[(tail_) % buf_.size()];
        buf_[(tail_) % buf_.size()].reset();
        ++tail_; --count_;
        cv_not_full_.notify_one();
        return v;
    }

private:
    std::vector<std::optional<T>> buf_;
    std::size_t head_ = 0, tail_ = 0, count_ = 0;
    std::mutex m_;
    std::condition_variable cv_not_full_, cv_not_empty_;
};
