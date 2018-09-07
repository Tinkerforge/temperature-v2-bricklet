Imports System
Imports Tinkerforge

Module ExampleThreshold
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Temperature Bricklet 2.0

    ' Callback subroutine for temperature callback
    Sub TemperatureCB(ByVal sender As BrickletTemperatureV2, ByVal temperature As Short)
        Console.WriteLine("Temperature: " + (temperature/100.0).ToString() + " °C")
        Console.WriteLine("It is too hot, we need air conditioning!")
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim t As New BrickletTemperatureV2(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register temperature callback to subroutine TemperatureCB
        AddHandler t.TemperatureCallback, AddressOf TemperatureCB

        ' Configure threshold for temperature "greater than 3000 °C"
        ' with a debounce period of 1s (1000ms)
        t.SetTemperatureCallbackConfiguration(1000, False, ">"C, 3000*100, 0)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
