using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// Simulates the robo-ball dynamics by controlling the speed and steering.
public class BallController : MonoBehaviour
{

  public float radius;

  public float phi;
  public float theta;

  // Speed (in degrees per second).
  public float speed;

  // Steering (in degrees).
  public float steering;

  public Rigidbody rb;

  void Start()
  {
    rb = GetComponent<Rigidbody>();
  }

  void Update()
  {
    // Update new center of mass according to dynamics.
    phi += speed * Time.deltaTime;
    theta = steering;
    //        speed    = Input.GetAxis("Vertical")   * Mathf.PI * 0.1f * Time.deltaTime;//up & Down
    //        steering = Input.GetAxis("Horizontal") * Mathf.PI * 0.25f;//Left and right
  }

  void FixedUpdate()
  {
    if (transform.position.y >= 0.4999f)
    {
      transform.position = new Vector3(transform.position.x, 0.4999f, transform.position.z);
    }
    // Assign center of mass.
    SetCenterOfMass();
  }

  void SetCenterOfMass()
  {
    float phiRad = Mathf.Deg2Rad * phi;
    float thetaRad = Mathf.Deg2Rad * theta;
    rb.centerOfMass = new Vector3(-radius * Mathf.Cos(phiRad), radius * Mathf.Sin(thetaRad), -radius * Mathf.Sin(phiRad));
    //rb.centerOfMass = new Vector3(- radius * Mathf.Cos (phiRad), - radius * Mathf.Sin (phiRad), - radius * Mathf.Sin (thetaRad));
    //com.Set(- radius * Mathf.Cos (phiRad), - radius * Mathf.Sin (phiRad), radius * Mathf.Sin (thetaRad));
  }
}
