package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
	"github.com/Tinkerforge/go-api-bindings/temperature_v2_bricklet"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your Temperature Bricklet 2.0.

func main() {
	ipcon := ipconnection.New()
	defer ipcon.Close()
	t, _ := temperature_v2_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
	defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	t.RegisterTemperatureCallback(func(temperature int16) {
		fmt.Printf("Temperature: %f °C\n", float64(temperature)/100.0)
		fmt.Println("It is too hot, we need air conditioning!")
	})

	// Configure threshold for temperature "greater than 30 °C"
	// with a debounce period of 1s (1000ms).
	t.SetTemperatureCallbackConfiguration(1000, false, '>', 30*100, 0)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()

}
