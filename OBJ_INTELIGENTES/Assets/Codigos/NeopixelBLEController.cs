using UnityEngine;
using Oculus.Interaction;
using UnityEngine.UI;
using System.Collections;
using UnityEngine.Android;

public class NeopixelBLEController : MonoBehaviour
{
    public string DeviceName = "ESP32 Neopixel Controller";
    public string ServiceUUID = "A9E90000-194C-4523-A473-5FDF36AA4D18";
    public string LedUUID = "A9E90001-194C-4523-A473-5FDF36AA4D18";

    public PokeInteractable pokeInteractable;
    public Grabbable varitaGrabbable;
    public Text connectionStatus;
    public Text Motors;
    public Text boolisWristMoving;
    public GameObject b_l_wrist; // Mano izquierda
    public GameObject b_r_wrist; // Mano derecha

    private bool isConnected = false;
    private string _deviceAddress;
    private bool scanning = false;

    private byte motorState = 0;
    private bool isVaritaGrabbed = false;
    private bool isWristMoving = false;
    private Vector3 lastWristPosition;
    private float lastUpdateTime = 0;
    private const float positionThreshold = 0.1f;

    private GameObject currentWrist; // Mano actual que sostiene la varita
    private Coroutine resetMotorCoroutine;

    void Start()
    {
        // Manejar permisos de ubicación y Bluetooth
        HandlePermissions();

        UpdateConnectionStatus("Iniciando BLE...");

        BluetoothLEHardwareInterface.Initialize(true, false, () =>
        {
            UpdateConnectionStatus("BLE inicializado.");
            SetNeopixelColor(false);
        }, (error) =>
        {
            UpdateConnectionStatus("Error al inicializar BLE: " + error);
        });

        if (pokeInteractable != null)
        {
            pokeInteractable.WhenPointerEventRaised += OnPokeInteraction;
        }

        if (varitaGrabbable != null)
        {
            varitaGrabbable.WhenPointerEventRaised += OnVaritaGrabEvent;
        }
    }

    private void HandlePermissions()
    {
        if (!Permission.HasUserAuthorizedPermission(Permission.FineLocation))
        {
            Permission.RequestUserPermission(Permission.FineLocation);
        }

        if (Application.platform == RuntimePlatform.Android)
        {
            if (!Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_SCAN"))
            {
                Permission.RequestUserPermission("android.permission.BLUETOOTH_SCAN");
            }
            if (!Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_CONNECT"))
            {
                Permission.RequestUserPermission("android.permission.BLUETOOTH_CONNECT");
            }
        }
    }

    private void Update()
    {
        if (currentWrist != null && isVaritaGrabbed && isConnected)
        {
            Vector3 currentWristPosition = currentWrist.transform.position;
            float distanceMoved = Vector3.Distance(currentWristPosition, lastWristPosition);

            if (distanceMoved > positionThreshold)
            {
                if (Time.time - lastUpdateTime <= 0.2f)
                {
                    UpdateMotorState(2);
                }

                lastWristPosition = currentWristPosition;
                lastUpdateTime = Time.time;
            }
        }
    }

    private void UpdateMotorState(byte newState)
    {
        if (motorState != newState)
        {
            motorState = newState;

            if (motorState == 2)
            {
                isWristMoving = true;

                if (resetMotorCoroutine != null)
                {
                    StopCoroutine(resetMotorCoroutine);
                }
                resetMotorCoroutine = StartCoroutine(ResetMotorStateAfterDelay(2f));
            }
            else if (motorState == 0)
            {
                isWristMoving = false;
            }

            boolisWristMoving.text = $"isWristMoving: {isWristMoving}";
            SendColorAndMotorCommand(0, 0, 255, motorState);
            Motors.text = $"MotorState: {motorState}, WristMoving: {isWristMoving}";
        }
    }

    private IEnumerator ResetMotorStateAfterDelay(float delay)
    {
        yield return new WaitForSeconds(delay);

        motorState = 0;
        isWristMoving = false;
        SendColorAndMotorCommand(0, 0, 255, motorState);
        Motors.text = $"MotorState: {motorState}, WristMoving: {isWristMoving}";
        boolisWristMoving.text = $"isWristMoving: {isWristMoving}";
    }

    private void OnPokeInteraction(PointerEvent evt)
    {
        if (evt.Type == PointerEventType.Select && !isConnected && !scanning)
        {
            ScanForDevice();
        }
    }

    private void OnVaritaGrabEvent(PointerEvent evt)
    {
        if (evt.Type == PointerEventType.Select && isConnected)
        {
            isVaritaGrabbed = true;
            UpdateCurrentWrist();
            SetNeopixelColor(true, 128, 0, 128);
        }
        else if (evt.Type == PointerEventType.Unselect && isConnected)
        {
            isVaritaGrabbed = false;
            currentWrist = null;
            SetNeopixelColor(true, 0, 255, 0);
        }
    }

    private void UpdateCurrentWrist()
    {
        if (b_l_wrist != null && Vector3.Distance(varitaGrabbable.transform.position, b_l_wrist.transform.position) < Vector3.Distance(varitaGrabbable.transform.position, b_r_wrist.transform.position))
        {
            currentWrist = b_l_wrist;
        }
        else
        {
            currentWrist = b_r_wrist;
        }
        lastWristPosition = currentWrist.transform.position;
    }

    public void SendColorAndMotorCommand(byte r, byte g, byte b, byte motorState)
    {
        byte[] command = new byte[4] { r, g, b, motorState };

        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, command, command.Length, true, (characteristicUUID) =>
        {
            UpdateConnectionStatus($"Comando enviado: LED({r}, {g}, {b}) Motor estado: {motorState}");
        });
    }

    private void UpdateConnectionStatus(string message)
    {
        connectionStatus.text = message;
    }

    void ScanForDevice()
    {
        scanning = true;

        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            if (name.Contains(DeviceName))
            {
                BluetoothLEHardwareInterface.StopScan();
                _deviceAddress = address;
                ConnectToDevice();
            }
        }, null);
    }

    void ConnectToDevice()
    {
        BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, null, null, (address, serviceUUID, characteristicUUID) =>
        {
            if (serviceUUID.ToLower() == ServiceUUID.ToLower() && characteristicUUID.ToLower() == LedUUID.ToLower())
            {
                isConnected = true;
                SetNeopixelColor(true, 0, 255, 0);
            }
        }, (disconnectedAddress) =>
        {
            isConnected = false;
            SetNeopixelColor(false);
        });
    }

    void SetNeopixelColor(bool connected, byte r = 0, byte g = 255, byte b = 0)
    {
        byte motorState = 0;
        byte[] data = new byte[] { r, g, b, motorState };

        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, data, data.Length, true, null);
    }
}
