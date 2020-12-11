#ifndef _STM32_BUTTON_H_
#define _STM32_BUTTON_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Modify according to your STM32 MCU/MPU
#include "stm32f1xx_hal.h"

// According to your need to modify the constants.
#define TICKS_INTERVAL 10 // millisecond
#define DEBOUNCE_TICKS 5 // MAX 8
#define SHORT_TICKS (300 / TICKS_INTERVAL)
#define LONG_TICKS (1000 / TICKS_INTERVAL)

typedef enum {
	PRESS_DOWN = 1,
	PRESS_UP,
	PRESS_REPEAT,
	SINGLE_CLICK,
	DOUBLE_CLICK,
	LONG_PRESS_START,
	LONG_PRESS_HOLD,
	NONE_PRESS
} PressEvent;

typedef void (*BtnCallback)(void*, PressEvent);

#define EVENT_NUM (uint8_t)7

typedef struct Button {
	uint16_t ticks;
	uint8_t repeat :4; // max 15
	uint8_t event :4;
	uint8_t state :3;
	uint8_t debounce_cnt :3;
	uint8_t active_level :1;
	uint8_t button_level :1;
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_PIN_x;
	BtnCallback cb[EVENT_NUM ];
	struct Button *next;
} Button;

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_PIN_x;
	GPIO_PinState active_level :1;
	PressEvent event[EVENT_NUM ];
} Btn_init_attr;

#ifdef __cplusplus
extern "C"
{
#endif

int8_t button_init(Button *handle, const Btn_init_attr *attr);
void button_deInit(Button *handle);
void button_ticks(void);
void button_callback(Button *btn, PressEvent event);

#ifdef __cplusplus
}
#endif

#endif
