#include "enabled_button.h"

#define EVENT_CB(ev)    \
	if (handle->cb[ev]) \
	handle->cb[ev]((Button *)handle,(PressEvent) (handle->event))

//button handle list head.
static struct Button *head_handle = NULL;

/**
 * @brief  Start the button work, add the handle into work list.
 * @param  handle: target handle strcut.
 * @retval 0: succeed. -1: already exist.
 */
static int8_t button_start(struct Button *handle) {
	struct Button *target = head_handle;
	while (target) {
		if (target == handle)
			return -1; //already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}

/**
 * @brief  Initializes the button struct handle.
 * @param  handle: the button handle strcut.
 * @param  attr: the button attr struct.
 * @retval 0: succeed. -1: already exist.
 */
int8_t button_init(struct Button *handle, const Btn_init_attr *attr) {
	memset(handle, 0, sizeof(struct Button));
	handle->event = (uint8_t) NONE_PRESS;
	handle->hal_button_Level = attr->pin_level;
	handle->button_level = handle->hal_button_Level();
	handle->active_level = attr->active_level;

	// Attach the button event and callback function
	for (uint8_t i = 0; i < attr->event_size; i++) {
		handle->cb[attr->event[i]] = attr->callback;
	}

	// start btn
	return button_start(handle);
}

/**
 * @brief  Button driver core function, driver state machine.
 * @param  handle: the button handle strcut.
 * @retval None
 */
static void button_handler(struct Button *handle) {
	uint8_t read_gpio_level = handle->hal_button_Level();

	//ticks counter working..
	if ((handle->state) > 0)
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if (read_gpio_level != handle->button_level) { //not equal to prev one
												   //continue read 3 times same new level change
		if (++(handle->debounce_cnt) >= DEBOUNCE_TICKS) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} else { //leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if (handle->button_level == handle->active_level) { //start press down
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
		if (handle->button_level != handle->active_level) { //released press up
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
		if (handle->button_level == handle->active_level) { //press down again
			handle->event = (uint8_t) PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			handle->event = (uint8_t) PRESS_REPEAT;
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		} else if (handle->ticks > SHORT_TICKS) { //released timeout
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
		if (handle->button_level != handle->active_level) { //released press up
			handle->event = (uint8_t) PRESS_UP;
			EVENT_CB(PRESS_UP);
			if (handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = 2; //repeat press
			} else {
				handle->state = 0;
			}
		} else if (handle->ticks > SHORT_TICKS) {
			handle->state = 0;
		}
		break;

	case 5:
		if (handle->button_level == handle->active_level) {
			//continue hold trigger
			handle->event = (uint8_t) LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);
		} else { //releasd
			handle->event = (uint8_t) PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; //reset
		}
		break;
	}
}

/**
 * @brief  Stop the button work, remove the handle off work list.
 * @param  handle: target handle strcut.
 * @retval None
 */
void button_deInit(struct Button *handle) {
	struct Button **curr;
	for (curr = &head_handle; *curr;) {
		struct Button *entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
			free(entry);
		} else
			curr = &entry->next;
	}
}

/**
 * @brief  background ticks, timer repeat invoking interval 5ms.
 * @param  None.
 * @retval None
 */
void button_ticks() {
	struct Button *target;
	for (target = head_handle; target; target = target->next) {
		button_handler(target);
	}
}
