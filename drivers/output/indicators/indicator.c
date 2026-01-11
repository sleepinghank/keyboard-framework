/* Copyright 2025 @ keyboard-framework
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file indicator.c
 * @brief 指示灯应用层实现
 *
 * 实现对业务层的API接口
 */

#include "indicator.h"
#include "indicator_hal.h"
#include "indicator_driver.h"
#include <string.h>

/* ========== 初始化 ========== */

int indicator_init(indicator_t* ind, pin_t pin, bool active_high) {
    if (ind == NULL) return -1;

    // 清零结构
    memset(ind, 0, sizeof(indicator_t));

    // 硬件配置
    ind->hw.pin         = pin;
    ind->hw.active_high = active_high;

    // 初始化硬件
    ind_hal_config_t hal_cfg = {
        .pin = pin,
        .active_high = active_high
    };
    ind_hal_init(&hal_cfg);

    // 默认配置
    ind->config.mode     = IND_MODE_OFF;
    ind->config.on_time  = 0;
    ind->config.off_time = 0;
    ind->config.duration = 0;
    ind->config.repeat   = 0;

    return 0;
}

void indicator_deinit(indicator_t* ind) {
    if (ind == NULL) return;

    // 停止指示
    indicator_stop(ind);

    // 反初始化硬件
    ind_hal_deinit(ind->hw.pin);
}

/* ========== 控制 ========== */

int indicator_start(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return -1;

    ind_driver_start(ind, config);
    return 0;
}

void indicator_stop(indicator_t* ind) {
    if (ind == NULL) return;

    ind_driver_stop(ind);
}

int indicator_update(indicator_t* ind, const ind_config_t* config) {
    if (ind == NULL || config == NULL) return -1;

    // 停止当前指示
    ind_driver_stop(ind);

    // 启动新配置
    ind_driver_start(ind, config);

    return 0;
}

/* ========== 状态查询 ========== */

bool indicator_is_running(const indicator_t* ind) {
    if (ind == NULL) return false;
    return (ind->runtime.state == IND_STATE_RUNNING);
}

/* ========== 任务 ========== */

bool indicator_task(indicator_t* ind) {
    if (ind == NULL) return false;
    return ind_driver_task(ind);
}
