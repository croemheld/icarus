//
// Created by croemheld on 08.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_THREAD_THREADSAFECONTAINER_H
#define ICARUS_INCLUDE_ICARUS_THREAD_THREADSAFECONTAINER_H

#include <icarus/ADT/Container.h>

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace icarus {

/**
 * Generic implementation for thread safe containers, intended to work for most STL containers.
 * This class will not provide a complete interface to the underlying container structure: This
 * is because they are seldom used in icarus. Yet, for some cases they are still necessary.
 * @tparam C The container type to wrap in a thread safe container.
 */
template <typename SubClass, typename Traits = ContainerTraits<SubClass>, typename C = typename Traits::container_type,
          typename T = typename Traits::value_type>
class ThreadSafeContainer {

protected:
  C Container;
  std::atomic_bool Status = {true};
  mutable std::shared_mutex Mutex;
  std::condition_variable_any Condition;

public:
  /**
   * Creates a new thread safe container object that wraps the original STL container member variable.
   * Each wrapped container hast to provide a ContainerTraits<SubClass> trait that contains the traits
   * of the respective container (see ADT/Container.h).
   */
  ThreadSafeContainer() = default;

  /**
   * Upon destroying this object we need to notify all threads that are potentially waiting in line to
   * access the elements in this container.
   */
  ~ThreadSafeContainer() { invalidate(); };

  /**
   * Returns the size of this container while holding a shared (a reader-only) lock on this structure.
   * @return The number of elements in this container.
   */
  std::size_t size() const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Container.size();
  }

  /**
   * Indicates, whether or not the container is empty while holding a shared (reader-only lock) on the
   * container structure.
   * @return True, if the container is empty.
   */
  bool empty() const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Container.empty();
  }

  /**
   * Clears all the elements in the container structure, while holding an unique (read-write) lock for
   * this object. When this is called, no other locks are held (no reader & writer locks).
   */
  void clear() {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    Container.clear();
    Condition.notify_all();
  }

  /**
   * Invalidates all condition variables, that are waiting for conditions related to this container to
   * evaluate to true, and subsequently notifies all sleeping threads of this change.
   */
  void invalidate() {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    Status = false;
    Condition.notify_all();
  }

  /**
   * Inspired by [1]: Holds the lock while applying a callback on every container element. The callback can only be
   * a non-capturing function (lambda) which solely accepts an element type as the only argument. Upon reaching the
   * end of the function, the lock is automatically released.
   * [1] https://stackoverflow.com/a/54449603/3741284
   * @tparam Func The type of the function, deduced automatically.
   * @param Function The callback function to call for each element in the container.
   */
  template <typename Func> void forEach(Func &&Function) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    for (T &Element : Container) {
      std::forward<Func>(Function)(Element);
    }
  }

  /**
   * Similar to forEach, except that any code can be executed while the lock on this object is being hold. After the
   * code in the scoped lock finished, the lock on this object is subsequently released. This template automatically
   * deduces the types of the arguments, so no templates need to be specified when calling this method.
   * @tparam Func The function type to execute while the lock is being held.
   * @tparam Args The argument types to pass to the generic function.
   * @param Function The function to call.
   * @param args The arguments to pass to the function.
   */
  template <typename Func, typename... Args> void lock(Func &&Function, Args &&...args) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    return std::forward<Func>(Function)(std::forward<Args>(args)...);
  }
};

} // namespace icarus

#endif // ICARUS_INCLUDE_ICARUS_THREAD_THREADSAFECONTAINER_H
