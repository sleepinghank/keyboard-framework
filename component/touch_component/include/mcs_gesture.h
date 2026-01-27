//
// Created by hank on 2025/1/27.
//

#ifndef C_MCS_GESTURE_H
#define C_MCS_GESTURE_H
#include <stdint.h>

#define MAX_FRAMES 100
#define FIXED_LENGTH 8

/* 结构体定义 */
typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    Point points[MAX_FRAMES];      // 动态指针      // 原始坐标序列
    int length;         // 原始长度
    Point inv_points[MAX_FRAMES]; // 平移不变处理后的坐标
    Point resampled[FIXED_LENGTH]; // 固定长度坐标
    double feature[2*FIXED_LENGTH]; // 最终特征向量
} GestureData;

// 手势类型枚举
typedef enum {
    GESTURE_S = 1,
    GESTURE_Z = 2,
    GESTURE_N = 3,
    GESTURE_W = 4,
    GESTURE_UNKNOWN = 0
} GestureType;

// 距离阈值结构体
typedef struct {
    double s_threshold;
    double z_threshold;
    double n_threshold;
    double w_threshold;
} distance_thresholds_t;



/**
 * 识别手势类型
 * @param input_points 输入的手势坐标点数组
 * @param point_count 坐标点数量
 * @return 识别出的手势类型
 */
GestureType recognize_gesture(Point* input_points, uint8_t point_count);

/// @brief  设置余弦相似度阈值，默认值0.75
/// @param threshold 阈值，范围0-1
void set_similarity_threshold(double threshold);

/// @brief 设置距离阈值 有默认值
/// @param thresholds 针对每个手势可以设置单独的距离阈值
void set_distance_thresholds(const distance_thresholds_t* thresholds);
#endif //C_MCS_GESTURE_H
