#include "Move.h"
#include "Servo.h"

// 全局变量
MoveControl_t g_move_ctrl;  // 全局运动控制结构体
int movedelay = 150;        // 动作间隔时间(单位:ms)
uint8_t Face_Mode = 0;  // 定义并初始化变量

// ==================== 基础控制函数 ====================

// 初始化运动控制
void Move_Init(void) {
    g_move_ctrl.current_state = MOVE_IDLE;
    g_move_ctrl.step = 0;
    g_move_ctrl.last_time = 0;
    g_move_ctrl.delay_ms = 0;
    g_move_ctrl.repeat_count = 0;
    g_move_ctrl.is_busy = 0;
    g_move_ctrl.target_repeats = 0;
    g_move_ctrl.completed_repeats = 0;
}

// 启动动作: times=0连续模式, times=1单次, times>1指定次数
void Move_Start_Continuous(MoveState_t state, uint8_t times) {
    g_move_ctrl.current_state = state;
    g_move_ctrl.step = 0;
    g_move_ctrl.last_time = HAL_GetTick();
    g_move_ctrl.is_busy = (state != MOVE_IDLE);
    g_move_ctrl.repeat_count = 0;
    g_move_ctrl.target_repeats = times;
    g_move_ctrl.completed_repeats = 0;
}

// 停止当前动作
void Move_Stop(void) {
    g_move_ctrl.current_state = MOVE_IDLE;
    g_move_ctrl.step = 0;
    g_move_ctrl.is_busy = 0;
    g_move_ctrl.target_repeats = 0;
    g_move_ctrl.completed_repeats = 0;
}

// 检查是否正在执行动作
uint8_t Move_IsBusy(void) {
    return g_move_ctrl.is_busy;
}
// 检查是否到达延时时间
static uint8_t Check_Delay(void) {
    if (HAL_GetTick() - g_move_ctrl.last_time >= g_move_ctrl.delay_ms) {
        g_move_ctrl.last_time = HAL_GetTick();
        return 1;
    }
    return 0;
}

// 统一的循环检查函数
static void Check_Repeat_And_Continue(void) {
    if (g_move_ctrl.target_repeats == 0) {// 连续模式:一直重复
        g_move_ctrl.step = 0;
        g_move_ctrl.repeat_count = 0;
    } else {// 指定次数模式
        g_move_ctrl.completed_repeats++;
        if (g_move_ctrl.completed_repeats < g_move_ctrl.target_repeats) {
            g_move_ctrl.step = 0;
            g_move_ctrl.repeat_count = 0;
        } else {
            Move_Stop();  // 达到指定次数,停止
        }
    }
}

// ==================== 动作处理函数 ====================

// 站立动作
static void Process_Stand(void) {
    switch (g_move_ctrl.step) {
        case 0:	Servo_SetAngle1(90);	Servo_SetAngle2(90);	Servo_SetAngle3(90);	Servo_SetAngle4(90);	g_move_ctrl.delay_ms = 500;g_move_ctrl.step++;	break;
        case 1:	if (Check_Delay())  	Check_Repeat_And_Continue();	break;
    }
}

// 前进动作
static void Process_Forward(void) {
    if (!Check_Delay()) return;
    
    switch (g_move_ctrl.step) {
        case 0:	Servo_SetAngle2(45);        Servo_SetAngle3(45);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 1:	Servo_SetAngle1(135);       Servo_SetAngle4(135);       g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 2:	Servo_SetAngle2(90);        Servo_SetAngle3(90);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 3:	Servo_SetAngle1(90);        Servo_SetAngle4(90);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 4:	Servo_SetAngle1(45);        Servo_SetAngle4(45);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 5:	Servo_SetAngle2(135);       Servo_SetAngle3(135);       g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 6:	Servo_SetAngle1(90);        Servo_SetAngle4(90);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 7:	Servo_SetAngle2(90);        Servo_SetAngle3(90);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 8:	Servo_SetAngle5(90);	if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	Face_Mode=1;	Check_Repeat_And_Continue(); break;
    }
}

// 后退动作
static void Process_Behind(void) {
    if (!Check_Delay()) return;
    
    switch (g_move_ctrl.step) {
        case 0:	Servo_SetAngle2(135);        Servo_SetAngle3(135);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 1:	Servo_SetAngle1(45);         Servo_SetAngle4(45);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 2:	Servo_SetAngle2(90);         Servo_SetAngle3(90);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 3:	Servo_SetAngle1(90);         Servo_SetAngle4(90);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 4:	Servo_SetAngle1(135);        Servo_SetAngle4(135);        g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 5:	Servo_SetAngle2(45);         Servo_SetAngle3(45);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 6:	Servo_SetAngle1(90);         Servo_SetAngle4(90);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 7:	Servo_SetAngle2(90);         Servo_SetAngle3(90);         g_move_ctrl.delay_ms = movedelay;    g_move_ctrl.step++; break;
        case 8:	if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	Face_Mode=1;	Check_Repeat_And_Continue(); break;
    }
}

// 左转动作
static void Process_Left(void) {
    if (!Check_Delay()) return;
	
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle1(45); 	Servo_SetAngle4(135);	 g_move_ctrl.delay_ms = movedelay; g_move_ctrl.step++; break;
        case 1: Servo_SetAngle2(45); 	Servo_SetAngle3(135); 	 g_move_ctrl.delay_ms = movedelay; g_move_ctrl.step++; break;
        case 2: Servo_SetAngle1(90);	 Servo_SetAngle4(90);	 g_move_ctrl.delay_ms = movedelay; g_move_ctrl.step++; break;
        case 3: Servo_SetAngle2(90); 	Servo_SetAngle3(90);	 g_move_ctrl.delay_ms = movedelay; g_move_ctrl.step++; break;
        case 4: if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	Face_Mode=1;	Check_Repeat_And_Continue(); break;
    }
}

// 右转动作
static void Process_Right(void) {
    if (!Check_Delay()) return;
	
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle2(45); 	Servo_SetAngle3(135); 	g_move_ctrl.delay_ms = movedelay; 	g_move_ctrl.step++; break;
        case 1: Servo_SetAngle1(45); 	Servo_SetAngle4(135); 	g_move_ctrl.delay_ms = movedelay; 	g_move_ctrl.step++; break;
        case 2: Servo_SetAngle2(90); 	Servo_SetAngle3(90); 	g_move_ctrl.delay_ms = movedelay; 	g_move_ctrl.step++; break;
        case 3: Servo_SetAngle1(90); 	Servo_SetAngle4(90); 	g_move_ctrl.delay_ms = movedelay;	g_move_ctrl.step++; break;
        case 4: if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	Face_Mode=1;	Check_Repeat_And_Continue(); break;
    }
}

// 睡觉动作
static void Process_Sleep(void) {
    if (!Check_Delay()) return;
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle1(20); Servo_SetAngle2(20); Servo_SetAngle3(160); Servo_SetAngle4(160); g_move_ctrl.delay_ms = 500; g_move_ctrl.step++; break;
        case 1: Check_Repeat_And_Continue(); break;
    }
}

// 趴下动作(全朝前）
static void Process_PaXia(void){
	if (!Check_Delay()) return;
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle1(20); Servo_SetAngle2(20); Servo_SetAngle3(20); Servo_SetAngle4(20); g_move_ctrl.delay_ms = 500; g_move_ctrl.step++; break;
        case 1: Check_Repeat_And_Continue(); break;
    }
}

// 摇摆
static void Process_Shake_QianHou(void) {
    if (!Check_Delay()) return;

	static uint8_t Shake_angle = 90;
    switch (g_move_ctrl.step) {
        case 0:if (Shake_angle < 150) {Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;  		break;
        case 1:if (Shake_angle > 30) {Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;			break;
        case 2:if (Shake_angle < 150) {Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;  		break;
        case 3:if (Shake_angle > 30) {Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;			break;
        case 4:if (Shake_angle < 150) {Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;  		break;
        case 5:if (Shake_angle > 30) {Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(Shake_angle);Servo_SetAngle2(Shake_angle);Servo_SetAngle3(Shake_angle);Servo_SetAngle4(Shake_angle);g_move_ctrl.delay_ms = 10;
            } else g_move_ctrl.step++;			break;
        case 6:	if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	
			{Face_Mode=1;Servo_SetAngle1(90);	Servo_SetAngle2(90);	Servo_SetAngle3(90);	Servo_SetAngle4(90);}Check_Repeat_And_Continue();	break;
    }
}

// 蹲下
static void Process_DunXia(void) {
    if (!Check_Delay()) return;
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(20); Servo_SetAngle4(20); g_move_ctrl.delay_ms = 500; g_move_ctrl.step++; break;
        case 1: Check_Repeat_And_Continue(); break;
    }
}

// 快速摇摆
static void Process_FAST_Shake_QianHou(void) {
    if (!Check_Delay()) return;

	static uint8_t FAST_Shake_angle = 90;
    switch (g_move_ctrl.step) {
        case 0:if (FAST_Shake_angle < 150) {FAST_Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;  		break;
        case 1:if (FAST_Shake_angle > 30) {FAST_Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;			break;
        case 2:if (FAST_Shake_angle < 150) {FAST_Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;  		break;
        case 3:if (FAST_Shake_angle > 30) {FAST_Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;			break;
        case 4:if (FAST_Shake_angle < 150) {FAST_Shake_angle += 2; // 每次增加2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;  		break;
        case 5:if (FAST_Shake_angle > 30) {FAST_Shake_angle -= 2; // 每次减少2度
                Servo_SetAngle1(FAST_Shake_angle);Servo_SetAngle2(FAST_Shake_angle);Servo_SetAngle3(FAST_Shake_angle);Servo_SetAngle4(FAST_Shake_angle);g_move_ctrl.delay_ms = 5;
            } else g_move_ctrl.step++;			break;
        case 6:	Servo_SetAngle1(90);	Servo_SetAngle2(90);	Servo_SetAngle3(90);	Servo_SetAngle4(90);	g_move_ctrl.delay_ms = 500;g_move_ctrl.step++;	break;
        case 7:	if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))	Face_Mode=1;Check_Repeat_And_Continue();	break;
    }
}

// 摇尾巴
static void Process_Wag_Tail(void) {
    if (!Check_Delay()) return;
	
	static uint8_t Wag_Tail_angle = 90;
    switch (g_move_ctrl.step) {
        case 0: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle > 30) {Wag_Tail_angle--; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 1: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle < 150) {Wag_Tail_angle++; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 2: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle > 30) {Wag_Tail_angle--; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 3: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle < 150) {Wag_Tail_angle++; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 4: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle > 30) {Wag_Tail_angle--; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 5: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(90); Servo_SetAngle4(90); 
				if (Wag_Tail_angle < 150) {Wag_Tail_angle++; 	Servo_SetAngle5(Wag_Tail_angle); g_move_ctrl.delay_ms = 2;}else {g_move_ctrl.step++;} break;
        case 6: Servo_SetAngle1(90); Servo_SetAngle2(90); Servo_SetAngle3(30); Servo_SetAngle4(30); Servo_SetAngle5(90); 
				if (!(g_move_ctrl.completed_repeats + 1 < g_move_ctrl.target_repeats))Face_Mode=1;	Check_Repeat_And_Continue(); break;
    }
}

// ==================== 主处理函数 ====================
// 主处理函数 - 在主循环中持续调用,根据当前状态执行相应动作
void Move_Process(void) {
    if (!g_move_ctrl.is_busy) return;
    
    switch (g_move_ctrl.current_state) {
        case MOVE_SLEEP:       	  		Process_Sleep();     			break;
		case MOVE_DunXia:		  		Process_DunXia();	  			break;
        case MOVE_STAND:          		Process_Stand();        		break;
        case MOVE_PaXia:          		Process_PaXia();        		break;
        case MOVE_FORWARD:        		Process_Forward();      		break;
        case MOVE_BEHIND:         		Process_Behind();       		break;
        case MOVE_LEFT:           		Process_Left();         		break;
        case MOVE_RIGHT:          		Process_Right();        		break;
        case MOVE_SHAKE_QIANHOU:  		Process_Shake_QianHou(); 		break;
		case MOVE_FAST_SHAKE_QIANHOU:	Process_FAST_Shake_QianHou();	break;
        case MOVE_WAG_TAIL:       		Process_Wag_Tail();    			break;
        default:                  		Move_Stop();            		break;
    }
}
