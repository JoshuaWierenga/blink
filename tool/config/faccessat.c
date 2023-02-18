// tests for faccessat
// missing on Windows
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (faccessat(AT_FDCWD, "blink", X_OK, 0) != 0) return 1;
  return 0;
}
