using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Temperature Bricklet 2.0

	// Callback function for temperature callback
	static void TemperatureCB(BrickletTemperatureV2 sender, short temperature)
	{
		Console.WriteLine("Temperature: " + temperature/100.0 + " °C");
		Console.WriteLine("It is too hot, we need air conditioning!");
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletTemperatureV2 t = new BrickletTemperatureV2(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register temperature callback to function TemperatureCB
		t.TemperatureCallback += TemperatureCB;

		// Configure threshold for temperature "greater than 3000 °C"
		// with a debounce period of 1s (1000ms)
		t.SetTemperatureCallbackConfiguration(1000, false, '>', 3000*100, 0);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
