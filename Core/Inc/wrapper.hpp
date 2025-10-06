#ifndef INC_WRAPPER_HPP_
#define INC_WRAPPER_HPP_

#ifdef __cplusplus
extern "C" {
#endif

void init(void);
void loop(void);

#ifdef __cplusplus
};
#endif

#endif /* INC_WRAPPER_HPP_ */

/*
 Example usage (place in your main/init code):

 #include "bmm350_hal.h"
 extern I2C_HandleTypeDef hi2c1; // your I2C handle
 struct bmm350_dev dev;

 // initialize HAL glue: pass 7-bit address of device (0x10..0x7F)
 bmm350_hal_init(&dev, &hi2c1, 0x10);
 // then initialize device
 if (bmm350_init(&dev) != 0) {
	 // handle error
 }

Notes:
 - HAL I2C expects 7-bit address. This glue shifts left internally for HAL API.
 - Delay uses DWT cycle counter when available for microsecond resolution; otherwise falls back to HAL_Delay.
*/
