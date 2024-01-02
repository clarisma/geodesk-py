// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once
#include <cassert>
#include <cstddef>  // For size_t
#include <cstdint>

template <typename T, size_t SIZE>
class FixedQueue
{
public:
    FixedQueue() : front_(0), rear_(0), count_(0) {}

    /**
     * Adds an item to the queue. The queue must not be full,
     * or else the behavior is undefined.
     */
    void add(T item)
    {
        assert(!isFull());
        queue_[rear_] = item;
        rear_ = (rear_ + 1) % SIZE;
        count_++;
    }

    /**
     * Attempts to add an item to the queue. Returns true if
     * there is room in the queue, else false.
     */
    bool tryAdd(T item)
    {
        if (isFull()) return false;
        add(item);
        return true;
    }

    /**
     * Removes an item from the head of the queue. The queue
     * must not be empty, or else the behavior is undefined.
     */
    T remove()
    {
        assert(!isEmpty());
        T item = queue_[front_];
        front_ = (front_ + 1) % SIZE;
        count_--;
        return item;
    }

    /**
     * Returns the item at the head of the queue without
     * removing it, or `nullptr` if the queue is empty.
     */
    T peek()
    {
        return isEmpty() ? nullptr : queue_[front_];
    }

    bool isEmpty() const { return count_ == 0; }
    bool isFull() const { return count_ == SIZE; }
    int count() { return count_; }

private:
	T queue_[SIZE];
    int front_;
    int rear_;
    int count_;
};
