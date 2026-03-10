#ifndef __BAYES_FILTERING_H__
#define __BAYES_FILTERING_H__

// #if TCFG_TOUCHPAD_ENABLE
#include <stdint.h>

#define M_PI  3.14159265358979323846   // pi

#define N_FEATURES 2

// 定义模型参数结构体
typedef struct {
    double priors[2];
    double sigmas[2][2];
    double thetas[2][2];
} bayes_model_t;

// 全局模型参数指针
extern bayes_model_t* current_model;

/// @brief  模型初始化
/// @param model 具体的参数
void bayes_mode_init(bayes_model_t* model);

/// @brief  计算 posterior probabilities
double* compute(double features[N_FEATURES]);

// 动态切换模型
// int predict_key( uint16_t has_key,double features[]);

/// @brief  预测结果
/// @param features 特征向量
/// @return 预测结果
int predict(double features[]);

double* predictProba(double features[]);

// #endif
#endif
