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
 * @file indicator_config.h
 * @brief [已废弃] 指示灯配置兼容层
 *
 * @deprecated 此文件已废弃，请直接使用：
 *   - KB904 产品: #include "keyboards/kb904/config.h"
 *     （config.h 会自动包含 config_hw.h 和 indicator.c）
 *
 * 迁移指南: docs/plans/2026-03-19-keyboards-config-refactor-design.md
 */

#pragma once

#warning "indicator_config.h is deprecated. Include keyboards/kb904/config.h instead."

/* ========== 兼容层：转发到产品配置 ========== */
#include "kb904/config.h"
