using UnityEngine;
using UnityEngine.Android;

public class PermissionRequest : MonoBehaviour
{
    void Start()
    {
        // Verificar si el permiso de ubicación ya ha sido concedido (necesario para BLE)
        if (!Permission.HasUserAuthorizedPermission(Permission.FineLocation))
        {
            // Solicitar el permiso de ubicación (BLE requiere ubicación)
            Permission.RequestUserPermission(Permission.FineLocation);
        }

        // Si estamos en Android 12 o superior, también solicitamos los permisos específicos de Bluetooth
        if (SystemInfo.operatingSystem.Contains("Android 12"))
        {
            // Verificar y solicitar permiso para escanear Bluetooth (BLUETOOTH_SCAN)
            if (!Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_SCAN"))
            {
                Permission.RequestUserPermission("android.permission.BLUETOOTH_SCAN");
            }

            // Verificar y solicitar permiso para conectar dispositivos Bluetooth (BLUETOOTH_CONNECT)
            if (!Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_CONNECT"))
            {
                Permission.RequestUserPermission("android.permission.BLUETOOTH_CONNECT");
            }

            // Verificar y solicitar permiso para anunciar dispositivos Bluetooth (BLUETOOTH_ADVERTISE)
            if (!Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_ADVERTISE"))
            {
                Permission.RequestUserPermission("android.permission.BLUETOOTH_ADVERTISE");
            }
        }
    }
}
