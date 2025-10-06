/*
 * bmm350_hal.c
 * HAL glue for BMM350 sensor using STM32 HAL I2C
 */

#include "bmm350_hal.h"
#include "string.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Internal context stored in dev->intf_ptr */
struct bmm350_ctx {
    I2C_HandleTypeDef *hi2c;
    uint16_t dev_addr; /* 7-bit << 1 for HAL functions */
};

/* DWT microsecond delay support */
static void enable_dwt(void)
{
    /* Enable DWT counter if available */
#if defined(DWT) && defined(CoreDebug)
    if ((DWT->CTRL & 1) == 0)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
#endif
}

static void bmm350_delay_us_impl(uint32_t period_us, void *intf_ptr)
{
    /* Try DWT-based delay first */
#if defined(DWT) && defined(SystemCoreClock)
    uint32_t cycles_per_us = SystemCoreClock / 1000000UL;
    uint32_t start = DWT->CYCCNT;
    uint32_t delay_cycles = period_us * cycles_per_us;
    while ((DWT->CYCCNT - start) < delay_cycles)
    {
        __NOP();
    }
#else
    /* Fallback to HAL_Delay with ms resolution */
    if (period_us == 0)
        return;
    HAL_Delay((period_us + 999) / 1000);
#endif
}

/* bmm350 read function: reg_addr -> write, then read len bytes
 * Signature matches typedef: int8_t (*)(uint8_t, uint8_t*, uint32_t, void*)
 */
static BMM350_INTF_RET_TYPE bmm350_hal_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    struct bmm350_ctx *ctx = (struct bmm350_ctx *)intf_ptr;
    HAL_StatusTypeDef hst;

    if (ctx == NULL || ctx->hi2c == NULL)
        return (BMM350_INTF_RET_TYPE)BMM350_E_NULL_PTR;

    /* Use HAL_I2C_Mem_Read to perform register read with repeated start (common for sensors) */
    hst = HAL_I2C_Mem_Read(ctx->hi2c, ctx->dev_addr, (uint16_t)reg_addr, I2C_MEMADD_SIZE_8BIT, reg_data, (uint16_t)len, 1000);
    if (hst != HAL_OK)
    {
     printf("bmm350_hal_read: HAL_I2C_Mem_Read failed stat=%d addr=0x%02X reg=0x%02X len=%u - trying fallback\n",
         (int)hst, (int)(ctx->dev_addr >> 1), reg_addr, (unsigned)len);
     /* Additional HAL debug: I2C handle state and error code */
     printf(" bmm350_hal_read: hi2c=%p Instance=%p\n",
         (void *)ctx->hi2c, (void *)ctx->hi2c->Instance);

        /* Fallback: try write register then read (some buses / devices require split transfer) */
        HAL_StatusTypeDef hst2 = HAL_I2C_Master_Transmit(ctx->hi2c, ctx->dev_addr, &reg_addr, 1, 500);
        if (hst2 != HAL_OK)
        {
            printf("bmm350_hal_read fallback: Master_Transmit failed stat=%d\n", (int)hst2);
         printf(" bmm350_hal_read fallback: hi2c=%p Instance=%p\n",
             (void *)ctx->hi2c, (void *)ctx->hi2c->Instance);
            return (BMM350_INTF_RET_TYPE)BMM350_E_COM_FAIL;
        }

        hst2 = HAL_I2C_Master_Receive(ctx->hi2c, ctx->dev_addr, reg_data, (uint16_t)len, 1000);
        if (hst2 != HAL_OK)
        {
            printf("bmm350_hal_read fallback: Master_Receive failed stat=%d\n", (int)hst2);
         printf(" bmm350_hal_read fallback: hi2c=%p Instance=%p\n",
             (void *)ctx->hi2c, (void *)ctx->hi2c->Instance);
            return (BMM350_INTF_RET_TYPE)BMM350_E_COM_FAIL;
        }
    }

    return (BMM350_INTF_RET_TYPE)BMM350_INTF_RET_SUCCESS;
}

/* bmm350 write function: send reg_addr then payload */
static BMM350_INTF_RET_TYPE bmm350_hal_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    struct bmm350_ctx *ctx = (struct bmm350_ctx *)intf_ptr;
    HAL_StatusTypeDef hst;

    if (ctx == NULL || ctx->hi2c == NULL)
        return (BMM350_INTF_RET_TYPE)BMM350_E_NULL_PTR;

    /* Use HAL_I2C_Mem_Write to write register + payload */
    hst = HAL_I2C_Mem_Write(ctx->hi2c, ctx->dev_addr, (uint16_t)reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)reg_data, (uint16_t)len, 1000);
    if (hst != HAL_OK)
    {
        printf("bmm350_hal_write: HAL_I2C_Mem_Write failed stat=%d addr=0x%02X reg=0x%02X len=%u\n", (int)hst, (int)(ctx->dev_addr >> 1), reg_addr, (unsigned)len);
        return (BMM350_INTF_RET_TYPE)BMM350_E_COM_FAIL;
    }

    return (BMM350_INTF_RET_TYPE)BMM350_INTF_RET_SUCCESS;
}

/* Wrapper that matches delay pointer signature in bmm350_defs.h */
static void bmm350_hal_delay_us(uint32_t period, void *intf_ptr)
{
    bmm350_delay_us_impl(period, intf_ptr);
}

int8_t bmm350_hal_init(struct bmm350_dev *dev, I2C_HandleTypeDef *hi2c, uint16_t i2c_addr7)
{
    if (dev == NULL || hi2c == NULL)
        return BMM350_E_NULL_PTR;

    struct bmm350_ctx *ctx = (struct bmm350_ctx *)malloc(sizeof(struct bmm350_ctx));
    if (ctx == NULL)
        return BMM350_E_INVALID_INPUT;

    ctx->hi2c = hi2c;
    /* HAL expects 8-bit address (7-bit << 1) */
    ctx->dev_addr = (uint16_t)(i2c_addr7 << 1);

    /* Enable DWT if available for microsecond delays */
    enable_dwt();

    dev->intf_ptr = ctx;
    dev->read = bmm350_hal_read;
    dev->write = bmm350_hal_write;
    dev->delay_us = bmm350_hal_delay_us;

    return BMM350_OK;
}
