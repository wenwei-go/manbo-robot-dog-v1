#include "Key.h"

unsigned char Key_Read(void){
    unsigned char m = 0;
    
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0)    m = 1;
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == 0)    m = 2;
    if(HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == 0)    m = 3;
    if(HAL_GPIO_ReadPin(KEY4_GPIO_Port, KEY4_Pin) == 0)    m = 4;
    
    return m;
}
