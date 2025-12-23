#include "math.h"
#include "bayes_filtering.h"
#include "string.h"
#include <stdint.h>

// 全局模型参数指针
bayes_model_t* current_model = NULL;

// 默认模型参数
static bayes_model_t default_model = {
    .priors = {0.3161764705882353, 0.6838235294117647},
    .sigmas = {{62379.262058067005, 80.2515446278459}, {241619.59808312583, 27.030716511705883}},
    .thetas = {{2019.82915921288, 17.615384615384617}, {725.8449131513647, 10.91852770885029}}
};

// 设置当前模型
void bayes_mode_init(bayes_model_t* model) {
    if (model != NULL) {
        current_model = model;
    } else {
        current_model = &default_model;
    }
}

double* compute(double features[N_FEATURES]){
    static double likelihoods[N_FEATURES];
    int i = 0;
    int j = 0;
    double nij;
    for (i = 0; i < N_FEATURES; ++i) {
        double sum = 0.;
        for (j = 0; j < N_FEATURES; ++j) {
            sum +=log(2. * M_PI * current_model -> sigmas[i][j]);
        }
        nij = -0.5 * sum;
        sum = 0.;
        for (j = 0; j < N_FEATURES; ++j) {
            sum += pow(features[j] - current_model -> thetas[i][j], 2.) / current_model -> sigmas[i][j];
        }
        nij -= 0.5*sum;
        likelihoods[i] = log(current_model -> priors[i]) + nij;
    }
    return likelihoods;
}

// 寻找明确的特征分类
static int findMax(double nums[]) {
    int idx = 0;
    int i = 0;
    for (i = 0; i < N_FEATURES; ++i) {
        idx = nums[i] > nums[idx] ? i : idx;
    }
    return idx;
}

static double logSumExp(double nums[]){
    double max = nums[findMax(nums)];
    double sum = 0.;
    int i = 0 , il = N_FEATURES;
    for (i = 0 , il = N_FEATURES; i < il; i++) {
        sum += exp(nums[i] - max);
    }
    return max - log(sum);
}

// 直接预测分类
int predict(double features[]){
    return findMax((double *) compute(features));
}

// 预测每个分类的比值
double* predictProba(double features[]) {
    double* jll = compute(features);
    double sum = logSumExp(jll);
    int i = 0;
    for (i = 0; i < N_FEATURES; i++) {
        jll[i] = exp(jll[i] - sum);
    }
    return jll;
}

int predict_secondary_data(double likelihoods[]){
    return findMax(likelihoods);
}

double* predictProba_secondary_data(double jll[]) {
    double sum = logSumExp(jll);
    int i = 0;
    for (i = 0; i < 2; i++) {
        jll[i] = exp(jll[i] - sum);
    }
    return jll;
}
