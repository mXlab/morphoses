using UnityEngine;
using System.Collections;

public class CameraFollowSphereVelocity : MonoBehaviour {

     // http://answers.unity3d.com/questions/34092/camera-follow-a-rolling-object.html

     // The target we are following
     public Transform target;
     private Rigidbody targetRB;

     // The distance in the x-z plane to the target
     public float distance = 10.0f;
     // the height we want the camera to be above the target
     public float height = 5.0f;
     // How much we damp
     public float heightDamping = 2.0f;
     public float rotationDamping = 3.0f;

     void Awake()
     {
         if (target != null)
             targetRB = target.GetComponent<Rigidbody>();
     }


     void LateUpdate()
     {
         // Early out if we don't have a target
         if (!target || !targetRB)
             return;

         Debug.Log(targetRB.velocity.magnitude);
         if (targetRB.velocity.magnitude < 0.001)
             return;

         //Take the target's velocity, remove the y and normalize it.  The final step is probably unneeded but makes it easier to extend.
         Vector3 direction = targetRB.velocity;
         direction.y = 0;
         direction = direction.normalized;

         //Get the desired rotation from the ball's velocity.
         Quaternion wantedRotation = Quaternion.LookRotation(direction);
         float wantedRotationAngle = wantedRotation.eulerAngles.y;
         float wantedHeight = target.position.y + height;

         //Our current rotation is the ball's position minus our own.
         Quaternion currentRotation = Quaternion.LookRotation(target.position - transform.position);
         float currentRotationAngle = currentRotation.eulerAngles.y;
         float currentHeight = transform.position.y;

         // Damp the rotation around the y-axis
         currentRotationAngle = Mathf.LerpAngle(currentRotationAngle, wantedRotationAngle, rotationDamping * Time.deltaTime);
         // Damp the height
         currentHeight = Mathf.Lerp(currentHeight, wantedHeight, heightDamping * Time.deltaTime);
         // Convert the angle into a rotation
         Quaternion finalRotation = Quaternion.Euler(0, currentRotationAngle, 0);

         // Set the position of the camera on the x-z plane to:
         // distance meters behind the target
         transform.position = target.position;
         transform.position -= finalRotation * Vector3.forward * distance;

         // Set the height of the camera
         Vector3 newPosition = new Vector3(transform.position.x, currentHeight, transform.position.z);
         transform.position = newPosition;
         // Always look at the target
         transform.LookAt(target);
     }
 }
