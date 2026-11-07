#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*字符集定义*/
/*以下两个宏定义只可解除其中一个的注释*/
//#define OLED_CHARSET_UTF8			//定义字符集为UTF8
#define OLED_CHARSET_GB2312		//定义字符集为GB2312

/*字模基本单元*/
typedef struct 
{
	
#ifdef OLED_CHARSET_UTF8			//定义字符集为UTF8
	char Index[5];					//汉字索引，空间为5字节
#endif
	
#ifdef OLED_CHARSET_GB2312			//定义字符集为GB2312
	char Index[3];					//汉字索引，空间为3字节
#endif
	
	uint8_t Data[32];				//字模数据
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

/*图像数据声明*/
extern const uint8_t Diode[];
extern const uint8_t Logo[];
extern const uint8_t Face_eyes[];
extern const uint8_t Face_happy[];
extern const uint8_t Face_mania[];
extern const uint8_t Face_sleep[];
extern const uint8_t Face_stare[];
extern const uint8_t Face_very_happy[];
extern const uint8_t Face_hello[];

extern const uint8_t huaji[];
extern const uint8_t BMP1[];//立正脸
extern const uint8_t BMP2[];//前进脸
extern const uint8_t BMP3[];//右转脸
extern const uint8_t BMP4[];//左转脸
extern const uint8_t BMP5[];//特殊脸（嘿嘿脸）
extern const uint8_t BMP6[];//睡觉脸（普通）
extern const uint8_t LOVE[];//love花痴脸
extern const uint8_t BMP8[];//酣睡脸（流口水）
extern const uint8_t BMP9[];//开心脸
extern const uint8_t BMP10[];//调皮脸
extern const uint8_t BMP11[];//迷糊脸（开心得迷糊）
extern const uint8_t BMP12[];//猫猫脸
extern const unsigned char AFRAID[];
extern const uint8_t PDX[];
extern const uint8_t Liu[];
#endif


/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
