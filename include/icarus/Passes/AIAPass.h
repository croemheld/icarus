//
// Created by croemheld on 04.01.2023.
//

#ifndef ICARUS_PASSES_AIAPASS_H
#define ICARUS_PASSES_AIAPASS_H

#include <llvm/IR/InstVisitor.h>

#include <icarus/Analysis/ProgramContext.h>
#include <icarus/Passes/Pass.h>

#include <icarus/Support/LLVMValue.h>
#include <icarus/Support/Traits.h>

#include <queue>

namespace icarus {

/**
 * Depending on the pass, the AnalysisContext instance may be applied in the default instruction order
 * or, if it is a backward analysis (e.g. Live-Variable Analysis), in reverse order. We assume that an
 * arbitrary pass usually performs forward analyses, so we use DefaultAnalysisIterator as a default param.
 *
 *
 */

class DefaultAnalysisIterator {

  DefaultAnalysisIterator() = default;

public:
  using Iter = llvm::BasicBlock::iterator;

  static llvm::BasicBlock *getEntryBlock(llvm::Function &F) { return F.isDeclaration() ? nullptr : &F.getEntryBlock(); }

  static Iter init(llvm::BasicBlock *BB) { return BB->begin(); }

  static Iter exit(llvm::BasicBlock *BB) { return BB->end(); }
};

class ReverseAnalysisIterator {

  ReverseAnalysisIterator() = default;

public:
  using Iter = llvm::BasicBlock::reverse_iterator;

  static llvm::BasicBlock *getEntryBlock(llvm::Function &F) { return F.isDeclaration() ? nullptr : getExitBlock(F); }

  static Iter init(llvm::BasicBlock *BB) { return BB->rbegin(); }

  static Iter exit(llvm::BasicBlock *BB) { return BB->rend(); }
};

/**
 * Base template class for all passes that wish to use the abstract interpretation-based analyses. The
 * implementation directly controls how instructions are handled in the underlying pass.
 * @tparam SubClass The subclass that specializes this template.
 * @tparam RetTy The return type of each instruction handler. By default it is a void return type.
 */
template <typename Iterator, typename SubClass, typename RetTy = void>
struct AnalysisContext : public llvm::InstVisitor<SubClass, RetTy> {

protected:
  ProgramContext<Iterator> PC;

  /**
   * Creates a new AnalysisContext instance that inherits all methods from the llvm::InstVisitor class
   * template. The class is going to accept either a single llvm::BasicBlock or a llvm::Function which
   * contains an entry basic block for the analysis.
   */
  AnalysisContext() = default;
};

/**
 * Alias to determine whether or not a class inherits from the AIAContext above. This is necessary for
 * ensuring the AIAPass below receives a correct template argument at compile-time.
 */
template <typename AIAContextImpl>
using enable_if_aiacontext = std::enable_if_t<is_template_base_of<AnalysisContext, AIAContextImpl>::value, bool>;

/*
 * Abstract Interpretation Analysis (AIA) Pass
 */

/**
 * Base class for all abstract interpretation-based analyses. The core logic of this class is provided
 * by the AIAContext (AnalysisContext) subclass passed as the template parameter. In order to use this
 * class, one has to inherit from both Pass and ThreadedAIAPass.
 * @tparam AIAContextImpl The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContextImpl, bool Threaded, typename Iterator> class ThreadedAIAPass : public Pass {

protected:
  void scheduleAnalysisContext(AIAContextImpl &AIAContext) {}

  void ProgramContextWorker(ProgramContext<Iterator> &PC) {
    while (!PC.isStackEmpty()) {
      auto &FC = PC.getCurrentFunctionStack();
      llvm::Instruction &I = FC->nextInstruction();
      INFO_WITH("iaa", "Interpreting: ", I);
      AIAContextImpl::visit(I);
    }
  }

public:
  /**
   * Creates a new abstract interpretation-based analysis pass with the provided AnalysisContext. With
   * the AnalysisContext template argument, we implement the basic techniques for the pass.
   */
  ThreadedAIAPass() = default;
};

/**
 * Partial specialization for all abstract interpretation-based analyses, that do NOT support multiple
 * threads during the analysis (logger threads not counted).
 * @tparam AIAContextImpl The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContextImpl, typename Iterator>
class ThreadedAIAPass<AIAContextImpl, false, Iterator> : public Pass {

  std::queue<std::unique_ptr<Task>> TaskQueue;

public:
  /**
   * Schedule method for ThreadedAIAPass subclasses that do NOT support multithreading. It essentially
   * uses the same method signature from the ThreadPool class except that the queue is not thread-safe
   * as there is only one thread running during the analysis (logger threads not counted).
   * @tparam Func The type of the function to submit.
   * @tparam Args The variadic types of the function arguments.
   * @tparam RetTy The return type of the function.
   * @tparam PTask Type of the packaged task wrapping the callable function.
   * @tparam TaskT The ThreadTask type with the template parameter from the function.
   * @param Function The function to execute in a thread pool task.
   * @param args The arguments to pass to the function.
   */
  template <typename Func, typename... Args,
            typename RetTy = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>,
            typename PTask = std::packaged_task<RetTy()>, typename TaskT = ThreadTask<PTask>>
  void schedule(Func &&Function, Args &&...args) {
    auto Task = std::bind(std::forward<Func>(Function), std::forward<Args>(args)...);
    std::packaged_task<RetTy()> PackagedTask(std::move(Task));
    TaskQueue.emplace(std::make_unique<TaskT>(std::move(PackagedTask)));
  }
};

/**
 * Partial specialization for all abstract interpretation-based analyses, that support multithreading.
 * @tparam AIAContextImpl The AnalysisContext subclass which implements the core algorithm.
 */
template <typename AIAContextImpl, typename Iterator>
class ThreadedAIAPass<AIAContextImpl, true, Iterator> : public Pass {

  ThreadPool TP;

protected:
  /**
   * Initialize the pass thread pool with the specified number of threads.
   * @param NumThreads The number of worker threads to initialize.
   */
  void initializeThreadPool(unsigned NumThreads) { TP.initialize(NumThreads); }

public:
  /**
   * Schedule method for ThreadedAIAPass subclasses that support multithreading.
   * @tparam Func The type of the function to submit.
   * @tparam Args The variadic types of the function arguments.
   * @param Function The function to execute in a thread pool task.
   * @param args The arguments to pass to the function.
   */
  template <typename Func, typename... Args> void schedule(Func &&Function, Args &&...args) {
    TP.submit(Function, std::forward<Args>(args)...);
  }
};

} // namespace icarus

#endif // ICARUS_PASSES_AIAPASS_H
