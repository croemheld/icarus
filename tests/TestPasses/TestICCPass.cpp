//
// Created by croemheld on 2/4/24.
//

#include <doctest.h>

#include <icarus/Passes/ICCPass.h>
#include <icarus/Passes/Pass.h>
#include <icarus/Passes/PassArguments.h>

#include <icarus/Logger/Logger.h>

using namespace icarus;

TEST_CASE("Testing ICCPass") {
  PassArguments IPA(SAMPLES_DIR "/indirect_calls.ll", "", 2);

  passes::ICCPass ICCPass;
  CHECK(ICCPass.checkPassArguments(IPA));

  ThreadPool::initialize(IPA.getNumThreads());
  logger::initLoggerOptions("", "");

  int Ret = ICCPass.runAnalysisPass(IPA);

  logger::waitFinished();
  ThreadPool::shutdown();

  CHECK(Ret == 0);
}
