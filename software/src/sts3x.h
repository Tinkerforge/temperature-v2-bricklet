/* temperature-v2-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * sts3x.h: Driver for STS3x temperature sensor
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

#ifndef STS3X_H
#define STS3X_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/hal/i2c_fifo/i2c_fifo.h"

typedef enum {
    STS3X_STATE_IDLE,
    STS3X_STATE_NEW_PERIODIC_MODE,
    STS3X_STATE_NEW_HEATER,
    STS3X_STATE_READ_DATA,
} STS3XState;

typedef struct {
    bool heater_on;
    bool new_heater;
    bool new_periodic_mode;

    uint32_t last_read_time;
    I2CFifo i2c_fifo;
    
    STS3XState state;

    int16_t temperature;
} STS3X;

extern STS3X sts3x;

void sts3x_init(void);
void sts3x_tick(void);

int16_t sts3x_get_temperature(void);

#endif