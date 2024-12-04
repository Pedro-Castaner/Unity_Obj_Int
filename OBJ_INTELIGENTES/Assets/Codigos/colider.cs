using UnityEngine;

public class IndependentTrigger : MonoBehaviour
{
    public Transform wristTransform; // Asigna el Transform de b_r_wrist

    void Update()
    {
        if (wristTransform != null)
        {
            // Sincroniza la posición del trigger con la muñeca
            transform.position = wristTransform.position;

            // Mantén la rotación del trigger fija
            transform.rotation = Quaternion.identity;
        }
    }
}
