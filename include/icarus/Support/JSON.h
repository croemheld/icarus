//
// Created by croemheld on 24.01.2023.
//

#ifndef ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H
#define ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H

#include <llvm/ADT/ArrayRef.h>

#include <icarus/Logger/Logger.h>

#include <nlohmann/json.hpp>

namespace icarus {

struct VariableDef;
struct CallContext;
struct InputArguments;
struct IcarusModule;

namespace adl_json {

/**
 * Conversion function for nlohmann::json -> VariableDef.
 * @param JSON The reference to the nlohmann::json instance.
 * @param VD The reference to the VariableRef instance.
 * @param IM Pointer to the IcarusModule in order to completely initialize the class.
 */
void from_json(const nlohmann::json& JSON, VariableDef& VD, IcarusModule *IM);

/**
 * Conversion function for nlohmann::json -> CallContext.
 * @param JSON The reference to the nlohmann::json instance.
 * @param VD The reference to the CallContext instance.
 * @param IM Pointer to the IcarusModule in order to completely initialize the class.
 */
void from_json(const nlohmann::json& JSON, CallContext& CC, IcarusModule *IM);

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
 *  - "functions": [ { "func": "<name>", "0": <variable>, "1": <variable>, ... }, ... ]
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
 * @param IM Pointer to the IcarusModule in order to completely initialize the class.
 */
void from_json(const nlohmann::json& JSON, InputArguments& IA, IcarusModule *IM);

/*
 * WARNING: All partially specialized template classes will fail if the provided type is NOT trivially
 * constructible. Because of variadic templates, there doesn't seem to be a way for a static check.
 */

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
    INFO(JSON);
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
   * Function call overload for this partially specialized template class. This function assumes, that
   * the provided JSON object is iterable, i.e. it will fail if the user provides a JSON object, which
   * does not match a type that can be natively converted into a vector.
   * @param JSON The reference to the nlohmann::json instance.
   * @param v The reference to the std::vector to populate.
   * @param args The additional arguments.
   */
  void operator()(const nlohmann::json& JSON, std::vector<T>& v, Args&&... args) {
    for (auto &Element : JSON) {
      v.push_back(T());
      from_json(Element, v.back(), std::forward<Args>(args)...);
    }
  }
};

/**
 * Partial specialization of from_json_function with std::map elements.
 * @tparam V The type of the values stored in the map.
 * @tparam Args The types of the additional arguments.
 */
template <typename V, typename ... Args>
struct from_json_function<std::map<std::string, V>, Args...> {
  /**
   * Function call overload for this partially specialized template class. This function assumes, that
   * the provided JSON object is an actual object with key-value ("key" : <value>) pairs.
   * @param JSON The reference to the nlohmann::json instance.
   * @param m  The reference to the std::map to populate.
   * @param args The additional arguments.
   */
  void operator()(const nlohmann::json& JSON, std::map<std::string, V>& m, Args&&... args) {
    /*
     * By the JSON specification the identifiers (keys) are always strings. Thus, we can simply insert
     * the Key variable directly for the lookup and don't need to explicitly convert it to a string.
     */
    for (auto &[Key, Val] : JSON.items()) {
      m[Key] = V();
      from_json(Val, m[Key], std::forward<Args>(args)...);
    }
  }
};

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
  adl_json::from_json_function<T, Args...>()(JSON, t, std::forward<Args>(args)...);
}

}

#endif // ICARUS_INCLUDE_ICARUS_SUPPORT_JSON_H
