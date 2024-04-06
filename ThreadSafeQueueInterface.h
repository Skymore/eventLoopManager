//
// Created by 陶睿 on 4/2/24.
//

#ifndef EVENTLOOPMANAGER_THREADSAFEQUEUE_H
#define EVENTLOOPMANAGER_THREADSAFEQUEUE_H
#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class ThreadSafeQueueInterface {
public:

    virtual ~ThreadSafeQueueInterface() = default;

    // Inserts the specified element into this queue.
    virtual void push(const T& value) = 0;

    // Retrieves and removes the head of this queue, waiting if necessary until an element becomes available.
    virtual T waitAndPop() = 0;

    // Retrieves and removes the head of this queue. Throws an exception if the queue is empty.
    virtual T pop() = 0;

    // Retrieves, but does not remove, the head of this queue. Throws an exception if the queue is empty.
    virtual T front() const = 0;

    // Retrieves the head of this queue, waiting if necessary until an element becomes available.
    virtual T waitAndFront() const = 0;

    // Retrieves, but does not remove, the tail of this queue. Throws an exception if the queue is empty.
    virtual T back() const = 0;

    // Retrieves the tail of this queue, waiting if necessary until an element becomes available.
    virtual T waitAndBack() const = 0;

    // Returns true if this queue contains no elements.
    virtual bool empty() const = 0;

    // Returns the number of elements in this queue.
    virtual size_t size() const = 0;

    // Removes all of the elements from this queue.
    virtual void clear() = 0;
};


#endif //EVENTLOOPMANAGER_THREADSAFEQUEUE_H
