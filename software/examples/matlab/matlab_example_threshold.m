function matlab_example_threshold()
    import com.tinkerforge.IPConnection;
    import com.tinkerforge.BrickletTemperatureV2;

    HOST = 'localhost';
    PORT = 4223;
    UID = 'XYZ'; % Change XYZ to the UID of your Temperature Bricklet 2.0

    ipcon = IPConnection(); % Create IP connection
    t = handle(BrickletTemperatureV2(UID, ipcon), 'CallbackProperties'); % Create device object

    ipcon.connect(HOST, PORT); % Connect to brickd
    % Don't use device before ipcon is connected

    % Register temperature callback to function cb_temperature
    set(t, 'TemperatureCallback', @(h, e) cb_temperature(e));

    % Configure threshold for temperature "greater than 30 °C"
    % with a debounce period of 1s (1000ms)
    t.setTemperatureCallbackConfiguration(1000, false, '>', 30*100, 0);

    input('Press key to exit\n', 's');
    ipcon.disconnect();
end

% Callback function for temperature callback
function cb_temperature(e)
    fprintf('Temperature: %g °C\n', e.temperature/100.0);
    fprintf('It is too hot, we need air conditioning!\n');
end
