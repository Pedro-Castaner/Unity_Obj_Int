using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using UnityEngine.Networking;
using System.Collections; // Asegúrate de incluir esto para IEnumerator

public class ESP32ControllerWeb : MonoBehaviour
{
    public Text statusText;
    public Button connectButton;
    public Button leftButton;
    public Button rightButton;
    public Button forwardButton;
    public Button backwardButton;

    private string esp32IP = "http://192.168.0.1"; // Cambia esto a la dirección IP de tu ESP32
    private bool isConnected = false;

    void Start()
    {
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
        pointerDownEntry.callback.AddListener((data) => { if (isConnected) StartCoroutine(SendCommand(command)); });
        trigger.triggers.Add(pointerDownEntry);

        EventTrigger.Entry pointerUpEntry = new EventTrigger.Entry { eventID = EventTriggerType.PointerUp };
        pointerUpEntry.callback.AddListener((data) => { if (isConnected) StartCoroutine(SendCommand("S")); });
        trigger.triggers.Add(pointerUpEntry);
    }

    void ConnectToDevice()
    {
        StartCoroutine(CheckConnection());
    }

    IEnumerator CheckConnection()
    {
        UnityWebRequest request = UnityWebRequest.Get(esp32IP + "/");
        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.ConnectionError || request.result == UnityWebRequest.Result.ProtocolError)
        {
            statusText.text = "Error connecting to ESP32: " + request.error;
            isConnected = false;
        }
        else
        {
            statusText.text = "Connected to ESP32";
            isConnected = true;
        }
    }

    IEnumerator SendCommand(string command)
    {
        string url = esp32IP;
        if (command == "L" || command == "R")
        {
            url += "/direction?dir=" + command;
        }
        else
        {
            url += "/speed?spd=" + command;
        }

        UnityWebRequest request = UnityWebRequest.Get(url);
        yield return request.SendWebRequest();

        if (request.result == UnityWebRequest.Result.ConnectionError || request.result == UnityWebRequest.Result.ProtocolError)
        {
            statusText.text = "Error sending command: " + request.error;
        }
        else
        {
            statusText.text = "Command sent: " + command;
        }
    }

    void OnDestroy()
    {
        // No se requiere desconexión explícita para HTTP
    }
}
