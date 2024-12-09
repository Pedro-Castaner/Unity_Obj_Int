using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovePlayer : MonoBehaviour
{
	public float maximumLenght;
	private Ray rayMouse;
	private Vector3 direction;
	private Quaternion rotation;
	public Camera cam;
	private WaitForSeconds updateTime = new WaitForSeconds(0.01f);


	public void StartUpdateRay()
	{
		StartCoroutine(UpdateRay());
	}

	IEnumerator UpdateRay()
	{
		if (cam != null)
		{
				RaycastHit hit;
				var mousePos = Input.mousePosition;
				rayMouse = cam.ScreenPointToRay(mousePos);
				if (Physics.Raycast(rayMouse.origin, rayMouse.direction, out hit, maximumLenght))
					RotateToMouse(gameObject, hit.point);
				else
				{
					var pos = rayMouse.GetPoint(maximumLenght);
					RotateToMouse(gameObject, pos);
				}
			
			yield return updateTime;
			StartCoroutine(UpdateRay());
		}
	}

	void RotateToMouse(GameObject obj, Vector3 destination)
	{
		direction = destination - obj.transform.position;
		rotation = Quaternion.LookRotation(direction);
		obj.transform.localRotation = Quaternion.Lerp(obj.transform.rotation, rotation, 1);
	}

	public Quaternion GetRotation()
	{
		return rotation;
	}
}
