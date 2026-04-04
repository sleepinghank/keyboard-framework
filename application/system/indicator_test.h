#pragma once

#include "kb904/config_product.h"

#ifdef INDICATOR_TEST_ENABLE
void indicator_test(void);
#else
#define indicator_test() ((void)0)
#endif
