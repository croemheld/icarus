//
// Created by croemheld on 09.01.2023.
//

#ifndef ICARUS_ADT_THREADSAFEMAP_H
#define ICARUS_ADT_THREADSAFEMAP_H

#include <icarus/ADT/Container.h>

#include <icarus/Threads/ThreadSafeContainer.h>

#include <sstream>

namespace icarus {

template <typename SubClass, typename K, typename V>
struct ThreadSafeMap : public ThreadSafeContainer<ThreadSafeMap<SubClass, K, V>, MapTraits<K, V>> {

  V &at(const K &k) {
    std::unique_lock<std::shared_mutex> Lock(this->Mutex);
    return this->Container[k];
  }

  V &operator[](const K &k) { return static_cast<SubClass *>(this)->at(k); }
};

} // namespace icarus

#endif // ICARUS_ADT_THREADSAFEMAP_H
