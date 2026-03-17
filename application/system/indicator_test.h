#pragma once

#include "sys_config.h"

#ifdef INDICATOR_TEST_ENABLE
void indicator_test(void);
#else
#define indicator_test() ((void)0)
#endif
