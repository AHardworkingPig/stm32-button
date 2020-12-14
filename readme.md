# STM32 Button

## 简介
STM32 Button 是一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调异步处理方式可以简化你的程序结构，去除冗余的按键处理硬编码，让你的按键业务逻辑更清晰。


## 按键事件

| 事件             | 说明                                 |
| ---------------- | ------------------------------------ |
| PRESS_DOWN       | 按键按下，每次按下都触发             |
| PRESS_UP         | 按键弹起，每次松开都触发             |
| PRESS_REPEAT     | 重复按下触发，变量repeat计数连击次数 |
| SINGLE_CLICK     | 单击按键事件                         |
| DOUBLE_CLICK     | 双击按键事件                         |
| LONG_PRESS_START | 达到长按时间阈值时触发一次           |
| LONG_PRESS_HOLD  | 长按期间一直触发                     |


## Examples

```c
#include "stm32_button.h"

Button_t button1, button2;
const Btn_init_attr attr = { .GPIO_PIN_x = GPIO_PIN_2, .GPIOx = GPIOC, .event =
		{ PRESS_DOWN }, .event_num = ONE, .active_level = GPIO_PIN_RESET };
const Btn_init_attr attr2 = { .GPIO_PIN_x = GPIO_PIN_2, .GPIOx = GPIOC, .event =
		{ PRESS_REPEAT }, .event_num = ONE, .active_level = GPIO_PIN_RESET };

button1 = button_init(&attr);
button2 = button_init(&attr2);

while (1) {
    HAL_Delay(5); // 5ms
    button_ticks();
}

// 回调函数
void button_callback(Button_t btn, PressEvent event, uint8_t repeat) {
	if (btn == button1) {
		if (event == PRESS_DOWN) {
			printf("PRESS_DOWN\n");
			button_deInit(btn);
			printf("remove button1\n");
		}
	} else if (btn == button2) {
		if (event == PRESS_REPEAT) {
			printf("PRESS_REPEAT -> %d\n", repeat);
		}
	}
}
```