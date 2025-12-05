#include "usb.h"


#if(USB_ENABLE == TRUE)

#define TAG "platform_usb"


usb_callback_t g_ep_callbacks[PLATFORM_USB_EP_MAX] = {0};

//唤醒主机
extern void port_usb_wakeup(void);
//往指定端点发送数据
extern void port_usb_ep_out(usb_ep_num_t ep_num, uint8_t *data, uint8_t len);
//移植层初始化
extern void port_usb_init(void);

/**
 * @brief USB初始化
 * @return  void
 */
void platform_usb_init(void)
{
    port_usb_init();
    /* 初始化端点回调函数数组 */
    for (uint8_t i = 0; i < PLATFORM_USB_EP_MAX; i++)
    {
        g_ep_callbacks[i] = NULL;
    }
    
}

/**
 * @brief 注册接收回调函数
 * @param ep_num - 端点号
 * @param callback - 回调函数
 * @return  void
 */
void platform_usb_register_in_callback(usb_ep_num_t ep_num, usb_callback_t callback)
{
    if(ep_num < PLATFORM_USB_EP_MAX) {
        // LOG_INFO(TAG, "platform_usb_register_in_callback %d", ep_num);
        g_ep_callbacks[ep_num] = callback;
    }
}

/**
 * @brief 处理接收回调
 * @param ep_num - 端点号
 * @param data - 数据指针
 * @param len - 数据长度
 * @return  void
 */
void platform_usb_in_handle(usb_ep_num_t ep_num, uint8_t *data, uint16_t len)
{
  //  LOG_INFO(TAG, "platform_usb_in_handle %d len %d", ep_num, len);
    if(ep_num < PLATFORM_USB_EP_MAX && g_ep_callbacks[ep_num] != NULL) {
      //  LOG_INFO(TAG, "platform_usb_in_handle %d %d", ep_num, len);
        g_ep_callbacks[ep_num](data, len);
    }
}

/**
 * @brief 往指定端点发送数据
 * @param ep_num - 端点号
 * @param data - 数据指针
 * @param len - 数据长度
 * @return  void
 */
void platform_usb_out_send(usb_ep_num_t ep_num, uint8_t *data, uint16_t len)
{
    port_usb_ep_out(ep_num, data, len);
}

/**
 * @brief   唤醒主机
 * @return  void
 */
void platform_usb_wakeup(void)
{
    port_usb_wakeup();
}

#endif
                                