/********************************** (C) COPYRIGHT *******************************
 * File Name          : pct1336_driver.c
 * Author             : Hank
 * Version            : V1.0
 * Date               : 2025/10/15
 * Description        : 触控板驱动层，对应不同触控板芯片的驱动层，通过I2C等方式读取触控板数据并返回
 *******************************************************************************/


/*********************************************************************
 * INCLUDES 头文件
 */
#include "pct1336_driver.h"
#include "i2c_master.h"
#include "wait.h"
#include "event_manager.h"
#include "debug.h"
/*********************************************************************
 * MACROS 宏定义
 */
// 调试打印宏
#if 1
#define PCT1336_log(...) dprintf(__VA_ARGS__)
#else
#define PCT1336_log(...)
#endif

#define TOUCH_SLAVE_ID                              0x33 // 触摸板从机地址

/* OSAL 事件定义 */
#define PCT1336_REG_POLL_EVT                        (1 << 0)

#define EVENT_TOUCH 0x02
#define EVENT_GEST 0x08
#define EVENT_BTN 0x40

/*********************************************************************
 * CONSTANTS 常量
 */


/*********************************************************************
 * TYPEDEFS 数据类型定义
 */



/*********************************************************************
 * GLOBAL VARIABLES 全局变量
 */

/*********************************************************************
 * EXTERNAL VARIABLES   外部变量
 */
// extern uint8_t touch_en;

/*********************************************************************
 * EXTERNAL FUNCTIONS 外部函数
 */

/*********************************************************************
 * LOCAL VARIABLES 本地变量
 */
static pct1336_params_t* init_params = NULL;
static uint8_t init_params_len = 0;

static uint8_t retry_cnt = 0;
static uint8_t state = 0;

/* OSAL 任务 ID */
static uint8_t pct1336_taskID = 0;

/*********************************************************************
 * LOCAL FUNCTIONS 本地函数
 */

/// @brief 触摸板写入数据
/// @param addr 写入的寄存器地址
/// @param val 数据
/// @return 1：写入成功 0：写入失败
static uint8_t pct1336_write_reg(uint16_t addr, uint8_t val)
{
    i2c_status_t status = i2c_writeReg(TOUCH_SLAVE_ID, (uint8_t)addr, &val, 1, I2C_TIMEOUT_INFINITE);
    return (status == I2C_STATUS_SUCCESS) ? 1 : 0;
}
/// @brief 触摸板读取数据
/// @param *p_val 数据指针
/// @return 1：读取成功 0：读取失败
static uint8_t pct1336_read_reg(uint16_t addr, uint8_t * p_val)
{
    i2c_status_t status = i2c_readReg(TOUCH_SLAVE_ID, (uint8_t)addr, p_val, 1, I2C_TIMEOUT_INFINITE);
    return (status == I2C_STATUS_SUCCESS) ? 1 : 0;
}
/// @brief 触摸板数据连续读
/// @param addr 读取的寄存器地址
/// @param *p_val 数据指针
/// @param sz 读取数据长度
/// @return 1：读取成功 0：读取失败
static uint8_t pct1336_PTP_read_reg(uint16_t addr, uint8_t * p_val, uint8_t sz)
{
    i2c_status_t status = i2c_readReg(TOUCH_SLAVE_ID, (uint8_t)addr, p_val, sz, I2C_TIMEOUT_INFINITE);
    return (status == I2C_STATUS_SUCCESS) ? 1 : 0;
}
/// @brief 触摸板写入用户寄存器数据
/// @param bank 访问的bank编号
/// @param addr bank中操作的地址
/// @param val 数据
/// @return 1：写入成功 0：写入失败
int8_t pct1336_write_user_reg(uint8_t bank, uint16_t addr, uint8_t val)
{
	pct1336_write_reg(0x7f,0x06);
    wait_us(500);
	pct1336_write_reg(0x73,bank);
	wait_us(500);
	pct1336_write_reg(0x74,addr);
	wait_us(500);
	pct1336_write_reg(0x75,val);
	wait_us(500);
	return 1;
}
/// @brief 触摸板读取用户寄存器数据
/// @param bank 访问的bank编号
/// @param addr bank中操作的地址
/// @param * p_val 数据指针
/// @return 1：读取成功 0：读取失败
int8_t pct1336_read_user_reg(uint8_t bank, uint16_t addr, uint8_t * p_val)
{
	pct1336_write_reg(0x7f,0x06);
    wait_us(500);
	pct1336_write_reg(0x73,bank);
	wait_us(500);
	pct1336_write_reg(0x74,addr);
	wait_us(500);
	pct1336_read_reg(0x75, p_val);	
	wait_us(500);
	return 1;
}

/// @brief 通过INT_LEVEL_CTRL 寄存器检查中断电平状态，判断触控芯片是否正常启动
/// @details 读取INT_LEVEL_CTRL 寄存器，检查中断电平状态，如果BIT(0)为1 则启动成功，重复读取5次，每次间隔1ms
/// @return 1：初始化成功 0：初始化失败
// static int8_t pct1336_check_run_status(void)
// {
//     uint8_t state;
//     uint8_t retry_count = 0;
    
//     // 重复读取5次，每次间隔1ms
//     for (retry_count = 0; retry_count < 5; retry_count++) {
//         // 读取INT_LEVEL_CTRL寄存器(位于bank 0x00, 地址0x27)
//         if (pct1336_read_user_reg(0x00, 0x27, &state)) {
//             // 检查BIT(0)是否为1
//             if ((state & 0x01) == 0x01) {
//                 touch_en = 1;
//                 DEBUG_log("touch init success, INT_LEVEL_CTRL: 0x%02X\r\n", state);
//                 return 1; // 初始化成功
//             }
//         }
//         wait_ms(1); // 间隔1ms
//     }
//     DEBUG_log("touch init fail, INT_LEVEL_CTRL: 0x%02X\r\n", state);
//     touch_en = 0;
//     return 0; // 初始化失败
// }


/// @brief 触摸板初始化结束流程
/// @details 设置触摸板初始化完成标志，并检查中断电平状态
/// @return 1：初始化成功 0：初始化失败
int8_t pct1336_register_end(void){
	pct1336_read_user_reg(0x00,0x09,&state); 
	state |= 0x01;
	pct1336_write_user_reg(0x00,0x09,state); 

	pct1336_read_user_reg(0x00,0x09,&state); 
    PCT1336_log("<< touch_Init state:%x\r\n",state);
	pct1336_read_user_reg(0x00,0x27,&state); 
    PCT1336_log("INT_LEVEL:%x\r\n",state);
    return 1;
}

/// @brief 注册触控板参数并启动
/// @return 1：重启成功，0：未就绪
int8_t pct1336_register_params(pct1336_params_t *params, uint8_t len)
{
    // 读取状态寄存器，检查BIT0是否为1，最多重试5次
    uint8_t state;
    pct1336_read_reg(0x70, &state);
    if ((state & 0x01) != 0x01) {
        // 触摸芯片尚未准备好，继续等待
        retry_cnt++;
        if (retry_cnt >= 50) {
            PCT1336_log("Touch resume timeout after 50 attempts (500ms)\r\n");
            OSAL_StopTask(pct1336_taskID, PCT1336_REG_POLL_EVT);
            return 0;
        }
        return 0;
    }
    OSAL_StopTask(pct1336_taskID, PCT1336_REG_POLL_EVT);

    if (len > 0) {
        uint8_t u8tmp = 0;
        for(u8tmp = 0; u8tmp < len; u8tmp++){
            pct1336_write_user_reg(params[u8tmp].params.bank, params[u8tmp].params.address, params[u8tmp].params.data);
        }
    }
    return pct1336_register_end();
}

/// @brief 重启 PCT1336 触摸芯片，遵循数据手册中的 Watchdog Reset Flow
/// @return 1：成功 0：失败
int8_t pct1336_reset(void)
{
    uint8_t state;

    // Step 1: Disable sleep mode (Write 0x01 to 0x7c)
    if (!pct1336_write_reg(0x7c, 0x01)) {
        PCT1336_log("pct1336_reset: Failed to disable sleep mode\r\n");
        return 0;
    }

    // Step 2: Check if sleep mode is disabled (0x7c[4] is 0?)
    // 注意：0x7c 寄存器中 bit[4] 是 sleep enable 标志
    do {
        if (!pct1336_read_reg(0x7c, &state)) {
            PCT1336_log("pct1336_reset: Failed to read sleep status\r\n");
            return 0;
        }
        // 检查 bit[4] 是否为 0（表示 sleep 已禁用）
    } while ((state & 0x10) != 0);

    // Step 3: Enable reset (Write 0x00 to 0x7b)
    if (!pct1336_write_reg(0x7b, 0x00)) {
        PCT1336_log("pct1336_reset: Failed to enable reset\r\n");
        return 0;
    }

    // Step 4: Suspend (Write 0xaa to 0x7a)
    if (!pct1336_write_reg(0x7a, 0xaa)) {
        PCT1336_log("pct1336_reset: Failed to suspend\r\n");
        return 0;
    }

    // Step 5: Reboot (Write 0xbb to 0x7a)
    if (!pct1336_write_reg(0x7a, 0xbb)) {
        PCT1336_log("pct1336_reset: Failed to reboot\r\n");
        return 0;
    }

    // 成功完成复位流程
    PCT1336_log("pct1336_reset: Success\r\n");
    return 1;
}


/*********************************************************************
 * OSAL 事件处理函数
 */

/// @brief PCT1336 OSAL 事件处理函数
/// @param task_id 任务ID
/// @param events 事件标志
/// @return 未处理的事件
static uint16_t pct1336_process_event(uint8_t task_id, uint16_t events)
{
    if (events & PCT1336_REG_POLL_EVT) {
        pct1336_register_params(init_params, init_params_len);
        return (events ^ PCT1336_REG_POLL_EVT);
    }
    return 0;
}

/*********************************************************************
 * PUBLIC FUNCTIONS 公开的函数
 */
/// @brief 设置触控板初始化参数
/// @param params 具体参数
/// @param len 参数长度，以3个字节为一个单位
void pct1336_set_init_params(pct1336_params_t* params, uint8_t len)
{
    init_params = params;
    init_params_len = len;
}

int8_t pct1336_init()
{ 
    // 参数有效性检查
    if (init_params_len != 0 && init_params == NULL )
    {
        PCT1336_log("pct1336_init: params is NULL but len is not 0\r\n");
        return 0;
    }
    if (init_params_len == 0){
        uint8_t result = 0;
        
        // 添加重试机制，间隔1ms，最多重试3次
        do {
            result = pct1336_write_reg(0xF1, 0x00);
            if (result == 1) {
                break; // 成功则跳出循环
            }
            
            retry_cnt++;
            wait_ms(1); // 间隔1ms
        } while (retry_cnt < 3);
        
        retry_cnt = 0;
        if (result != 1) {
            PCT1336_log("pct1336_init: pct1336_write_reg fail\r\n");
            return 0;
        }
        PCT1336_log(">> touch_Init success\r\n");
        return 1;
    } else {
        // while(1)
	    // {
        //     if(pct1336_write_reg(0x7f, 0x06) == 1)
        //         break;

        //     retry_cnt++;

        //     wait_us(50);	
        //     if(retry_cnt >= 3)
        //     {
        //         //DBGPRINTF((">> touch_Init fail\r\n"));
        //         return 0;	
        //     }
        // }

        // wait_us(200);	
        // pct1336_write_reg(0x7b, 0x01);
        // wait_us(200);	
        // pct1336_write_reg(0x7a, 0xaa);
        // wait_us(200);	
        // pct1336_write_reg(0x7a, 0xbb);//reset
        // wait_ms(70);
        while(1)
        {
            if(pct1336_write_reg(0x7f, 0x06) == 1)
                break;

            retry_cnt++;

            wait_us(50);	
            if(retry_cnt >= 3)
            {
                //DBGPRINTF((">> touch_Init fail\r\n"));
                return 0;	
            }
        }

        wait_us(200);
        pct1336_write_reg(0x7b, 0x01);
        wait_us(200);
        pct1336_write_reg(0x7a, 0xaa);
        wait_us(200);
        pct1336_write_reg(0x7a, 0xbb); // reset
        retry_cnt = 0;
        state = 0;

        // 注册 OSAL 任务并启动轮询定时器
        if (pct1336_taskID == 0) {
            pct1336_taskID = OSAL_ProcessEventRegister(pct1336_process_event);
        }
        OSAL_StartReloadTask(pct1336_taskID, PCT1336_REG_POLL_EVT, 10);
        return 1;
    }
}

int8_t pct1336_resume(void){
	return pct1336_write_reg(0x7A,0xDD);
}

int8_t pct1336_sleep(void){
	uint8_t u8tmp;
	pct1336_write_reg(0x7f,0x06);

	pct1336_read_reg(0x7c,&u8tmp);

	if((u8tmp & 0x01) == 0x00)
	{
		pct1336_write_reg(0x7c,u8tmp | 0x01);

		while(1)
		{
			pct1336_read_reg(0x7c,&u8tmp);

			if((u8tmp & 0x10) == 0x00)
				break;
		}
	}

	pct1336_write_reg(0x7B,0x01);

	pct1336_write_reg(0x7A,0xAA);    //suspend

	return 1;
}

int8_t pct1336_read_status(uint8_t *st, uint8_t *button_st , uint8_t *gesture_st){
	// 参数检查 - 要求至少st不为空
    if (st == NULL) {
        return 0;
    }
    
    // 读取状态寄存器
    if (!pct1336_read_reg(0x71, st)) {
        return 0;
    }
    
    // 如果提供了button_st指针，则读取按钮状态
    if (button_st != NULL) {
        *button_st = 0; // 默认值
        
        if ((*st & EVENT_BTN) != 0) {
            uint8_t u8tmp = 0;
            if (!pct1336_read_reg(0x68, &u8tmp)) {
                return 0;
            }
            // 当检测到按钮触发标识时才读取按钮状态
            if (u8tmp == 62) { // 62表示按钮被触发
                if (!pct1336_read_reg(0x6A, button_st)) {
                    return 0;
                }
            }
        }
    }
    
    // 如果提供了gesture_st指针，则读取手势状态
    if (gesture_st != NULL) {
        *gesture_st = 0; // 默认值
        
        if ((*st & EVENT_GEST) != 0) {
            if (!pct1336_read_reg(0x60, gesture_st)) {
                return 0;
            }
        }
    }
    
    return 1;
}

int8_t pct1336_clear_int(void){
	return pct1336_write_reg(0x71,0x00);
}

int8_t pct1336_read_mouse(uint16_t *x, uint16_t *y){
	// 参数检查
    if (x == NULL || y == NULL) {
        return 0;
    }
    
    // 定义寄存器地址
    #define DELTA_X_LOW_ADDR  0x62
    #define AUTO_INCREMENT    0x80
    
    // 使用连续读取一次性读取Delta X和Delta Y数据
    // 寄存器顺序: Delta X Low, Delta X High, Delta Y Low, Delta Y High
    uint8_t buffer[4];
    if (!pct1336_PTP_read_reg(AUTO_INCREMENT | DELTA_X_LOW_ADDR, &buffer[0], 4)) {
        return 0; // 读取失败
    }

    // 组合高低字节形成完整的坐标值
    *x = (uint16_t)(buffer[0] | (buffer[1] << 8));  // Delta X
    *y = (uint16_t)(buffer[2] | (buffer[3] << 8));  // Delta Y
    
    return 1;
}

int8_t pct1336_read_ptp_report(touchpad_data_t *ptp){
   // 参数检查
    if (ptp == NULL) {
        return 0;
    }
    
    uint8_t TP_Count = 0;
    
    // 读取触摸点数量
    if (!pct1336_read_reg(0x01, (uint8_t*)&TP_Count)) {
        return 0; // 读取失败
    }
    
    // 检查是否有触摸点
    if(TP_Count == 0){
        // 无触摸点，清空数据并返回成功
        memset(ptp, 0x00, sizeof(touchpad_data_t));
        return 1;
    }
    
    // 检查触摸点数量是否超过最大值
    if(TP_Count > TP_MAX_CONTACT_COUNT) {
        TP_Count = TP_MAX_CONTACT_COUNT;
    }
    
    // 清空结构体
    memset(ptp, 0x00, sizeof(touchpad_data_t));
    ptp->contact_count = TP_Count;

    // 读取所有触摸点数据
    // 每个触摸点占用6个字节，从地址0x02开始读取
    uint8_t read_len = 6 * TP_Count;
    uint8_t ptp_data[6 * TP_MAX_CONTACT_COUNT] = {0};
    
    if (!pct1336_PTP_read_reg(0x80 | 0x02, &ptp_data[0], read_len)) {
        return 0; // 读取失败
    }

    // 解析每个触摸点的数据
    for (uint8_t i = 0; i < TP_Count; i++) {   
		// 单独获取触摸大小
        uint8_t size = 0;
        // 提取触摸点状态信息
        ptp->contacts[i].tip = ((ptp_data[i*6]) & 0x01);              // 触摸标志位
        ptp->contacts[i].confidence = ((ptp_data[i*6]) & 0x02) >> 1;  // 置信度标志位
        ptp->contacts[i].contact_id = (ptp_data[i*6+1] & 0x07);       // 触摸点ID
        
        // 提取坐标信息 (小端序)
        ptp->contacts[i].x = ((ptp_data[i*6+3]) << 8) + (ptp_data[i*6+2]);  // X坐标
        ptp->contacts[i].y = ((ptp_data[i*6+5]) << 8) + (ptp_data[i*6+4]);  // Y坐标
        

        if (!pct1336_read_reg(0x20 + i * 12, (uint8_t*)&size)) {
            return 0; // 读取失败
        }
        ptp->contacts[i].size = size;
    }
    
    return 1;
}


void pct1336_watchdog_check(void)
{
    uint8_t status = 0;
    
    // 读取状态寄存器 0x71
    if (pct1336_read_reg(0x71, &status)) {
        // 检查错误状态位 (bit 0) 或看门狗复位位 (bit 7)
        if ((status & TOUCH_STATUS_ERROR) == TOUCH_STATUS_ERROR || 
            (status & TOUCH_STATUS_WATCHDOG_RESET) == TOUCH_STATUS_WATCHDOG_RESET) {
            int8_t ret = pct1336_init();
            // 记录错误日志
            PCT1336_log("Touchpad error detected, status: 0x%02X. Restarting...\r\n", status);
            
            // 重启触控板
            if (ret == 1) {
                PCT1336_log("Touchpad reset successful\r\n");
            } else {
                PCT1336_log("Touchpad reset failed\r\n");
            }
        } 
        // else {
        //     PCT1336_log("Touchpad status: 0x%02X\r\n", status);
        // }
    } else {
        PCT1336_log("Failed to read touchpad status\r\n");
    }
}


/*********************************************************************
*********************************************************************/

