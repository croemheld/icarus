#include <stdio.h>
#include <stdlib.h>

int custom_add(int a, int b) {
  return a + b;
}

int custom_sub(int a, int b) {
  return a - b;
}

int custom_mul(int a, int b) {
  return a * b;
}

int custom_div(int a, int b) {
  return a / b;
}

int (*func[4])(int, int) = {custom_add, custom_sub, custom_mul, custom_div};

int main(int argc, char *argv[]) {
  return func[rand() % 4](4, 2);
}
