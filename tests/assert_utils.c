#include "assert_utils.h"

int assert_true(int cond, const char *msg, const char *file, int line)
{
  if (!cond)
  {
    printf("[FAIL] %s  (%s:%d)\n", msg, file, line);
    return 1;
  }
  printf("[PASS] %s\n", msg);
  return 0;
}

int assert_float_eq(float a, float b, float eps, const char *msg, const char *file, int line)
{
  float diff = a - b;
  if (diff < 0.0f) diff = -diff;
  if (diff > eps)
  {
    printf("[FAIL] %s  (got %.6f, expected %.6f)  (%s:%d)\n", msg, a, b, file, line);
    return 1;
  }
  printf("[PASS] %s\n", msg);
  return 0;
}
