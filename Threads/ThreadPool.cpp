//
// Created by croemheld on 15.01.2023.
//

#include <icarus/Threads/ThreadPool.h>

namespace icarus {

ThreadPool &ThreadPool::get() {
  static ThreadPool TP;
  return TP;
}

void ThreadPool::worker() {
  while (Running) {
    std::unique_ptr<Task> Task;
    if (Tasks.pop(Task)) {
      Task->execute();
      --TotalTasks;
      Condition.notify_one();
    }
  }
}

void ThreadPool::doInitialize(unsigned ThreadNum) {
  ThreadNum = std::max(1U, ThreadNum);

  /* Add current thread to map of thread IDs */
  ThreadIDMap[std::this_thread::get_id()] = 0;

  for (unsigned N = 1; N < ThreadNum; ++N) {
    std::thread Thread(&ThreadPool::worker, this);
    ThreadIDMap[Thread.get_id()] = N;
    Threads.emplace_back(std::move(Thread));
  }
  Running = true;
}

unsigned ThreadPool::doGetThreadID() const {
  return ThreadIDMap.at(std::this_thread::get_id());
}

unsigned ThreadPool::doGetThreadNum() const {
  return Threads.size();
}

void ThreadPool::doAwaitCompletion() {
  std::unique_lock<std::shared_mutex> Lock(Mutex);
  Condition.wait(Lock, [&]() {
    return TotalTasks == 0;
  });
}

void ThreadPool::doShutdown() {
  Running = false;
  Tasks.invalidate();
  for (auto &Thread : Threads) {
    if (Thread.joinable())
      Thread.join();
  }
}

/*
 * Static public methods
 */

void ThreadPool::initialize(unsigned Threads) {
  get().doInitialize(Threads);
}

unsigned ThreadPool::getThreadID() {
  return get().doGetThreadID();
}

unsigned ThreadPool::getThreadNum() {
  return get().doGetThreadNum();
}

void ThreadPool::awaitCompletion() {
  get().doAwaitCompletion();
}

void ThreadPool::shutdown() {
  get().doShutdown();
}

}