using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SimpleSpellCaster : MonoBehaviour
{
    public GameObject firePoint; // Punto desde donde se dispara el hechizo
    public GameObject spellEffectPrefab; // Prefab del efecto visual del hechizo
    public float spellVelocity = 20f; // Velocidad del hechizo
    public float cooldownTime = 1f; // Tiempo entre disparos
    public NeopixelBLEController neopixelBLEController;

    public AudioSource audioSource; // AudioSource para el sonido del hechizo



    private float nextFireTime = 0f; // Control del tiempo de espera entre disparos
    private bool fire = false; // Evita múltiples disparos continuos mientras el gatillo está presionado

    void Update()
    {
        if (neopixelBLEController != null && neopixelBLEController.IsWristMoving())
        {
            // Aquí puedes implementar un retraso de 2 segundos antes de disparar el hechizo
            if (Time.time >= nextFireTime)
            {
                StartCoroutine(DelayedCastSpell(2f));
                nextFireTime = Time.time + cooldownTime + 2f; // Cooldown ajustado
            }
        }
    }

    private IEnumerator DelayedCastSpell(float delay)
    {
        yield return new WaitForSeconds(delay);
        CastSpell();
    }


    void CastSpell()
    {
        if (firePoint != null && spellEffectPrefab != null)
        {
            // Crear el hechizo
            GameObject spell = Instantiate(spellEffectPrefab, firePoint.transform.position, firePoint.transform.rotation);

            // Forzar escalado global
            spell.transform.localScale = new Vector3(0.3f, 0.3f, 0.3f); // Ajusta los valores según el tamaño deseado

            // Escalar partículas dentro del prefab (si existen)
            ParticleSystem[] particleSystems = spell.GetComponentsInChildren<ParticleSystem>();
            foreach (ParticleSystem ps in particleSystems)
            {
                var main = ps.main;
                main.startSizeMultiplier = 0.3f; // Escalar el tamaño inicial de las partículas
                main.startSpeedMultiplier = 0.3f; // Escalar la velocidad de las partículas
            }

            /* Evitar colisión inicial
            Collider spellCollider = spell.GetComponent<Collider>();
            Collider firePointCollider = firePoint.GetComponent<Collider>();
            if (spellCollider != null && firePointCollider != null)
            {
                Physics.IgnoreCollision(spellCollider, firePointCollider);
            }*/

            // Movimiento del hechizo
            Rigidbody rb = spell.GetComponent<Rigidbody>();
            if (rb != null)
            {
                rb.velocity = firePoint.transform.forward * spellVelocity;
            }
            // Reproducir el sonido
            if (audioSource != null)
            {
                audioSource.Play();
            }
            // Destruir después de 3 segundos
            Destroy(spell, 7f);
        }
    }
}