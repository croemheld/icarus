//
// Created by croemheld on 10.01.2023.
//

#ifndef ICARUS_THREADS_THREADSAFEQUEUE_H
#define ICARUS_THREADS_THREADSAFEQUEUE_H

#include <icarus/ADT/Container.h>

#include <icarus/Threads/ThreadSafeContainer.h>

namespace icarus {

/**
 * Full specialization of the ThreadSafeContainer template class wrapping a std::queue. It is equipped
 * with synchronization primitives in the base class and implements some of std::queue's methods.
 * @tparam SubClass The subclass that might provide custom methods overriding default behavior.
 * @tparam T The type of the elements stored in the thread safe queue.
 */
template <typename SubClass, typename T>
struct ThreadSafeQueue : public ThreadSafeContainer<ThreadSafeQueue<SubClass, T>, QueueTraits<T>> {

  /**
   * Pushes an element to the end of the queue while holding an unique (read-write) lock.
   * @param t The element to push to the end of the queue.
   */
  void push(T &&t) {
    std::unique_lock<std::shared_mutex> Lock(this->Mutex);
    this->Container.push(std::move(t));
    this->Condition.notify_one();
  }

  /**
   * Removes the first element from the queue while holding an unique (read-write) lock. The caller is
   * responsible for checking if the queue contains any elements before calling this method.
   */
  void pop() {
    std::unique_lock<std::shared_mutex> Lock(this->Mutex);
    T t = std::move(this->Container.front());
    this->Container.pop();
  }

  /**
   * Removes the first element from the queue and returns it while holding an unique (read-write) lock
   * if the queue is not empty. If it is empty, the thread blocks until at least one element is inside
   * the queue. The element is then assigned to the argument by reference.
   * @param t The reference to the variable where the first element of the queue is assigned to.
   * @return True, if the element was successfully removed and assigned to the reference.
   */
  bool pop(T &t) {
    std::unique_lock<std::shared_mutex> Lock(this->Mutex);
    this->Condition.wait(Lock, [&]() { return !this->Container.empty() || !this->Status; });
    if (!this->Status)
      return false;
    t = std::move(this->Container.front());
    this->Container.pop();
    return true;
  }
};

} // namespace icarus

#endif // ICARUS_THREADS_THREADSAFEQUEUE_H
