/* temperature-v2-bricklet
 * Copyright (C) 2018 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/utility/callback_value.h"

#include "sts3x.h"

CallbackValue_int16_t callback_value_temperature;

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_GET_TEMPERATURE: return get_callback_value_int16_t(message, response, &callback_value_temperature);
		case FID_SET_TEMPERATURE_CALLBACK_CONFIGURATION: return set_callback_value_callback_configuration_int16_t(message, &callback_value_temperature);
		case FID_GET_TEMPERATURE_CALLBACK_CONFIGURATION: return get_callback_value_callback_configuration_int16_t(message, response, &callback_value_temperature);
		case FID_SET_HEATER_CONFIGURATION: return set_heater_configuration(message);
		case FID_GET_HEATER_CONFIGURATION: return get_heater_configuration(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}

BootloaderHandleMessageResponse set_heater_configuration(const SetHeaterConfiguration *data) {
	bool new_heater_value = data->heater_config == TEMPERATURE_V2_HEATER_CONFIG_ENABLED;
	if(new_heater_value != sts3x.heater_on) {
		sts3x.heater_on  = new_heater_value;
		sts3x.new_heater = true;
	}

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_heater_configuration(const GetHeaterConfiguration *data, GetHeaterConfiguration_Response *response) {
	response->header.length = sizeof(GetHeaterConfiguration_Response);
	response->heater_config = sts3x.heater_on ? TEMPERATURE_V2_HEATER_CONFIG_ENABLED : TEMPERATURE_V2_HEATER_CONFIG_DISABLED;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

bool handle_temperature_callback(void) {
	return handle_callback_value_callback_int16_t(&callback_value_temperature, FID_CALLBACK_TEMPERATURE);
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	callback_value_init_int16_t(&callback_value_temperature, sts3x_get_temperature);

	communication_callback_init();
}
