using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public class ESP32Controller : MonoBehaviour
{
    public Text statusText;
    public Button connectButton;
    public Button leftButton;
    public Button rightButton;
    public Button forwardButton;
    public Button backwardButton;
    private string deviceName = "ESP32 Remote Control";
    private string serviceUUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    private string directionCharUUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
    private string speedCharUUID = "cd76e1c1-36e1-4688-b7f5-ea07361b27b9";
    private bool isConnected = false;
    private string connectedAddress;

    void Start()
    {
        BluetoothLEHardwareInterface.Initialize(true, false, () => {
            statusText.text = "Bluetooth initialized";
        }, (error) => {
            statusText.text = "Error initializing Bluetooth: " + error;
        });
        connectButton.onClick.AddListener(() => {
            ConnectToDevice();
        });
        AddEventTrigger(forwardButton, "F");
        AddEventTrigger(backwardButton, "B");
        AddEventTrigger(leftButton, "L");
        AddEventTrigger(rightButton, "R");
    }

    void AddEventTrigger(Button button, string command)
    {
        EventTrigger trigger = button.gameObject.AddComponent<EventTrigger>();
        
        EventTrigger.Entry pointerDownEntry = new EventTrigger.Entry { eventID = EventTriggerType.PointerDown };
        pointerDownEntry.callback.AddListener((data) => { StartCoroutine(SendCommandContinuously(command)); });
        trigger.triggers.Add(pointerDownEntry);
        
        EventTrigger.Entry pointerUpEntry = new EventTrigger.Entry { eventID = EventTriggerType.PointerUp };
        pointerUpEntry.callback.AddListener((data) => { StopAllCoroutines(); SendCommand("S"); });
        trigger.triggers.Add(pointerUpEntry);
    }

    IEnumerator SendCommandContinuously(string command)
    {
        while (true)
        {
            if (isConnected)
            {
                SendCommand(command);
            }
            yield return new WaitForSeconds(0.1f);  // Envía el comando cada 0.1 segundos
        }
    }

    void ConnectToDevice()
    {
        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) => {
            if (name.Contains(deviceName))
            {
                statusText.text = "Device found: " + name;
                BluetoothLEHardwareInterface.StopScan();
                connectedAddress = address;
                BluetoothLEHardwareInterface.ConnectToPeripheral(address, (addr) => {
                    statusText.text = "Connected to: " + name;
                }, null, (addr, serviceUUID, characteristicUUID) => {
                    isConnected = true;
                    statusText.text = "Characteristic found: " + characteristicUUID;
                }, (addr) => {
                    isConnected = false;
                    statusText.text = "Disconnected";
                });
            }
        }, (address, name, rssi, advertisingInfo) => {
            statusText.text = "Found: " + name + " RSSI: " + rssi.ToString();
        });
    }

    void SendCommand(string command)
    {
        string charUUID = (command == "L" || command == "R" || command == "S") ? directionCharUUID : speedCharUUID;
        byte[] bytes = System.Text.Encoding.UTF8.GetBytes(command);
        BluetoothLEHardwareInterface.WriteCharacteristic(connectedAddress, serviceUUID, charUUID, bytes, bytes.Length, true, (characteristicUUID) => {
            statusText.text = "Command sent: " + command;
        });
    }

    void OnDestroy()
    {
        if (isConnected)
        {
            BluetoothLEHardwareInterface.DisconnectPeripheral(connectedAddress, (address) => {
                isConnected = false;
                statusText.text = "Disconnected";
            });
        }
        BluetoothLEHardwareInterface.DeInitialize(() => {
            statusText.text = "Bluetooth deinitialized";
        });
    }
}