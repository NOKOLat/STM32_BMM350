#include "wrapper.hpp"
#include "main.h"
#include <stdio.h>
#include "bmm350_hal.h"
#include "bmm350.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;
static struct bmm350_dev dev;
static const uint8_t bmm350_addr7 = 0x14;

void init(){

	// アドレスや使用する関数の設定
	int8_t result = bmm350_hal_init(&dev, &hi2c1, bmm350_addr7);
	if (result != BMM350_OK){

		printf("bmm350_hal_init failed: %d\n", result);
	}

	// センサーとの接続チェック
	result = HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(bmm350_addr7 << 1), 3, 100) ;
	if (result != HAL_OK) {

		printf("I2C device not ready at address 0x%02X - check wiring/address\n", bmm350_addr7);
	}

	// センサーの初期化
	result = bmm350_init(&dev);
	if (result != BMM350_OK){

		printf("bmm350_init failed: %d\n", result);
	}

	// ODRと平均化の設定
	result = bmm350_set_odr_performance(BMM350_DATA_RATE_400HZ, BMM350_NO_AVERAGING, &dev);
	if (result != BMM350_OK){

		printf("set_odr_performance failed: %d\n", result);
	}

	// 各軸の測定の有効化
	result = bmm350_enable_axes(BMM350_X_EN, BMM350_Y_EN, BMM350_Z_EN, &dev);
	if (result != BMM350_OK) {

		printf("enable_axes failed: %d\n", result);
	}

	// 電源モードを設定
	result = bmm350_set_powermode(BMM350_NORMAL_MODE, &dev);
	if (result != BMM350_OK) {

		printf("set_powermode failed: %d\n", result);
	}

	// 初期化終了メッセージ
	printf("BMM350 initialized (chip id %u)\n", dev.chip_id);
}

void loop(){

	struct bmm350_mag_temp_data mag;

	// データの読み取り
	int8_t result = bmm350_get_compensated_mag_xyz_temp_data(&mag, &dev);
	if (result == BMM350_OK){

		// 成功処理
		printf("Mag X=%.3f uT Y=%.3f uT Z=%.3f uT Temp=%.2f C\n", mag.x, mag.y, mag.z, mag.temperature);
	}
	else {

		// 失敗処理
		printf("get_mag_data failed: %d \n", result);
	}
}
