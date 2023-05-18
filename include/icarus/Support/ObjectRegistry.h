//
// Created by croemheld on 26.12.2022.
//

#ifndef ICARUS_SUPPORT_OBJECTREGISTRY_H
#define ICARUS_SUPPORT_OBJECTREGISTRY_H

#include <llvm/Support/RWMutex.h>

#include <map>
#include <type_traits>

namespace icarus {

template <typename T, typename Ret, std::enable_if_t<std::is_default_constructible<T>{}, bool> = true>
Ret *createObj() {
  return new T();
}

/**
 * Template version of PassRegistrationListener which can be applied on multiple types.
 * @tparam T The type of the element to invoke a callback on.
 * @tparam Registry The ObjectRegistry for this listener class.
 */
template <typename T, typename Registry> struct RegistryListener {
  RegistryListener() = default;
  virtual ~RegistryListener() = default;

  /**
   * Callback method handling the registration of the specified element.
   */
  virtual void onRegistration(T) {}

  /**
   * Apply the callback instance on every object stored in the registry.
   */
  void enumerateObjects() { Registry::getObjectRegistry()->enumerateWith(this); }

  /**
   * Apply the callback instance on the selected object from the registry.
   */
  virtual void apply(T) {}
};

template <typename K, typename V, class Derived> class ObjectRegistry {

  mutable llvm::sys::SmartRWMutex<true> Lock;

protected:
  std::vector<RegistryListener<V *, Derived> *> Listeners;
  std::map<K, V *> ObjectMap;

public:
  /**
   * @return A pointer to the single instance of this registry.
   */
  static Derived *getObjectRegistry() {
    static Derived ObjectRegistryObj;
    return &ObjectRegistryObj;
  }

  /**
   * Mandatory method to be implemented by deriving classes.
   * @param Obj The object to store in the map structure.
   */
  virtual void registerObject(const V &Obj) = 0;

  /**
   * Get the associated entry for the provided key in the object registry.
   * @param k The key to lookup.
   * @return A pointer to the associated object or nullptr, if the key does not exist.
   */
  V *getObjectOrNull(const K &k) const {
    if (!ObjectMap.count(k))
      return nullptr;
    return ObjectMap.at(k);
  }

  /**
   * Apply the listener callback on every element in the object map structure.
   * @param L The callback listener to apply on every object in the map.
   */
  void enumerateWith(RegistryListener<V *, Derived> *L) {
    llvm::sys::SmartScopedReader<true> Guard(Lock);
    for (auto &[_, Val] : ObjectMap)
      L->apply(Val);
  }

  /**
   * Add a listener callback for every future object registration.
   * @param L The listener callback to add to the registry.
   */
  void addRegistrationListener(RegistryListener<V *, Derived> *L) {
    llvm::sys::SmartScopedWriter<true> Guard(Lock);
    Listeners.push_back(L);
  }
};

} // namespace icarus

#endif // ICARUS_SUPPORT_OBJECTREGISTRY_H
