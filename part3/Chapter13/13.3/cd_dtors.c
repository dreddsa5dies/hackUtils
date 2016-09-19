#include <stdio.h>

static void start(void) __attribute__ ((constructor));
static void stop(void) __attribute__ ((destructor));

int main() {
  printf("this is main()\n");
  return 0;
}

void start(void) {
  printf("this is start()\n");
}

void stop(void) {
  printf("this is stop()\n");
}
