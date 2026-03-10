// utils/linkedlist.h
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct node node_t;
typedef struct list list_t;

typedef enum _report_t {
    NONE_RPT = 0,
    NORMAL_RPT = 1,
    FUNCTION_RPT = 2,
} report_t;

typedef struct {
    uint16_t key_code;      // 键值
    uint8_t cycle;          // 按下时长（防抖用）
    uint8_t is_report;      // 是否上报
    report_t report_type;   // 报告类型
} bouncing_data_t;

struct node {
    bouncing_data_t data;
    node_t* next;
};

struct list {
    node_t* head;
};

// 链表操作函数
list_t* make_list_proc(void);
void add(uint16_t data, list_t* list);
void del(uint16_t data, list_t* list);
void deactivate(uint16_t data, list_t* list);
void del_all_child(list_t* list);
uint8_t find_key(list_t* list, uint16_t key);
uint8_t find_activate_key(list_t* list, uint16_t key);
void increase_cycle(list_t* list);
void pop_key(list_t* list, uint16_t* key, uint8_t* sz, uint8_t cycle_limit);
void display(list_t* list);
void reverse(list_t* list);
void destroy(list_t* list);
uint8_t insert_key(list_t* list, uint16_t _pre, uint16_t _new);

#endif
