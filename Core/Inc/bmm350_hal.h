/*
 * bmm350_hal.h
 * HAL glue for BMM350 sensor using STM32 HAL I2C
 */
#ifndef BMM350_HAL_H
#define BMM350_HAL_H

#include "bmm350_defs.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize bmm350_dev with HAL callbacks
 * Parameters:
 *  dev        : pointer to user bmm350_dev structure
 *  hi2c       : pointer to HAL I2C handle (e.g. &hi2c1)
 *  i2c_addr7  : 7-bit I2C device address (without R/W bit)
 */
int8_t bmm350_hal_init(struct bmm350_dev *dev, I2C_HandleTypeDef *hi2c, uint16_t i2c_addr7);

#ifdef __cplusplus
}
#endif

#endif /* BMM350_HAL_H */
