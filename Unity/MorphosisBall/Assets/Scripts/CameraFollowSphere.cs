using UnityEngine;
using System.Collections;

public class CameraFollowSphere : MonoBehaviour {

  public Transform target;

  public Vector3 offsetPosition;

  void Start() {
    offsetPosition = transform.position - target.position;
  }

  void LateUpdate () {
        // Vector3 direction=player.GetComponent<Rigidbody>().velocity.normalized;
        //
        //
        // Vector3 offset= distance*direction*rotation;
        transform.position = target.transform.position + offsetPosition;
        transform.LookAt(target);
    }
}
