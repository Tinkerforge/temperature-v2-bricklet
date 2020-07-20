/* temperature-v2-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * sts3x.c: Driver for STS3x temperature sensor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "sts3x.h"

#include "configs/config_sts3x.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/logging/logging.h"
#include "bricklib2/utility/crc8.h"
#include "bricklib2/os/coop_task.h"

STS3X sts3x;
CoopTask sts3x_task;

void sts3x_init_i2c(void) {

	sts3x.i2c_fifo.baudrate         = STS3X_I2C_BAUDRATE;
	sts3x.i2c_fifo.address          = STS3X_I2C_ADDRESS;
	sts3x.i2c_fifo.i2c              = STS3X_I2C;

	sts3x.i2c_fifo.scl_port         = STS3X_SCL_PORT;
	sts3x.i2c_fifo.scl_pin          = STS3X_SCL_PIN;
	sts3x.i2c_fifo.scl_mode         = STS3X_SCL_PIN_MODE;
	sts3x.i2c_fifo.scl_input        = STS3X_SCL_INPUT;
	sts3x.i2c_fifo.scl_source       = STS3X_SCL_SOURCE;
	sts3x.i2c_fifo.scl_fifo_size    = STS3X_SCL_FIFO_SIZE;
	sts3x.i2c_fifo.scl_fifo_pointer = STS3X_SCL_FIFO_POINTER;

	sts3x.i2c_fifo.sda_port         = STS3X_SDA_PORT;
	sts3x.i2c_fifo.sda_pin          = STS3X_SDA_PIN;
	sts3x.i2c_fifo.sda_mode         = STS3X_SDA_PIN_MODE;
	sts3x.i2c_fifo.sda_input        = STS3X_SDA_INPUT;
	sts3x.i2c_fifo.sda_source       = STS3X_SDA_SOURCE;
	sts3x.i2c_fifo.sda_fifo_size    = STS3X_SDA_FIFO_SIZE;
	sts3x.i2c_fifo.sda_fifo_pointer = STS3X_SDA_FIFO_POINTER;

	i2c_fifo_init(&sts3x.i2c_fifo);

	const XMC_GPIO_CONFIG_t input_pin_config = {
		.mode             = XMC_GPIO_MODE_INPUT_INVERTED_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD,
	};

	const XMC_GPIO_CONFIG_t low_pin_config = {
		.mode             = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
        .output_level     = XMC_GPIO_OUTPUT_LEVEL_HIGH,
	};

	XMC_GPIO_Init(STS3X_ALERT_PIN, &input_pin_config);
	XMC_GPIO_Init(STS3X_NREST_PIN, &low_pin_config);
    system_timer_sleep_ms(2);
    XMC_GPIO_SetOutputHigh(STS3X_NREST_PIN);
    system_timer_sleep_ms(2);

    sts3x.new_periodic_mode = true;
}

void sts3x_task_tick(void) {
	uint32_t error = 0;
	while(true) {
		if(sts3x.new_periodic_mode) {
			// We use 4 SPS, since with 10 SPS there might be "self-heating" of the sensor (see datasheet Table 8)
			uint8_t data[2] = {0x23, 0x34}; // 4 samples per second
			error = i2c_fifo_coop_write_direct(&sts3x.i2c_fifo, 2, data, true);

			if(error == 0) {
				sts3x.new_periodic_mode = false;
			}
		} else if(sts3x.new_heater) {
			uint8_t data[2] = {0x30, sts3x.heater_on ? 0x6D : 0x66};
			error = i2c_fifo_coop_write_direct(&sts3x.i2c_fifo, 2, data, true);

			if(error == 0) {
				sts3x.new_heater = false;
			}
		} else {
			uint8_t data[3] = {0, 0, 0};
			error = i2c_fifo_coop_read_register(&sts3x.i2c_fifo, 0xE000, 3, data);
			if(error & XMC_I2C_CH_STATUS_FLAG_NACK_RECEIVED) {
				// Don't count a NACK as a standard error
				error = 0;

				// A NACK is to be expected according to datasheet, if the STS3X is currently not ready.
				// We wait for 50ms so the STS3X can finish the measurement.
				// If the STS3X has some kind of error condition where it always NACKs, we will reset it after a while
				sts3x.crc_error_count++;
				coop_task_sleep_ms(50);

				// We re-initialize the i2c_fifo here to make sure it isn't in any kind of error state because of the NACK.
				i2c_fifo_init(&sts3x.i2c_fifo);
			} else if(error == 0) {
				uint8_t crc = crc8(data, 2);
				if(crc == data[2]) {
					sts3x.temperature     = -4500 + 17500*((data[0] << 8) | data[1])/(0xFFFF);
					sts3x.crc_error_count = 0;

					coop_task_sleep_ms(250);
					continue;
				} else {
					sts3x.crc_error_count++;
				}
			}
		}
			
		if((error != 0) || (sts3x.crc_error_count >= 100)) {
			sts3x_init_i2c();
			sts3x.crc_error_count = 0;
		}

		coop_task_yield();
	}
}


void sts3x_init(void) {
	memset(&sts3x, 0, sizeof(STS3X));
	sts3x_init_i2c();
	coop_task_init(&sts3x_task, sts3x_task_tick);
}

void sts3x_tick(void) {
	coop_task_tick(&sts3x_task);
}

int16_t sts3x_get_temperature(void) {
	return sts3x.temperature;
}