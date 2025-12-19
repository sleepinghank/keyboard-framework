/********************************** (C) COPYRIGHT *******************************
 * 文件名称: hiddev.h
 * 作者: WCH
 * 版本: V1.0
 * 日期: 2018/12/11
 * 描述: HID设备相关定义和接口声明
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * 注意: 本软件(修改或未修改)和二进制文件仅用于
 * 南京沁恒微电子生产的微控制器。
 *******************************************************************************/

#ifndef HIDDEV_H
#define HIDDEV_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * 包含头文件
 */

/*********************************************************************
 * 宏定义
 */

/*********************************************************************
 * 常量定义
 */

// HID设备参数
#define HIDDEV_ERASE_ALLBONDS             0     // 擦除所有绑定设备。仅写操作。无大小。

// HID读写操作
#define HID_DEV_OPER_WRITE                0     // 写操作
#define HID_DEV_OPER_READ                 1     // 读操作
#define HID_DEV_OPER_ENABLE               2     // 启用报告ID的通知
#define HID_DEV_OPER_DISABLE              3     // 禁用报告ID的通知

// HID回调事件
#define HID_DEV_SUSPEND_EVT               0     // HID挂起
#define HID_DEV_EXIT_SUSPEND_EVT          1     // HID退出挂起
#define HID_DEV_SET_BOOT_EVT              2     // HID设置启动模式
#define HID_DEV_SET_REPORT_EVT            3     // HID设置报告模式

/* HID报告类型 */
#define HID_REPORT_TYPE_INPUT             1     // 输入报告
#define HID_REPORT_TYPE_OUTPUT            2     // 输出报告
#define HID_REPORT_TYPE_FEATURE           3     // 特征报告

/* HID信息标志 */
#define HID_FLAGS_REMOTE_WAKE             0x01  // 远程唤醒
#define HID_FLAGS_NORMALLY_CONNECTABLE    0x02  // 正常可连接

/* 控制点命令 */
#define HID_CMD_SUSPEND                   0x00  // 挂起
#define HID_CMD_EXIT_SUSPEND              0x01  // 退出挂起

/* HID协议模式值 */
#define HID_PROTOCOL_MODE_BOOT            0x00  // 启动协议模式
#define HID_PROTOCOL_MODE_REPORT          0x01  // 报告协议模式

/* 属性值长度 */
#define HID_PROTOCOL_MODE_LEN             1     // HID协议模式长度
#define HID_INFORMATION_LEN               4     // HID信息长度
#define HID_REPORT_REF_LEN                2     // HID报告引用描述符长度
#define HID_EXT_REPORT_REF_LEN            2     // 外部报告引用描述符长度

// HID键盘/小键盘使用ID(USB HID使用表规范中可用代码的子集)
#define HID_KEYBOARD_RESERVED             0     // 0x00 - 无事件指示
#define HID_KEYBOARD_A                    4     // 0x04 - 键盘a和A
#define HID_KEYBOARD_B                    5     // 0x05 - 键盘b和B
#define HID_KEYBOARD_C                    6     // 0x06 - 键盘c和C
#define HID_KEYBOARD_D                    7     // 0x07 - 键盘d和D
#define HID_KEYBOARD_E                    8     // 0x08 - 键盘e和E
#define HID_KEYBOARD_F                    9     // 0x09 - 键盘f和F
#define HID_KEYBOARD_G                    10    // 0x0A - 键盘g和G
#define HID_KEYBOARD_H                    11    // 0x0B - 键盘h和H
#define HID_KEYBOARD_I                    12    // 0x0C - 键盘i和I
#define HID_KEYBOARD_J                    13    // 0x0D - 键盘j和J
#define HID_KEYBOARD_K                    14    // 0x0E - 键盘k和K
#define HID_KEYBOARD_L                    15    // 0x0F - 键盘l和L
#define HID_KEYBOARD_M                    16    // 0x10 - 键盘m和M
#define HID_KEYBOARD_N                    17    // 0x11 - 键盘n和N
#define HID_KEYBOARD_O                    18    // 0x12 - 键盘o和O
#define HID_KEYBOARD_P                    19    // 0x13 - 键盘p和P
#define HID_KEYBOARD_Q                    20    // 0x14 - 键盘q和Q
#define HID_KEYBOARD_R                    21    // 0x15 - 键盘r和R
#define HID_KEYBOARD_S                    22    // 0x16 - 键盘s和S
#define HID_KEYBOARD_T                    23    // 0x17 - 键盘t和T
#define HID_KEYBOARD_U                    24    // 0x18 - 键盘u和U
#define HID_KEYBOARD_V                    25    // 0x19 - 键盘v和V
#define HID_KEYBOARD_W                    26    // 0x1A - 键盘w和W
#define HID_KEYBOARD_X                    27    // 0x1B - 键盘x和X
#define HID_KEYBOARD_Y                    28    // 0x1C - 键盘y和Y
#define HID_KEYBOARD_Z                    29    // 0x1D - 键盘z和Z
#define HID_KEYBOARD_1                    30    // 0x1E - 键盘1和!
#define HID_KEYBOARD_2                    31    // 0x1F - 键盘2和@
#define HID_KEYBOARD_3                    32    // 0x20 - 键盘3和#
#define HID_KEYBOARD_4                    33    // 0x21 - 键盘4和$
#define HID_KEYBOARD_5                    34    // 0x22 - 键盘5和%
#define HID_KEYBOARD_6                    35    // 0x23 - 键盘6和^
#define HID_KEYBOARD_7                    36    // 0x24 - 键盘7和&
#define HID_KEYBOARD_8                    37    // 0x25 - 键盘8和*
#define HID_KEYBOARD_9                    38    // 0x26 - 键盘9和(
#define HID_KEYBOARD_0                    39    // 0x27 - 键盘0和)
#define HID_KEYBOARD_RETURN               40    // 0x28 - 键盘回车(ENTER)
#define HID_KEYBOARD_ESCAPE               41    // 0x29 - 键盘ESC
#define HID_KEYBOARD_DELETE               42    // 0x2A - 键盘删除(退格)
#define HID_KEYBOARD_TAB                  43    // 0x2B - 键盘Tab
#define HID_KEYBOARD_SPACEBAR             44    // 0x2C - 键盘空格
#define HID_KEYBOARD_MINUS                45    // 0x2D - 键盘-和_
#define HID_KEYBOARD_EQUAL                46    // 0x2E - 键盘=和+
#define HID_KEYBOARD_LEFT_BRKT            47    // 0x2F - 键盘[和{
#define HID_KEYBOARD_RIGHT_BRKT           48    // 0x30 - 键盘]和}
#define HID_KEYBOARD_BACK_SLASH           49    // 0x31 - 键盘\和|
#define HID_KEYBOARD_SEMI_COLON           51    // 0x33 - 键盘;和:
#define HID_KEYBOARD_SGL_QUOTE            52    // 0x34 - 键盘'和"
#define HID_KEYBOARD_GRV_ACCENT           53    // 0x35 - 键盘重音符和波浪号
#define HID_KEYBOARD_COMMA                54    // 0x36 - 键盘,和<
#define HID_KEYBOARD_DOT                  55    // 0x37 - 键盘.和>
#define HID_KEYBOARD_FWD_SLASH            56    // 0x38 - 键盘/和?
#define HID_KEYBOARD_CAPS_LOCK            57    // 0x39 - 键盘大写锁定
#define HID_KEYBOARD_F1                   58    // 0x3A - 键盘F1
#define HID_KEYBOARD_F2                   59    // 0x3B - 键盘F2
#define HID_KEYBOARD_F3                   60    // 0x3C - 键盘F3
#define HID_KEYBOARD_F4                   61    // 0x3D - 键盘F4
#define HID_KEYBOARD_F5                   62    // 0x3E - 键盘F5
#define HID_KEYBOARD_F6                   63    // 0x3F - 键盘F6
#define HID_KEYBOARD_F7                   64    // 0x40 - 键盘F7
#define HID_KEYBOARD_F8                   65    // 0x41 - 键盘F8
#define HID_KEYBOARD_F9                   66    // 0x42 - 键盘F9
#define HID_KEYBOARD_F10                  67    // 0x43 - 键盘F10
#define HID_KEYBOARD_F11                  68    // 0x44 - 键盘F11
#define HID_KEYBOARD_F12                  69    // 0x45 - 键盘F12
#define HID_KEYBOARD_PRNT_SCREEN          70    // 0x46 - 键盘打印屏幕
#define HID_KEYBOARD_SCROLL_LOCK          71    // 0x47 - 键盘滚动锁定
#define HID_KEYBOARD_PAUSE                72    // 0x48 - 键盘暂停
#define HID_KEYBOARD_INSERT               73    // 0x49 - 键盘插入
#define HID_KEYBOARD_HOME                 74    // 0x4A - 键盘Home
#define HID_KEYBOARD_PAGE_UP              75    // 0x4B - 键盘上一页
#define HID_KEYBOARD_DELETE_FWD           76    // 0x4C - 键盘向前删除
#define HID_KEYBOARD_END                  77    // 0x4D - 键盘End
#define HID_KEYBOARD_PAGE_DOWN            78    // 0x4E - 键盘下一页
#define HID_KEYBOARD_RIGHT_ARROW          79    // 0x4F - 键盘右箭头
#define HID_KEYBOARD_LEFT_ARROW           80    // 0x50 - 键盘左箭头
#define HID_KEYBOARD_DOWN_ARROW           81    // 0x51 - 键盘下箭头
#define HID_KEYBOARD_UP_ARROW             82    // 0x52 - 键盘上箭头
#define HID_KEYBPAD_NUM_LOCK              83    // 0x53 - 小键盘数字锁定和清除
#define HID_KEYBPAD_DIVIDE                84    // 0x54 - 小键盘/
#define HID_KEYBOARD_MULTIPLY             85    // 0x55 - 小键盘*
#define HID_KEYBOARD_SUBTRACT             86    // 0x56 - 小键盘-
#define HID_KEYBPAD_ADD                   87    // 0x57 - 小键盘+
#define HID_KEYBPAD_ENTER                 88    // 0x58 - 小键盘回车
#define HID_KEYBPAD_1                     89    // 0x59 - 小键盘1和End
#define HID_KEYBPAD_2                     90    // 0x5A - 小键盘2和下箭头
#define HID_KEYBPAD_3                     91    // 0x5B - 小键盘3和下一页
#define HID_KEYBPAD_4                     92    // 0x5C - 小键盘4和左箭头
#define HID_KEYBPAD_5                     93    // 0x5D - 小键盘5
#define HID_KEYBPAD_6                     94    // 0x5E - 小键盘6和右箭头
#define HID_KEYBPAD_7                     95    // 0x5F - 小键盘7和Home
#define HID_KEYBPAD_8                     96    // 0x60 - 小键盘8和上箭头
#define HID_KEYBPAD_9                     97    // 0x61 - 小键盘9和上一页
#define HID_KEYBPAD_0                     98    // 0x62 - 小键盘0和插入
#define HID_KEYBPAD_DOT                   99    // 0x63 - 小键盘.和删除
#define HID_KEYBOARD_MUTE                 127   // 0x7F - 键盘静音
#define HID_KEYBOARD_VOLUME_UP            128   // 0x80 - 键盘音量增大
#define HID_KEYBOARD_VOLUME_DOWN          129   // 0x81 - 键盘音量减小
#define HID_KEYBOARD_LEFT_CTRL            224   // 0xE0 - 键盘左Control
#define HID_KEYBOARD_LEFT_SHIFT           225   // 0xE1 - 键盘左Shift
#define HID_KEYBOARD_LEFT_ALT             226   // 0xE2 - 键盘左Alt
#define HID_KEYBOARD_LEFT_GUI             227   // 0xE3 - 键盘左GUI
#define HID_KEYBOARD_RIGHT_CTRL           228   // 0xE4 - 键盘右Control
#define HID_KEYBOARD_RIGHT_SHIFT          229   // 0xE5 - 键盘右Shift
#define HID_KEYBOARD_RIGHT_ALT            230   // 0xE6 - 键盘右Alt
#define HID_KEYBOARD_RIGHT_GUI            231   // 0xE7 - 键盘右GUI

#define HID_MOUSE_BUTTON_LEFT             253   // 鼠标左键
#define HID_MOUSE_BUTTON_MIDDLE           254   // 鼠标中键
#define HID_MOUSE_BUTTON_RIGHT            255   // 鼠标右键

// HID消费者使用ID(USB HID使用表规范中可用代码的子集)
#define HID_CONSUMER_POWER                48    // 0x30 - 电源
#define HID_CONSUMER_RESET                49    // 0x31 - 重置
#define HID_CONSUMER_SLEEP                50    // 0x32 - 睡眠

#define HID_CONSUMER_MENU                 64    // 0x40 - 菜单
#define HID_CONSUMER_SELECTION            128   // 0x80 - 选择
#define HID_CONSUMER_ASSIGN_SEL           129   // 0x81 - 分配选择
#define HID_CONSUMER_MODE_STEP            130   // 0x82 - 模式步进
#define HID_CONSUMER_RECALL_LAST          131   // 0x83 - 回调最后
#define HID_CONSUMER_QUIT                 148   // 0x94 - 退出
#define HID_CONSUMER_HELP                 149   // 0x95 - 帮助
#define HID_CONSUMER_CHANNEL_UP           156   // 0x9C - 频道增加
#define HID_CONSUMER_CHANNEL_DOWN         157   // 0x9D - 频道减少

#define HID_CONSUMER_PLAY                 176   // 0xB0 - 播放
#define HID_CONSUMER_PAUSE                177   // 0xB1 - 暂停
#define HID_CONSUMER_RECORD               178   // 0xB2 - 录制
#define HID_CONSUMER_FAST_FORWARD         179   // 0xB3 - 快进
#define HID_CONSUMER_REWIND               180   // 0xB4 - 倒带
#define HID_CONSUMER_SCAN_NEXT_TRK        181   // 0xB5 - 扫描下一曲目
#define HID_CONSUMER_SCAN_PREV_TRK        182   // 0xB6 - 扫描上一曲目
#define HID_CONSUMER_STOP                 183   // 0xB7 - 停止
#define HID_CONSUMER_EJECT                184   // 0xB8 - 弹出
#define HID_CONSUMER_RANDOM_PLAY          185   // 0xB9 - 随机播放
#define HID_CONSUMER_SELECT_DISC          186   // 0xBA - 选择光盘
#define HID_CONSUMER_ENTER_DISC           187   // 0xBB - 输入光盘
#define HID_CONSUMER_REPEAT               188   // 0xBC - 重复
#define HID_CONSUMER_STOP_EJECT           204   // 0xCC - 停止/弹出
#define HID_CONSUMER_PLAY_PAUSE           205   // 0xCD - 播放/暂停
#define HID_CONSUMER_PLAY_SKIP            206   // 0xCE - 播放/跳过

#define HID_CONSUMER_VOLUME               224   // 0xE0 - 音量
#define HID_CONSUMER_BALANCE              225   // 0xE1 - 平衡
#define HID_CONSUMER_MUTE                 226   // 0xE2 - 静音
#define HID_CONSUMER_BASS                 227   // 0xE3 - 低音
#define HID_CONSUMER_VOLUME_UP            233   // 0xE9 - 音量增大
#define HID_CONSUMER_VOLUME_DOWN          234   // 0xEA - 音量减小

/*********************************************************************
 * 类型定义
 */

// HID报告映射表
typedef struct
{
    uint16_t handle;     // 报告特征的句柄
    uint16_t cccdHandle; // 报告特征CCCD的句柄
    uint8_t  id;         // 报告ID
    uint8_t  type;       // 报告类型
    uint8_t  mode;       // 协议模式(报告或启动)
} hidRptMap_t;

// HID设备配置结构
typedef struct
{
    uint32_t idleTimeout; // 空闲超时(毫秒)
    uint8_t  hidFlags;    // HID特征标志

} hidDevCfg_t;

/*********************************************************************
 * 全局变量
 */

// 这些变量在使用HID设备的服务.c文件中定义

// HID报告映射长度
extern uint16_t hidReportMapLen;

// HID协议模式
extern uint8_t hidProtocolMode;

/*********************************************************************
 * 配置文件回调
 */

// HID报告回调
typedef uint8_t (*hidDevReportCB_t)(uint8_t id, uint8_t type, uint16_t uuid,
                                    uint8_t oper, uint16_t *pLen, uint8_t *pData);

// HID事件回调
typedef void (*hidDevEvtCB_t)(uint8_t evt);

// HID密码回调
typedef void (*hidDevPasscodeCB_t)(uint8_t *deviceAddr, uint16_t connectionHandle,
                                   uint8_t uiInputs, uint8_t uiOutputs);

typedef struct
{
    hidDevReportCB_t      reportCB;       // 报告回调函数
    hidDevEvtCB_t         evtCB;          // 事件回调函数
    hidDevPasscodeCB_t    passcodeCB;     // 密码回调函数
    gapRolesStateNotify_t pfnStateChange; // 设备状态改变时的回调函数
} hidDevCB_t;

/*********************************************************************
 * @fn      HidDev_Init
 *
 * @brief   HID设备任务的初始化函数。
 *          这在初始化期间调用，应包含任何特定于应用程序的初始化
 *          (如硬件初始化/设置、表初始化、上电通知等)。
 *
 * @param   task_id - TMOS分配的ID。此ID应用于发送消息和设置定时器。
 *
 * @return  无
 */
extern void HidDev_Init(void);

/*********************************************************************
 * @fn      HidDev_ProcessEvent
 *
 * @brief   HID设备任务事件处理器。此函数用于处理任务的所有事件。
 *          事件包括定时器、消息和任何其他用户定义的事件。
 *
 * @param   task_id  - TMOS分配的任务ID。
 * @param   events - 要处理的事件。这是一个位图，可以包含多个事件。
 *
 * @return  未处理的事件
 */
extern uint16_t HidDev_ProcessEvent(uint8_t task_id, uint16_t events);

/*********************************************************************
 * @fn      HidDev_Register
 *
 * @brief   向HID设备注册回调函数。
 *
 * @param   pCfg - 参数配置。
 * @param   pCBs - 回调函数。
 *
 * @return  无。
 */
extern void HidDev_Register(hidDevCfg_t *pCfg, hidDevCB_t *pCBs);

/*********************************************************************
 * @fn      HidDev_RegisterReports
 *
 * @brief   向HID设备注册报告表。
 *
 * @param   numReports - 报告表的长度。
 * @param   pRpt - 报告表。
 *
 * @return  无。
 */
extern void HidDev_RegisterReports(uint8_t numReports, hidRptMap_t *pRpt);

/*********************************************************************
 * @fn      HidDev_Report
 *
 * @brief   发送HID报告。
 *
 * @param   id - HID报告ID。
 * @param   type - HID报告类型。
 * @param   len - 报告长度。
 * @param   pData - 报告数据。
 *
 * @return  无。
 */
extern uint8_t HidDev_Report(uint8_t id, uint8_t type, uint8_t len, uint8_t *pData);

/*********************************************************************
 * @fn      HidDev_Close
 *
 * @brief   关闭连接或停止广播。
 *
 * @return  无。
 */
extern void HidDev_Close(void);

/*********************************************************************
 * @fn      HidDev_SetParameter
 *
 * @brief   设置HID设备参数。
 *
 * @param   param - 配置文件参数ID
 * @param   len - 要写入的数据长度
 * @param   pValue - 指向要写入数据的指针。这取决于参数ID，
 *          并将被转换为适当的数据类型(例如：uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t HidDev_SetParameter(uint8_t param, uint8_t len, void *pValue);

/*********************************************************************
 * @fn      HidDev_GetParameter
 *
 * @brief   获取HID设备参数。
 *
 * @param   param - 配置文件参数ID
 * @param   pValue - 指向获取数据的指针。这取决于参数ID，
 *          并将被转换为适当的数据类型(例如：uint16_t类型的数据
 *          将被转换为uint16_t指针)。
 *
 * @return  bStatus_t
 */
extern bStatus_t HidDev_GetParameter(uint8_t param, void *pValue);

/*********************************************************************
 * @fn      HidDev_PasscodeRsp
 *
 * @brief   响应密码请求。
 *
 * @param   status - 如果密码可用则为SUCCESS，否则
 *                   参见@ref SMP_PAIRING_FAILED_DEFINES。
 * @param   passcode - 包含密码的整数值。
 *
 * @return  无
 */
extern void HidDev_PasscodeRsp(uint8_t status, uint32_t passcode);

/*********************************************************************
 * @fn          HidDev_ReadAttrCB
 *
 * @brief       HID设备属性读取回调。
 *
 * @param       connHandle - 接收消息的连接句柄
 * @param       pAttr - 指向属性的指针
 * @param       pValue - 指向要读取的数据的指针
 * @param       pLen - 要读取的数据长度
 * @param       offset - 要读取的第一个字节的偏移量
 * @param       maxLen - 要读取的最大数据长度
 *
 * @return      成功或失败
 */
extern bStatus_t HidDev_ReadAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                   uint8_t *pValue, uint16_t *pLen, uint16_t offset, uint16_t maxLen, uint8_t method);

/*********************************************************************
 * @fn      HidDev_WriteAttrCB
 *
 * @brief   HID设备属性写入回调。
 *
 * @param   connHandle - 接收消息的连接句柄
 * @param   pAttr - 指向属性的指针
 * @param   pValue - 指向要写入的数据的指针
 * @param   len - 数据长度
 * @param   offset - 要写入的第一个字节的偏移量
 *
 * @return  成功或失败
 */
extern bStatus_t HidDev_WriteAttrCB(uint16_t connHandle, gattAttribute_t *pAttr,
                                    uint8_t *pValue, uint16_t len, uint16_t offset, uint8_t method);

/*********************************************************************
*********************************************************************/

extern void port_ble_init(void);


#ifdef __cplusplus
}
#endif

#endif /* HIDDEV_H */
