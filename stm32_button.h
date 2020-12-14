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

// button handler
typedef void *Button_t;

typedef enum {
	PRESS_DOWN = 0u,
	PRESS_UP,
	PRESS_REPEAT,
	SINGLE_CLICK,
	DOUBLE_CLICK,
	LONG_PRESS_START,
	LONG_PRESS_HOLD
} PressEvent;

typedef enum {
	ONE = 1u, TWO, THREE, FOUR, FIVE, SIX, SEVEN
} EventNumber;

// System only
enum {
	EVENT_NUM = 7u, NONE_PRESS
};

typedef struct {
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_PIN_x;
	GPIO_PinState active_level :1;
	PressEvent event[EVENT_NUM];
	EventNumber event_num;
} Btn_init_attr;

#ifdef __cplusplus
extern "C"
{
#endif

Button_t button_init(const Btn_init_attr *attr);
void button_deInit(Button_t handle);
void button_ticks(void);
void button_callback(Button_t btn, PressEvent event, uint8_t repeat);

#ifdef __cplusplus
}
#endif

#endif
