using UnityEngine;
using Oculus.Interaction;
using UnityEngine.UI;

public class NeopixelBLEController : MonoBehaviour
{
    public string DeviceName = "ESP32 Neopixel Controller";  // Nombre del dispositivo BLE
    public string ServiceUUID = "A9E90000-194C-4523-A473-5FDF36AA4D18";  // UUID del servicio BLE
    public string LedUUID = "A9E90001-194C-4523-A473-5FDF36AA4D18";  // UUID de la característica para controlar el Neopixel

    public PokeInteractable pokeInteractable;  // El botón de interacción en VR
    public Text connectionStatus;  // Texto para mostrar el estado de la conexión
    public Text debugText;  // Nuevo texto para mostrar los mensajes de depuración en la UI
    private bool isConnected = false;  // Bandera de conexión
    private string _deviceAddress;  // Dirección del dispositivo BLE

    private bool scanning = false;

    void Start()
    {
        // Verificación inicial de que el Text está asignado
        if (debugText == null)
        {
            UnityEngine.Debug.LogError("Debug Text no está asignado en el Inspector.");
        }

        UpdateConnectionStatus("Iniciando BLE...");
        UpdateDebugText("Iniciando BLE...");

        // Inicializamos BLE
        BluetoothLEHardwareInterface.Initialize(true, false, () =>
        {
            UpdateConnectionStatus("BLE inicializado.");
            UpdateDebugText("BLE inicializado.");
            SetNeopixelColor(false);  // Rojo al inicio (no conectado)
        }, (error) =>
        {
            UpdateConnectionStatus("Error al inicializar BLE: " + error);
            UpdateDebugText("Error al inicializar BLE: " + error);
        });

        // Escuchar el evento del botón Poke
        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised += OnPokeInteraction;
        }
        else
        {
            UpdateConnectionStatus("Error: PokeInteractable no configurado.");
            UpdateDebugText("Error: PokeInteractable no configurado.");
        }
    }

    private void OnDestroy()
    {
        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised -= OnPokeInteraction;
        }
    }

    // Método invocado al presionar el botón de Poke
    private void OnPokeInteraction(PointerEvent evt)
    {
        UpdateConnectionStatus("Poke detectado. Evento: " + evt.Type.ToString());
        UpdateDebugText("Poke detectado. Evento: " + evt.Type.ToString());

        if (evt.Type == PointerEventType.Select && !isConnected && !scanning)
        {
            UpdateConnectionStatus("Iniciando escaneo por BLE...");
            UpdateDebugText("Iniciando escaneo por BLE...");
            ScanForDevice();
        }
    }

    // Actualizar el estado de conexión en la UI
    private void UpdateConnectionStatus(string message)
    {
        connectionStatus.text = message;
    }

    // Actualizar el texto de depuración en la UI
    private void UpdateDebugText(string message)
    {
        if (debugText != null)
        {
            debugText.text += message + "\n";  // Agregar cada mensaje nuevo
            UnityEngine.Debug.Log("Texto de debug actualizado: " + message);  // Agregar log en consola para ver si está llamando a esta función
        }
        else
        {
            UnityEngine.Debug.LogError("Debug Text es nulo, no se puede actualizar.");
        }
    }

    // Iniciar el escaneo para encontrar el dispositivo BLE
    void ScanForDevice()
    {
        scanning = true;
        UpdateConnectionStatus("Escaneando dispositivos BLE...");
        UpdateDebugText("Escaneando dispositivos BLE...");

        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            // Mostrar en el texto de la UI qué dispositivos se encuentran durante el escaneo
            UpdateConnectionStatus($"Dispositivo detectado: {name}");
            UpdateDebugText($"Dispositivo detectado: {name}");

            if (name.Contains(DeviceName))
            {
                UpdateConnectionStatus("Dispositivo encontrado: " + name);
                UpdateDebugText("Dispositivo encontrado: " + name);
                BluetoothLEHardwareInterface.StopScan();
                _deviceAddress = address;
                ConnectToDevice();
            }
            else
            {
                UpdateConnectionStatus($"Otro dispositivo encontrado: {name}, ignorando...");
                UpdateDebugText($"Otro dispositivo encontrado: {name}, ignorando...");
            }
        }, (address, name, rssi, advertisingInfo) =>
        {
            UpdateConnectionStatus($"Dispositivo detectado: {name} con RSSI: {rssi}");
            UpdateDebugText($"Dispositivo detectado: {name} con RSSI: {rssi}");
        });
    }

    // Conectar al dispositivo BLE encontrado
    void ConnectToDevice()
    {
        UpdateConnectionStatus("Conectando al dispositivo...");
        UpdateDebugText("Conectando al dispositivo...");

        BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, null, null, (address, serviceUUID, characteristicUUID) =>
        {
            // Log para mostrar los UUIDs recibidos
            UpdateDebugText($"Servicio UUID recibido: {serviceUUID}, Característica UUID recibida: {characteristicUUID}");

            // Verificar si los UUID coinciden con los definidos
            if (serviceUUID == ServiceUUID && characteristicUUID == LedUUID)
            {
                isConnected = true;
                UpdateConnectionStatus("Conectado al dispositivo ESP32");
                UpdateDebugText("Conectado al dispositivo ESP32");
                SetNeopixelColor(true);  // Cambia a verde cuando está conectado
            }
            else
            {
                UpdateConnectionStatus("UUID no coincide, no es el dispositivo esperado.");
                UpdateDebugText("UUID no coincide, no es el dispositivo esperado.");
            }
        }, (disconnectedAddress) =>
        {
            isConnected = false;
            UpdateConnectionStatus("Dispositivo desconectado");
            UpdateDebugText("Dispositivo desconectado");
            SetNeopixelColor(false);  // Rojo si se desconecta
        });
    }

    // Cambiar el color del Neopixel: verde para conectado, rojo para desconectado
    void SetNeopixelColor(bool connected)
    {
        byte[] data = connected ? new byte[] { 0x01 } : new byte[] { 0x00 };  // 0x01 es verde, 0x00 es rojo
        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, data, data.Length, true, (characteristicUUID) =>
        {
            UpdateConnectionStatus("Neopixel color cambiado.");
            UpdateDebugText("Neopixel color cambiado.");
        });
    }
}
