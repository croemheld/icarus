//
// Created by croemheld on 28.05.2023.
//

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char *argv[]) {
  doctest::Context context;

  context.setOption("order-by", "name");
  context.applyCommandLine(argc, argv);

  return context.run();
}