using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraShake : MonoBehaviour
{
    /// <summary>
    /// ////////////////////////////////////
    /// </summary>
    Vector3 orignalPosition;
    private void Start()
    {
        orignalPosition = transform.position;
    }
    public IEnumerator Shake(float duration, float magnitude)
    { 
        float elapsed = 0f;
        while (elapsed < duration)
        {
            float x = Random.Range(-1f, 1f) * magnitude;
            float y = Random.Range(-1f, 1f) * magnitude;
            transform.position = orignalPosition + new Vector3(x, y, -0.5f);
            elapsed += Time.deltaTime;
            yield return 0;
        }
        transform.position = orignalPosition;
    }
}
