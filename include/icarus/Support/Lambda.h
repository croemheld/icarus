//
// Created by croemheld on 31.12.2022.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_LAMBDA_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_LAMBDA_H

namespace icarus {

struct lambda {

  /*
   * Variadic template arguments (experimental)
   */

  template <typename Ret, typename Sig, typename... Ts>
  static Ret exc(Ts... args) {
    return (Ret) (*(Sig*)fun())(std::forward<Ts>(args)...);
  }

  template <typename Ret = void, typename... Ts, typename RetPtr = Ret(*)(Ts...), typename Sig>
  static RetPtr ptr(Sig& sig) {
    fun(&sig);
    return (RetPtr) exc<Ret, Sig, Ts...>;
  }

  static void *fun(void *Callback = nullptr) {
    static void *InternalCallback;
    if (Callback != nullptr)
      InternalCallback = Callback;
    return InternalCallback;
  }

};



}

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_LAMBDA_H
