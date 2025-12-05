#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "sys_config.h"

#if(USB_ENABLE == TRUE)

// USB 宏定义
#define USB_WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)


/**
 * @brief USB端点号定义
 */
typedef enum {
    PLATFORM_USB_EP0_IN = 0,
    PLATFORM_USB_EP0_OUT = 1,
    PLATFORM_USB_EP1_IN = 2,
    PLATFORM_USB_EP1_OUT = 3,
    PLATFORM_USB_EP2_IN = 4,
    PLATFORM_USB_EP2_OUT = 5,
    PLATFORM_USB_EP3_IN = 6,
    PLATFORM_USB_EP3_OUT = 7,
    PLATFORM_USB_EP4_IN = 8,
    PLATFORM_USB_EP4_OUT = 9,
    PLATFORM_USB_EP5_IN = 10,
    PLATFORM_USB_EP5_OUT = 11,
    PLATFORM_USB_EP6_IN = 12,
    PLATFORM_USB_EP6_OUT = 13,
    
    PLATFORM_USB_EP_MAX = 14//最大端点号
} usb_ep_num_t;

/**
 * @brief USB回调函数类型定义
 */
typedef void (*usb_callback_t)(uint8_t *data, uint16_t len);

/**
 * @brief   平台USB初始化
 * @return  void
 */
extern void platform_usb_init(void);

/**
 * @brief   注册接收回调函数
 * @param   ep_num - 端点号
 * @param   callback - 回调函数指针
 * @return  void
 */
extern void platform_usb_register_in_callback(usb_ep_num_t ep_num, usb_callback_t callback);

/**
 * @brief   往指定端点发送数据
 * @param   ep_num - 端点号
 * @param   data - 数据指针
 * @param   len - 数据长度
 * @return  void
 */
extern void platform_usb_out_send(usb_ep_num_t ep_num, uint8_t *data, uint16_t len);
/**
 * @brief   唤醒主机
 * @return  void
 */
extern void platform_usb_wakeup(void);

//------------------------------编译错误--------------------------------

//------------------------------编译错误--------------------------------

#endif

#ifdef __cplusplus
}
#endif