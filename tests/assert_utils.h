#ifndef ASSERT_UTILS_H
#define ASSERT_UTILS_H

#include <stdio.h>

#define ASSERT_TRUE(cond, msg) assert_true((int)(cond), (msg), __FILE__, __LINE__)
#define ASSERT_FLOAT_EQ(a, b, eps, msg) assert_float_eq((a), (b), (eps), (msg), __FILE__, __LINE__)

int assert_true(int cond, const char *msg, const char *file, int line);
int assert_float_eq(float a, float b, float eps, const char *msg, const char *file, int line);

#endif /* ASSERT_UTILS_H */
