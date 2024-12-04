using System.Diagnostics;
using UnityEngine;
using UnityEngine.UI;
using Oculus.Interaction;

public class IMUBLEController : MonoBehaviour
{
    public string DeviceName = "ESP32 Neopixel Controller";  // Nombre del dispositivo BLE
    public string ServiceUUID = "A9E90000-194C-4523-A473-5FDF36AA4D18";  // UUID del servicio BLE
    public string ImuUUID = "A9E90002-194C-4523-A473-5FDF36AA4D18";  // UUID para el IMU
    public Text connectionStatus;  // Texto para el estado de la conexión
    public Text imuDataText;  // Texto para mostrar los datos del IMU
    public Text imuDataCanvasText;  // Texto para mostrar los datos del IMU en el Canvas
    public PokeInteractable pokeInteractable;  // Botón de interacción

    private bool isConnected = false;  // Bandera de conexión
    private string _deviceAddress;  // Dirección del dispositivo BLE
    private bool scanning = false;

    void Start()
    {
        UpdateConnectionStatus("Iniciando BLE...");

        // Inicializar BLE y configurar la suscripción al botón Poke
        BluetoothLEHardwareInterface.Initialize(true, false, () =>
        {
            UpdateConnectionStatus("BLE inicializado.");
        }, (error) =>
        {
            UpdateConnectionStatus("Error al inicializar BLE: " + error);
        });

        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised += OnPokeInteraction;
        }
    }

    void OnDestroy()
    {
        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised -= OnPokeInteraction;
        }
    }

    private void OnPokeInteraction(PointerEvent evt)
    {
        if (evt.Type == PointerEventType.Select && !scanning && !isConnected)
        {
            UpdateConnectionStatus("Iniciando escaneo BLE por botón...");
            ScanForDevice();
        }
    }

    void ConnectToDevice()
    {
        BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, null, null, (address, serviceUUID, characteristicUUID) =>
        {
            if (serviceUUID.Trim().ToLower() == ServiceUUID.ToLower() && characteristicUUID.Trim().ToLower() == ImuUUID.ToLower())
            {
                isConnected = true;
                UpdateConnectionStatus("Conectado al dispositivo ESP32");
                SubscribeToIMUData();
            }
        }, (disconnectedAddress) =>
        {
            isConnected = false;
            UpdateConnectionStatus("Dispositivo desconectado");
        });
    }

    void ScanForDevice()
    {
        scanning = true;
        UpdateConnectionStatus("Escaneando dispositivos BLE...");

        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            if (name.Contains(DeviceName))
            {
                BluetoothLEHardwareInterface.StopScan();
                _deviceAddress = address;
                ConnectToDevice();
                scanning = false;
            }
        });
    }

    void SubscribeToIMUData()
    {
        BluetoothLEHardwareInterface.SubscribeCharacteristicWithDeviceAddress(
            _deviceAddress,
            ServiceUUID,
            ImuUUID,
            null,
            OnIMUDataReceived
        );
    }

    private void OnIMUDataReceived(string deviceAddress, string characteristic, byte[] data)
    {
        if (data.Length >= 12)
        {
            float accelX = System.BitConverter.ToSingle(data, 0);
            float accelY = System.BitConverter.ToSingle(data, 4);
            float accelZ = System.BitConverter.ToSingle(data, 8);

            imuDataText.text = $"AccelX: {accelX:F2} AccelY: {accelY:F2} AccelZ: {accelZ:F2}";
            imuDataCanvasText.text = $"AccelX: {accelX:F2} AccelY: {accelY:F2} AccelZ: {accelZ:F2}";

            UnityEngine.Debug.Log($"IMU Data Received - AccelX: {accelX}, AccelY: {accelY}, AccelZ: {accelZ}");
        }
        else
        {
            UpdateConnectionStatus("Datos de IMU recibidos con tamaño inesperado.");
        }
    }

    private void UpdateConnectionStatus(string message)
    {
        connectionStatus.text = message;
    }
}
