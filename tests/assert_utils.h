#ifndef ASSERT_UTILS_H
#define ASSERT_UTILS_H

#include <stdio.h>
#include "miniaudio.h"
#include <assert.h>

// The macro stays in the header so it can be "pasted" into your tests.
#define ASSERT_SUCCESS(result, msg) \
  handle_assertion(result, msg, __FILE__, __LINE__)

// We declare a helper function that will do the heavy lifting.
int handle_assertion(ma_result result, const char *msg, const char *file, int line);

#endif
