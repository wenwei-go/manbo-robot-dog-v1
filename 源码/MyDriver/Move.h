#ifndef __MOVE_H
#define __MOVE_H

#include "main.h"

typedef enum {
    MOVE_IDLE = 0,
    MOVE_SLEEP,           	// 1. 睡觉
	MOVE_DunXia,			// 2. 蹲下
    MOVE_STAND,             // 3. 立正
    MOVE_PaXia,             // 4. 趴下
    MOVE_FORWARD,           // 5. 前进
	MOVE_BEHIND,			// 6. 后退
    MOVE_LEFT,              // 7. 左转
    MOVE_RIGHT,             // 8. 右转
    MOVE_SHAKE_QIANHOU,     // 9. 摇摆
    MOVE_FAST_SHAKE_QIANHOU,// 10. 摇摆加速
    MOVE_WAG_TAIL,          // 11. 摇尾巴
} MoveState_t;

// 运动控制结构体
typedef struct {
    MoveState_t current_state;  // 当前动作状态
    uint8_t step;               // 当前步骤
    uint32_t last_time;         // 上次执行时间
    uint32_t delay_ms;          // 延时时间（毫秒）
    uint8_t repeat_count;       // 重复计数（用于动作内部循环）
    uint8_t is_busy;            // 是否正在执行动作
    uint8_t target_repeats;     // 目标重复次数（0=连续，1=单次，>1=指定次数）
    uint8_t completed_repeats;  // 已完成重复次数
} MoveControl_t;

// 全局变量声明
extern MoveControl_t g_move_ctrl;
extern int movedelay;
extern uint8_t Face_Mode;

// 函数声明
void Move_Init(void);                                           // 初始化运动控制
void Move_Start_Continuous(MoveState_t state, uint8_t times);   // 启动动作（统一接口）
void Move_Stop(void);                                           // 停止当前动作
uint8_t Move_IsBusy(void);                                      // 检查是否正在执行动作
void Move_Process(void);                                        // 主处理函数（在主循环中调用）

#endif /* __MOVE_H */
