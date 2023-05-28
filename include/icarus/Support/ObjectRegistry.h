//
// Created by croemheld on 26.12.2022.
//

#ifndef ICARUS_SUPPORT_OBJECTREGISTRY_H
#define ICARUS_SUPPORT_OBJECTREGISTRY_H

#include <llvm/Support/RWMutex.h>

#include <map>
#include <type_traits>

namespace icarus {

template <typename T, typename Base = T, std::enable_if_t<std::is_default_constructible_v<T>, bool> = true>
Base *constructObject() {
  return (Base *)new T();
}

template <typename T> class ObjectConstructor {
  std::function<T *()> Constructor;

public:
  ObjectConstructor() = delete;

  explicit ObjectConstructor(std::function<T *()> &Constructor) : Constructor(Constructor) {}

  explicit ObjectConstructor(std::function<T *()> &&Constructor) : Constructor(std::move(Constructor)) {}

  ObjectConstructor(ObjectConstructor &Other) : Constructor(Other.getConstructor()) {}

  std::function<T *()> &getConstructor() { return Constructor; }

  T *operator()() { return Constructor(); }
};

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
  void enumerateObjects() { Registry::enumerateWith(this); }

  /**
   * Apply the callback instance on the selected object from the registry.
   */
  virtual void apply(T) {}
};

template <typename K, typename V, class Derived> class ObjectRegistry {

  mutable llvm::sys::SmartRWMutex<true> Lock;

  std::vector<RegistryListener<V *, Derived> *> Listeners;
  std::map<K, V *> ObjectMap;

  using ValueT = typename std::map<K, V *>::value_type::second_type;

  /**
   * Identity function for fetching an object from the registry.
   */
  static inline std::function<V *(V *)> ValueIdentity = [](V *Value) { return Value; };

  /**
   * Get the associated object for the provided key with the specifiec value extractor function.
   * @tparam ValueExtractor The type of the value extractor function.
   * @tparam T The type of the object to extract.
   * @param k The key to look up.
   * @param Extractor The value extractor function.
   * @return The object associated with the key and the extractor function.
   */
  template <typename ValueExtractor, typename T = std::invoke_result_t<ValueExtractor, ValueT>>
  T doGetObject(const K &k, ValueExtractor &&Extractor) {
    if (!ObjectMap.count(k))
      return nullptr;
    return Extractor(ObjectMap.at(k));
  }

  template <typename ValueExtractor, typename T = std::invoke_result_t<ValueExtractor, ValueT>>
  std::vector<T> doGetObjects(ValueExtractor &&Extractor) {
    std::vector<T> Objects;
    for (auto [_, Obj] : ObjectMap)
      Objects.push_back(Extractor(Obj));
    return Objects;
  }

  void doRegisterObject(const K &k, const V &Obj) {
    if (!ObjectMap.count(k)) {
      ObjectMap.insert(std::make_pair(k, &Obj));

      for (auto *L : Listeners)
        L->onRegistration(&Obj);
    }
  }

  /**
   * Apply the listener callback on every element in the object map structure.
   * @param L The callback listener to apply on every object in the map.
   */
  void doEnumerateWith(RegistryListener<V *, Derived> *L) {
    llvm::sys::SmartScopedReader<true> Guard(Lock);
    for (auto &[_, Val] : ObjectMap)
      L->apply(Val);
  }

  /**
   * Add a listener callback for every future object registration.
   * @param L The listener callback to add to the registry.
   */
  void doAddRegistrationListener(RegistryListener<V *, Derived> *L) {
    llvm::sys::SmartScopedWriter<true> Guard(Lock);
    Listeners.push_back(L);
  }

protected:
  /**
   * @return A pointer to the single instance of this registry.
   */
  static Derived *getObjectRegistry() {
    static Derived ObjectRegistryObj;
    return &ObjectRegistryObj;
  }

public:
  static void registerObject(const K &k, const V &Obj) { getObjectRegistry()->doRegisterObject(k, Obj); }

  template <typename ValueExtractor, typename T = std::invoke_result_t<ValueExtractor, ValueT>>
  static T getObject(const K &k, ValueExtractor &&Extractor = ValueIdentity) {
    return getObjectRegistry()->doGetObject(k, std::forward<ValueExtractor>(Extractor));
  }

  template <typename ValueExtractor, typename T = std::invoke_result_t<ValueExtractor, ValueT>>
  static std::vector<T> getObjects(ValueExtractor &&Extractor = ValueIdentity) {
    return getObjectRegistry()->doGetObjects(std::forward<ValueExtractor>(Extractor));
  }

  /**
   * Apply the listener callback on every element in the object map structure.
   * @param L The callback listener to apply on every object in the map.
   */
  static void enumerateWith(RegistryListener<V *, Derived> *L) { getObjectRegistry()->doEnumerateWith(L); }

  /**
   * Add a listener callback for every future object registration.
   * @param L The listener callback to add to the registry.
   */
  static void addRegistrationListener(RegistryListener<V *, Derived> *L) {
    getObjectRegistry()->doAddRegistrationListener(L);
  }
};

} // namespace icarus

#endif // ICARUS_SUPPORT_OBJECTREGISTRY_H
