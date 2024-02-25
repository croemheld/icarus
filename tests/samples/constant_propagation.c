#include <stdio.h>

void func(int *var) {
  if (*var > 2) {
    *var = *var * 2;
  } else {
    *var = *var * 3;
  }
}

int main(int argc, char *argv[]) {
  int var = argc / 2;
  func(&var);
  return var + 5;
}
