using UnityEngine;
using Oculus.Interaction;
using UnityEngine.UI;

public class NeopixelBLEController : MonoBehaviour
{
    public string DeviceName = "ESP32 Neopixel Controller";  // Nombre del dispositivo BLE
    public string ServiceUUID = "A9E90000-194C-4523-A473-5FDF36AA4D18";  // UUID del servicio BLE
    public string LedUUID = "A9E90001-194C-4523-A473-5FDF36AA4D18";  // UUID de la característica para controlar el Neopixel

    public PokeInteractable pokeInteractable;  // El botón de interacción en VR
    public Grabbable varitaGrabbable;  // El componente Grabbable de la varita
    public Text connectionStatus;  // Texto para mostrar el estado de la conexión
    private bool isConnected = false;  // Bandera de conexión
    private string _deviceAddress;  // Dirección del dispositivo BLE

    private bool scanning = false;

    void Start()
    {
        UpdateConnectionStatus("Iniciando BLE...");

        // Inicializamos BLE
        BluetoothLEHardwareInterface.Initialize(true, false, () =>
        {
            UpdateConnectionStatus("BLE inicializado.");
            SetNeopixelColor(false);  // Rojo al inicio (no conectado)
        }, (error) =>
        {
            UpdateConnectionStatus("Error al inicializar BLE: " + error);
        });

        // Escuchar el evento del botón Poke
        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised += OnPokeInteraction;
        }
        else
        {
            UpdateConnectionStatus("Error: PokeInteractable no configurado.");
        }

        // Escuchar eventos de agarre de la varita
        if (varitaGrabbable != null)
        {
            varitaGrabbable.WhenPointerEventRaised += OnVaritaGrabEvent;
        }
        else
        {
            UpdateConnectionStatus("Error: Varita Grabbable no configurado.");
        }
    }

    private void OnDestroy()
    {
        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised -= OnPokeInteraction;
        }

        if (varitaGrabbable != null)
        {
            varitaGrabbable.WhenPointerEventRaised -= OnVaritaGrabEvent;
        }
    }

    // Método invocado al presionar el botón de Poke
    private void OnPokeInteraction(PointerEvent evt)
    {
        UpdateConnectionStatus("Poke detectado. Evento: " + evt.Type.ToString());

        if (evt.Type == PointerEventType.Select && !isConnected && !scanning)
        {
            UpdateConnectionStatus("Iniciando escaneo por BLE...");
            ScanForDevice();
        }
    }

    // Método invocado al agarrar o soltar la varita
    private void OnVaritaGrabEvent(PointerEvent evt)
    {
        if (evt.Type == PointerEventType.Select && isConnected)
        {
            // Cuando la varita es agarrada, cambia el LED a morado
            UpdateConnectionStatus("Varita agarrada, cambiando LED a morado.");
            SendColorAndMotorCommand(128, 0, 128, true);  // Morado y motor encendido
        }
        else if (evt.Type == PointerEventType.Unselect && isConnected)
        {
            // Cuando la varita es soltada, vuelve a verde
            UpdateConnectionStatus("Varita soltada, cambiando LED a verde.");
            SendColorAndMotorCommand(0, 255, 0, false);  // Verde y motor apagado
        }
    }

    // Enviar color y estado del motor en un solo comando
    public void SendColorAndMotorCommand(byte r, byte g, byte b, bool motorState)
    {
        byte[] command = new byte[4];
        command[0] = r;  // Rojo
        command[1] = g;  // Verde
        command[2] = b;  // Azul
        command[3] = (byte)(motorState ? 1 : 0);  // 1 para encender, 0 para apagar

        // Envía el comando a la característica BLE
        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, command, command.Length, true, (characteristicUUID) =>
        {
            UpdateConnectionStatus("Comando enviado: LED(" + r + ", " + g + ", " + b + ") Motor estado: " + (motorState ? "Encendido" : "Apagado"));
        });
    }

    // Actualizar el estado de conexión en la UI
    private void UpdateConnectionStatus(string message)
    {
        connectionStatus.text = message;
    }

    // Iniciar el escaneo para encontrar el dispositivo BLE
    void ScanForDevice()
    {
        scanning = true;
        UpdateConnectionStatus("Escaneando dispositivos BLE...");

        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            // Mostrar en el texto de la UI qué dispositivos se encuentran durante el escaneo
            UpdateConnectionStatus($"Dispositivo detectado: {name}");

            if (name.Contains(DeviceName))
            {
                UpdateConnectionStatus("Dispositivo encontrado: " + name);
                BluetoothLEHardwareInterface.StopScan();
                _deviceAddress = address;
                ConnectToDevice();
            }
            else
            {
                UpdateConnectionStatus($"Otro dispositivo encontrado: {name}, ignorando...");
            }
        }, (address, name, rssi, advertisingInfo) =>
        {
            UpdateConnectionStatus($"Dispositivo detectado: {name} con RSSI: {rssi}");
        });
    }

    // Conectar al dispositivo BLE encontrado
    void ConnectToDevice()
    {
        UpdateConnectionStatus("Conectando al dispositivo...");

        BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, null, null, (address, serviceUUID, characteristicUUID) =>
        {
            // Verificar si los UUID coinciden con los definidos
            if (serviceUUID.Trim().ToLower() == ServiceUUID.ToLower() && characteristicUUID.Trim().ToLower() == LedUUID.ToLower())
            {
                isConnected = true;
                UpdateConnectionStatus("Conectado al dispositivo ESP32");
                SetNeopixelColor(true);  // Cambia a verde cuando está conectado
            }
            else
            {
                UpdateConnectionStatus("UUID no coincide, no es el dispositivo esperado.");
            }
        }, (disconnectedAddress) =>
        {
            isConnected = false;
            UpdateConnectionStatus("Dispositivo desconectado");
            SetNeopixelColor(false);  // Rojo si se desconecta
        });
    }

    // Cambiar el color del Neopixel: verde para conectado, rojo para desconectado
    void SetNeopixelColor(bool connected)
    {
        byte r = connected ? (byte)0x00 : (byte)0xFF; // Rojo (255) si no está conectado
        byte g = connected ? (byte)0xFF : (byte)0x00; // Verde (255) si está conectado
        byte b = (byte)0x00; // Azul (0)

        // Agregar el estado del motor (0 para apagado, 1 para encendido)
        byte motorState = 0; // Cambiar a 1 si necesitas que el motor esté encendido
        byte[] data = new byte[] { r, g, b, motorState };

        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, data, data.Length, true, (characteristicUUID) =>
        {
            UpdateConnectionStatus(connected ? "Neopixel cambiado a verde." : "Neopixel cambiado a rojo.");
        });
    }

}
