import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletTemperatureV2;

public class ExampleThreshold {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your Temperature Bricklet 2.0
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletTemperatureV2 t = new BrickletTemperatureV2(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Add temperature listener
		t.addTemperatureListener(new BrickletTemperatureV2.TemperatureListener() {
			public void temperature(int temperature) {
				System.out.println("Temperature: " + temperature/100.0 + " °C");
				System.out.println("It is too hot, we need air conditioning!");
			}
		});

		// Configure threshold for temperature "greater than 3000 °C"
		// with a debounce period of 1s (1000ms)
		t.setTemperatureCallbackConfiguration(1000, false, '>', 3000*100, 0);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
