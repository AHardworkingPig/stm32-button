#include "stm32_button.h"

#define EVENT_CB(ev)    \
	if (handle->cb[ev]) \
	handle->cb[ev]((Button *)handle,(PressEvent) (handle->event))

// button handle list head.
static Button *head_handle = NULL;

/**
 * @brief  Initializes the button struct handle.
 * @param  handle: the button handle struct.
 * @param  attr: button attr instance.
 * @retval 0: succeed. -1: already exist. -2: event error.
 */
int8_t button_init(Button *handle, const Btn_init_attr *attr) {
	Button *target = head_handle;
	while (target) {
		if (target == handle)
			return -1; // already exist.
		target = target->next;
	}

	memset(handle, 0, sizeof(Button));
	handle->event = (uint8_t) NONE_PRESS;
	handle->GPIOx = attr->GPIOx;
	handle->GPIO_PIN_x = attr->GPIO_PIN_x;
	handle->active_level = (uint8_t) attr->active_level;
	handle->button_level = !handle->active_level;

	// Attach the button event callback function
	for (uint8_t i = 0; i < EVENT_NUM ; i++) {
		if (attr->event[i] == 0) {
			if (i == 0) {
				goto EVENT_ERROR;
			} else {
				const uint8_t next = i++;
				if (next == EVENT_NUM) {
					break; // event attach success
				}
				for (uint8_t m = next; m < EVENT_NUM ; m++) {
					if (attr->event[m]) {
						goto EVENT_ERROR;
					}
				}
				break; // event attach success
			}
		} else if (attr->event[i] < 0 || attr->event[i] > EVENT_NUM) {
			EVENT_ERROR: memset(handle, 0, sizeof(Button));
			return -2; // event error!
		} else {
			handle->cb[attr->event[i]] =
					(void (*)(void*, PressEvent)) button_callback;
		}
	}

	// start button
	handle->next = head_handle;
	head_handle = handle;
	return 0; // success
}

/**
 * @brief  Button driver core function, driver state machine.
 * @param  handle: the button handle struct.
 * @retval none.
 */
static void button_handler(Button *handle) {
	uint8_t read_gpio_level = (uint8_t) HAL_GPIO_ReadPin(handle->GPIOx,
			handle->GPIO_PIN_x);

	// ticks counter working..
	if ((handle->state) > 0)
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if (read_gpio_level != handle->button_level) { // not equal to prev one
												   // continue read 3 times same new level change
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} else { // leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if (handle->button_level == handle->active_level) { // start press down
			handle->event = (uint8_t) PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		} else {
			handle->event = (uint8_t) NONE_PRESS;
		}
		break;

	case 1:
		if (handle->button_level != handle->active_level) { // released press up
			handle->event = (uint8_t) PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;
		} else if (handle->ticks > LONG_TICKS) {
			handle->event = (uint8_t) LONG_PRESS_START;
			EVENT_CB(LONG_PRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if (handle->button_level == handle->active_level) { // press down again
			handle->event = (uint8_t) PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			handle->event = (uint8_t) PRESS_REPEAT;
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		} else if (handle->ticks > SHORT_TICKS) { // released timeout
			if (handle->repeat == 1) {
				handle->event = (uint8_t) SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			} else if (handle->repeat == 2) {
				handle->event = (uint8_t) DOUBLE_CLICK;
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			}
			handle->state = 0;
		}
		break;

	case 3:
		if (handle->button_level != handle->active_level) { // released press up
			handle->event = (uint8_t) PRESS_UP;
			EVENT_CB(PRESS_UP);
			if (handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = 2; // repeat press
			} else {
				handle->state = 0;
			}
		} else if (handle->ticks > SHORT_TICKS) {
			handle->state = 0;
		}
		break;

	case 5:
		if (handle->button_level == handle->active_level) {
			// continue hold trigger
			handle->event = (uint8_t) LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		} else { // released
			handle->event = (uint8_t) PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; // reset
		}
		break;
	}
}

/**
 * @brief  Stop the button work, remove the handle off work list.
 * @param  handle: target handle struct.
 * @retval none.
 */
void button_deInit(Button *handle) {
	Button **curr;
	for (curr = &head_handle; *curr;) {
		Button *entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
			free(entry);
		} else
			curr = &entry->next;
	}
}

/**
 * @brief  background ticks, timer repeat invoking interval 5ms.
 * @param  none.
 * @retval none.
 */
void button_ticks() {
	Button *target;
	for (target = head_handle; target; target = target->next) {
		button_handler(target);
	}
}

/**
 * @brief Callback Function.
 * @param btn: button instance.
 * @param event: button event.
 * @retval none.
 */
void __attribute__((weak)) button_callback(Button *btn, PressEvent event) {
	// nothing to do,please rewrite.
}
