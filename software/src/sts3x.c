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

STS3X sts3x;

void sts3x_init(void) {
	memset(&sts3x, 0, sizeof(STS3X));

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

	sts3x.state = STS3X_STATE_IDLE; 
}

void sts3x_tick(void) {
	I2CFifoState state = i2c_fifo_next_state(&sts3x.i2c_fifo);

	if(state & I2C_FIFO_STATE_ERROR) {
		if((state == I2C_FIFO_STATE_READ_DIRECT_ERROR) && (sts3x.i2c_fifo.i2c_status & XMC_I2C_CH_STATUS_FLAG_NACK_RECEIVED)) {
			i2c_fifo_init(&sts3x.i2c_fifo);
		} else {
			loge("STS3X I2C error: %d\n\r", state);
			sts3x_init();
			return;
		}
	}

    switch (state) {
		case I2C_FIFO_STATE_READ_DIRECT_READY: {
			uint8_t data[16];
			uint8_t length = i2c_fifo_read_fifo(&sts3x.i2c_fifo, data, 16);

			switch(sts3x.state) {
				case STS3X_STATE_READ_DATA: {
					if(length != 3) {
						loge("STS3X unexpected I2C read length: %d\n\r", length);
						sts3x_init();
						return;
					}

					uint8_t crc = crc8(data, 2);
					if(crc == data[2]) {
						sts3x.temperature = -4500 + 17500*((data[0] << 8) | data[1])/(0xFFFF);
					}
					sts3x.state = STS3X_STATE_IDLE; 
					break;
				}

				default: sts3x.state = STS3X_STATE_IDLE; break;
			}
            break;
        }

        case I2C_FIFO_STATE_WRITE_DIRECT_READY: {
			switch(sts3x.state) {
				case STS3X_STATE_NEW_PERIODIC_MODE: sts3x.state = STS3X_STATE_IDLE; sts3x.new_periodic_mode = false; break;
				case STS3X_STATE_NEW_HEATER:        sts3x.state = STS3X_STATE_IDLE; sts3x.new_heater = false; break;
				default:                            sts3x.state = STS3X_STATE_IDLE; break;
			}
            break;
        }

        case I2C_FIFO_STATE_IDLE: {
			break; // Handled below
		}

		default: {
			// If we end up in a ready state that we don't handle, something went wrong
			if (state & I2C_FIFO_STATE_READY) {
				loge("STS3X unrecognized I2C ready state: %d\n\r", state);
				sts3x_init();
			}

			return;
		}
	}

    if((state == I2C_FIFO_STATE_IDLE || (state & I2C_FIFO_STATE_READY) != 0) || (state == I2C_FIFO_STATE_READ_DIRECT_ERROR)) {
        if(sts3x.new_periodic_mode) {
			// uint8_t data[2] = {0x27, 0x37}; / 10 samples per second
			// We use 4 SPS, since with 10 SPS there might be "self-heating" of the sensor (see datasheet Table 8)
			uint8_t data[2] = {0x23, 0x34}; // 4 samples per second
            i2c_fifo_write_direct(&sts3x.i2c_fifo, 2, data, true);

			sts3x.state = STS3X_STATE_NEW_PERIODIC_MODE;
        } else if(sts3x.new_heater) {
			uint8_t data[2] = {0x30, sts3x.heater_on ? 0x6D : 0x66};
            i2c_fifo_write_direct(&sts3x.i2c_fifo, 2, data, true);

			sts3x.state = STS3X_STATE_NEW_HEATER;
        } else {
			if(system_timer_is_time_elapsed_ms(sts3x.last_read_time, 250)) {
				uint8_t data[2] = {0xE0, 0x00};
				i2c_fifo_write_direct(&sts3x.i2c_fifo, 2, data, false);
				i2c_fifo_read_direct(&sts3x.i2c_fifo, 3, true);

				sts3x.state = STS3X_STATE_READ_DATA;
				sts3x.last_read_time = system_timer_get_ms();
			}
		}
    }
}

int16_t sts3x_get_temperature(void) {
	return sts3x.temperature;
}