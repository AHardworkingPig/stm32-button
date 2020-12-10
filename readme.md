# EnabledButton

## 简介
EnabledButton 是一个小巧简单易用的事件驱动型按键驱动模块，可无限量扩展按键，按键事件的回调异步处理方式可以简化你的程序结构，去除冗余的按键处理硬编码，让你的按键业务逻辑更清晰。


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
#include "enabled_button.h"

uint8_t button_pin_level() {
	return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2);
}
void btn_test_cb(void *btn, PressEvent event) {
	if (event == SINGLE_CLICK) {
		printf("SINGLE_CLICK!\n");
	} else if (event == DOUBLE_CLICK) {
		printf("DOUBLE_CLICK!\n");
	} else if (event == PRESS_DOWN) {
		printf("PRESS_DOWN!\n");
	} else if (event == PRESS_REPEAT) {
		printf("PRESS_REPEAT! %d\n", ((Button*) btn)->repeat);
	} else if (event == LONG_PRESS_START) {
		printf("LONG_PRESS_START!\n");
	} else if (event == LONG_PRESS_HOLD) {
		printf("LONG_PRESS_HOLD!\n");
	} else if (event == PRESS_UP) {
		printf("PRESS_UP!\n");
	}

	//button_deInit((Button*) btn);
}
Button button1;
const Btn_init_attr attr = { .pin_level = button_pin_level, .active_level =
		GPIO_PIN_RESET, .event = { SINGLE_CLICK, DOUBLE_CLICK, PRESS_REPEAT,
		LONG_PRESS_START, PRESS_UP, LONG_PRESS_HOLD, PRESS_DOWN },
		.event_size = 7, .callback = btn_test_cb };
button_init(&button1, &attr);

while (1) {
    HAL_Delay(5); // 5ms
    button_ticks();
}
```
