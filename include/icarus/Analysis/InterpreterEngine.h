//
// Created by croemheld on 01.02.2023.
//

#ifndef ICARUS_ANALYSIS_INTERPRETERENGINE_H
#define ICARUS_ANALYSIS_INTERPRETERENGINE_H

#include <icarus/Analysis/ExecutionEngine.h>

namespace icarus {

/**
 * Base class for interpreting a program. This class is essentially equal to llvm::ExecutionEngine and
 * llvm::Interpreter. The major difference is that the implementation is made for an interpreter only.
 * If the need arises for another type of execution engine (e.g. LLVM's MCJIT), we will need to create
 * a new class for this and separate the code.
 *
 * The original interpreter also has the disadvantage that it: 1) moves ownership of each llvm::Module
 * to itself (why though?), which would break icarus as each module is stored by PassArguments. Unfor-
 * tunately, and 2), the code of the class itself is not exposed via LLVM's API by header files, which
 * makes it impossible to use the class or extend it. Thus, we have to essentially rewrite the classes
 * to match our purpose, which also means that most of the code is copied 1:1 with minor changes.
 */
class InterpreterEngine : public ExecutionEngine {};

} // namespace icarus

#endif // ICARUS_ANALYSIS_INTERPRETERENGINE_H
