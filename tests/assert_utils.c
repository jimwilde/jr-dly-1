#include "assert_utils.h"

int handle_assertion(ma_result result, const char *msg, const char *file, int line)
{
  if (result != MA_SUCCESS)
  {
    printf("[FAIL] %s\n       File: %s, Line: %d (Result: %d)\n", msg, file, line, result);
    return 1; // Signal failure
  }
  else
  {
    printf("[PASS] %s\n", msg);
    return 0; // Signal success
  }
}
