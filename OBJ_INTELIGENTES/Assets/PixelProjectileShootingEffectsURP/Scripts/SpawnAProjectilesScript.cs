using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SpawnAProjectilesScript : MonoBehaviour {

	public MovePlayer movePlayer;
	public CameraShake cameraShake;
	public GameObject firePoint;
	private GameObject effectToSpawn;
	public List<GameObject> VFXs = new List<GameObject>();
	private float timeToFire = 0f;
	int i = 0;
	private void Start()
	{
		movePlayer.StartUpdateRay();
	}
	void Update()
	{
		if (VFXs.Count > 0)
		{
			if (VFXs.Count > 0)
				effectToSpawn = VFXs[i];
			if (Input.GetMouseButton(0) && Time.time >= timeToFire)
			{
				StartCoroutine(cameraShake.Shake(0.5f, 0.1f));
				timeToFire = Time.time + 10f / effectToSpawn.GetComponent<ProjectileMoveScript>().fireRate;
				SpawnVFX();
			}
		}
		if (Input.GetKeyDown(KeyCode.Space))
		{
			i++;
			if (i == VFXs.Count)
				i = 0;
		}
	}

	public void SpawnVFX()
	{
		GameObject vfx;
		if (firePoint != null)
		{
			vfx = Instantiate(effectToSpawn, firePoint.transform.position, Quaternion.identity);
			if (movePlayer != null)
				vfx.transform.localRotation = movePlayer.GetRotation();
		}
		else
			vfx = Instantiate(effectToSpawn);
	}

}