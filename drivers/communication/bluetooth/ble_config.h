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
 * @file ble_config.h
 * @brief 蓝牙模块配置
 *
 * 配置加载顺序：
 * 1. 产品配置 (keyboards/kb904/config_product.h) - 定义覆盖值
 * 2. 默认配置 (defaults/ble_defaults.h) - 使用 #ifndef 保护
 * 3. 本文件 - 蓝牙驱动配置
 */

#pragma once

#include "kb904/config_product.h"

