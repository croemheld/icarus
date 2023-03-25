//
// Created by croemheld on 10.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_THREADS_THREADPOOL_H
#define ICARUS_INCLUDE_ICARUS_THREADS_THREADPOOL_H

#include <icarus/ADT/Container.h>

#include <icarus/Threads/ThreadSafeQueue.h>

#include <atomic>
#include <future>

namespace icarus {

/**
 * General class representing a thread pool task. The class is used as an interface, since a derived
 * class (see ThreadTask below) always has different types depending on the template arguments. This
 * allows us to run any function with any signature in the thread pool.
 */
struct Task {
  Task() = default;
  Task(const Task& Other) = delete;
  Task(Task&& Other) = default;
  virtual ~Task() = default;
  Task& operator=(const Task& Other) = delete;
  Task& operator=(Task&& Other) = default;

  /**
   * Executes the provided function in this thread pool task. The implementation is done by subclass
   * as it requires a template argument that cannot be done in this interface.
   */
  virtual void execute() = 0;
};

/**
 * Implementation of the Task class above. It wraps any function in a Task class
 * instance that is moved to the task queue of the thread pool.
 * @tparam Func The function signature of the task to schedule.
 */
template <typename Func>
class ThreadTask : public Task {
  Func ThreadFunction;
public:
  explicit ThreadTask(Func &&Function) : ThreadFunction(std::move(Function)) {}
  ThreadTask(const ThreadTask& Other) = delete;
  ThreadTask(ThreadTask&& Other) = delete;
  ~ThreadTask() override = default;
  ThreadTask& operator=(const ThreadTask& Other) = delete;
  ThreadTask& operator=(ThreadTask&& Other) noexcept = default;
  void execute() override { ThreadFunction(); }
};

/**
 * The class for a global thread pool instance. There should only be one single thread pool that can be accessed
 * via static methods to ensure uniformity and maximum throughput for all threads. Do not create a local or even
 * one or more global thread pools. Instead, use only the static methods via ThreadPool::initialize, ...
 */
class ThreadPool {

  /**
   * Specialization of the thread safe queue for scheduling thread pool tasks. All tasks stored in the
   * queue are wrapped in std::unique_ptr so that they are deallocated automatically after completion.
   */
  struct TaskQueue : public ThreadSafeQueue<TaskQueue, std::unique_ptr<Task>> {};

  std::atomic_bool Running = false;

  TaskQueue Tasks;
  std::atomic_uint TotalTasks = 0;
  std::vector<std::thread> Threads;

  mutable std::shared_mutex Mutex;
  std::condition_variable_any Condition;

  std::map<std::thread::id, unsigned> ThreadIDMap;

  /**
   * @return The singleton ThreadPool instance.
   */
  static ThreadPool &get();

  /**
   * The main logic for every ThreadPool task which waits until a new task has been enqueued before it
   * executes the submitted function.
   */
  void worker();

  /**
   * Initializes the thread pool tasks and the normalized thread ID map structure.
   * @param ThreadNum The number of threads - 1 to initialize in the tread pool.
   */
  void doInitialize(unsigned ThreadNum = std::thread::hardware_concurrency());

  /**
   * @return The normalized thread ID of the current std::thread instance.
   */
  unsigned doGetThreadID() const;

  /**
   * @return The number of threads initialized in this thread pool instance.
   */
  unsigned doGetThreadNum() const;

  /**
   * Submits an arbitrarily typed function and its argument in the thread pool task queue.
   * @tparam Func The type of the function to submit.
   * @tparam Args The variadic types of the function arguments.
   * @tparam RetTy The return type of the function.
   * @tparam PTask Type of the packaged task wrapping the callable function.
   * @tparam TaskT The ThreadTask type with the template parameter from the function.
   * @param Function The function to execute in a thread pool task.
   * @param args The arguments to pass to the function.
   * @return A std::future that allows us to wait for the scheduled function to return.
   */
  template <typename Func, typename ... Args>
  auto doSubmit(Func &&Function, Args&&... args) {
    auto Task = std::bind(std::forward<Func>(Function), std::forward<Args>(args)...);

    using RetTy = std::result_of_t<decltype(Task)()>;
    using PTask = std::packaged_task<RetTy()>;
    using TaskT = ThreadTask<PTask>;

    PTask PackagedTask(std::move(Task));
    std::future<RetTy> Future(PackagedTask.get_future());
    Tasks.push(std::make_unique<TaskT>(std::move(PackagedTask)));
    ++TotalTasks;
    return Future;
  }

  /**
   * Waits until all tasks in the queue have been finished. This method is blocking the current thread
   * until ThreadPool::TotalTasks is zero before returning.
   */
  void doAwaitCompletion();

  /**
   * Shuts down the thread pool instance. The method needs to be called to clean up just before icarus
   * exits, otherwise we get errors of threads not being able to join, even though the program exited.
   */
  void doShutdown();

public:

  /**
   * Static method with call to singleton method ThreadPool::doInitialize.
   * @param Threads The number of threads - 1 to initialize in the tread pool.
   */
  static void initialize(unsigned Threads);

  /**
   * @return The normalized thread ID of the current std::thread instance.
   */
  static unsigned getThreadID();

  /**
   *
   * @return The number of threads initialized in this thread pool instance.
   */
  static unsigned getThreadNum();

  /**
   * Static method with call to singleton method ThreadPool::submit.
   * @tparam Func The type of the function to submit.
   * @tparam Args The variadic types of the function arguments.
   * @param Function The function to execute in a thread pool task.
   * @param args The arguments to pass to the function.
   * @return A std::future that allows us to wait for the scheduled function to return.
   */
  template <typename Func, typename ... Args>
  static auto submit(Func &&Function, Args&&... args) {
    return get().doSubmit(std::forward<Func>(Function), std::forward<Args>(args)...);
  }

  /**
   * Static method with call to singleton method ThreadPool::doAwaitCompletion.
   */
  static void awaitCompletion();

  /**
   * Static method with call to singleton method ThreadLogger::doShutdown.
   */
  static void shutdown();

};

}

#endif // ICARUS_INCLUDE_ICARUS_THREADS_THREADPOOL_H
