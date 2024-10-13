using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class AppControl : MonoBehaviour
{
    public Button resetButton;
    public Button quitButton;

    void Start()
    {
        // Asignar los métodos a los botones
        if (resetButton != null)
        {
            resetButton.onClick.AddListener(ResetApp);
        }

        if (quitButton != null)
        {
            quitButton.onClick.AddListener(QuitApp);
        }
    }

    // Método para reiniciar la aplicación
    void ResetApp()
    {
        SceneManager.LoadScene(SceneManager.GetActiveScene().buildIndex);
    }

    // Método para salir de la aplicación
    void QuitApp()
    {
        #if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
        #else
            Application.Quit();
        #endif
    }
}
