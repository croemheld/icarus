//
// Created by croemheld on 24.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H

#include <llvm/IR/Module.h>

#include <nlohmann/json.hpp>

namespace icarus {

struct VariableDef;
struct CallContext;
struct InputArguments;

namespace adl_json {

/**
 * Conversion function for nlohmann::json -> VariableDef.
 * @param JSON The reference to the nlohmann::json instance.
 * @param VD The reference to the VariableRef instance.
 * @param M Pointer to the llvm::Module in order to completely initialize the class.
 */
void from_json(const nlohmann::json& JSON, VariableDef& VD, llvm::Module *M);

/**
 * Conversion function for nlohmann::json -> CallContext.
 * @param JSON The reference to the nlohmann::json instance.
 * @param VD The reference to the CallContext instance.
 * @param M Pointer to the llvm::Module in order to completely initialize the class.
 */
void from_json(const nlohmann::json& JSON, CallContext& CC, llvm::Module *M);

/**
 * Convert JSON object from nlohmann::json to InputArguments to generate a initial state for analysis.
 * This function assumes that the JSON object contains some or all of the following identifiers:
 *
 *  - "variables": [ "<name>": <variable>, ... ]
 *  <name> specifies the name of the variable. It may be an existing global variable or a new one. For
 *  the <variable> placeholder, there are several options. If it is a non-aggregate, such as an int or
 *  float, it contains the type preceded by the value (i.e., "i32 0"). For aggregate types, the place-
 *  holder is a JSON object with properties "type" and "elem" for the type and its elements. Arrays in
 *  this example would be: { "type": "[5 x i32]", "elem": { <variable>, <variable>, ..., <variable> }}
 *
 *  - "functions": [ "<name>": { "0": <variable>, ... }, ... ]
 *  <name> specifies the name of the function at which to start the analysis. It is allowed to specify
 *  multiple functions (even the same function twice or thrice), each with different arguments. Assign
 *  a function argument by specifying the index in the function property (e.g. "0", "1", ...) followed
 *  by the variable definition (see above). It is not necessary to specify all arguments.
 *
 *  - "simulated": [ "<name>", ... ]
 *  Specify a list of function names that, in case they are called in the code, should be simulated. A
 *  simulated function is then not analyzed, but returns a custom return value. If no simulated handle
 *  is provided for the function, is is completely ignored from the analysis.
 *
 * @param JSON The nlohmann::json object which to use to populate a FunctionContext.
 * @param IA The reference to the InputArguments instance which to populate.
 */
void from_json(const nlohmann::json& JSON, InputArguments& IA, llvm::Module *M);

/**
 * Base template class for all types and additional arguments. To add support for other types, we need
 * to partially specialize (or explicitly) the same class.
 * @tparam T The type of the object to populate.
 * @tparam Args The types of the additional arguments.
 */
template <typename T, typename ... Args>
struct from_json_function {
  /**
   * Function call overload for this base template class.
   * @param JSON The reference to the nlohmann::json instance.
   * @param t The reference to the object to populate.
   * @param args The additional arguments.
   */
  void operator()(const nlohmann::json& JSON, T& t, Args&&... args) {
    from_json(JSON, t, std::forward<Args>(args)...);
  }
};

/**
 * Partial specialization of from_json_function with std::vector elements.
 * @tparam T The type of the std::vector elements.
 * @tparam Args The types of the additional arguments.
 */
template <typename T, typename ... Args>
struct from_json_function<std::vector<T>, Args...> {
  /**
   * Function call overload for this partially specialized template class.
   * @param JSON The reference to the nlohmann::json instance.
   * @param v The reference to the std::vector to populate.
   * @param args The additional arguments.
   */
  void operator()(const nlohmann::json& JSON, std::vector<T>& v, Args&&... args) {
    for (auto &Element : JSON) {
      v.emplace_back(T());
      from_json(Element, v.back(), std::forward<Args>(args)...);
    }
  }
};

/**
 * Core function for all conversion function from nlohmann::json to an arbitrary type T. It works like
 * the adl_string::to_string function, but also works with partial specialization. This works, because
 * we use a template class instead of a template function overload.
 * @tparam T The type of the object to populate.
 * @tparam Args The deduced types of the additional arguments.
 * @param JSON The reference to the nlohmann::json instance.
 * @param t The object to populate.
 * @param args The additional arguments to pass to the custom from_json function.
 */
template <typename T, typename ... Args>
void from_json_impl(const nlohmann::json& JSON, T& t, Args&&... args) {
  return from_json_function<T, Args...>()(JSON, t, std::forward<Args>(args)...);
}

}

/**
 * Globally accessible from_json function for all generic objects.
 * @tparam T The type of the object to populate.
 * @tparam Args The deduced types of the additional arguments.
 * @param JSON The reference to the nlohmann::json instance.
 * @param t The object to populate.
 * @param args The additional arguments to pass to the custom from_json function.
 */
template <typename T, typename ... Args>
void from_json(const nlohmann::json& JSON, T& t, Args&&... args) {
  adl_json::from_json_impl(JSON, t, std::forward<Args>(args)...);
}

}

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H
