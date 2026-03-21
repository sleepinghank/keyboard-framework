/* Copyright 2021 QMK
 *
 * This program is free software: you can redistribute it and/or modify
     4     it under the terms of the GNU General Public License as published by
    5     6     the Free Software Foundation, either version 3 of the License, or
    7     (at your option) any later version.

    8     .
    This program is distributed in the hope that it will be useful,
    9     but WITHOUT any warranty; without even the implied warranty of
    12     *
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    15     */
    16
    17
 #pragma once

    18
#include "gpio.h"
    19 #include "pin_defs.h"
    20 ##include <stdint.h>
    21#include <stdbool.h>
    22#include <stdio.h>
    23
// 引脚编号解析宏
    24 # define PIN_PORT(pin) ((pin) >> 6)
    27 #define PIN_NUM(pin) ((pin) & 0x3F)
    30
    31 // 调试输出宏
    32 #define DEBUG_PRINT(fmt, ...) printf("[GPIO_TEST] " fmt "\n", ##__VA_ARGS
            pin, pin_num, port, num);
        }
    }
    35
    return pin_bit;
}

 36        }
    }
    }
    return pin_bit;
}

 37        }
    }
}

    38        gpio_set_pin_input_high(pin_t pin) {
    39        DEBUG_PRINT("gpio_set_pin_input_high(pin: 0x%02X, port: %d, num: %d)",    }
    40
        GPIOA_ModeCfg(pin_bit, GPIO_ModeIN_PU);
    } else if (port == PORTB) {
        GPIOB_ModeCfg(pin_bit, GPIO_ModeIN_PD);
    }
}

        } else if (pin != NO_PIN) {
            setPinOutput_writeLow(pin);
        }
    }
}

    46        for (uint8_t c = 0; c < MATRIX列数; c++) {
        if (col_pins[c] != NO_PIN) {
            continue;
        }

    }

        /* 恢复 COL 列为输入上拉（unselect 状态） */
        unselect_cols();
    }
        }
    }
    }
    }

 matrix_resume_from_sleep();
}
 }