using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// Simulates the robo-ball dynamics by controlling the speed and steering.
public class InputControl : MonoBehaviour
{

  public float maxSpeed = 15.0f;
  public float maxSteering = 45.0f;

  //  public bool invert = false;

  void Start()
  {
  }

  void Update()
  {
    // Simple trick to go backwards.
    int mult = (Input.GetButton("Fire1") ? -1 : 1);

    float vertical = Input.GetAxis("Vertical") * mult;
    float horizontal = Input.GetAxis("Horizontal") * mult; ;

    //    Debug.Log(vertical + "," + horizontal);

    GetComponent<BallController>().speed = vertical * maxSpeed; //up & down
    GetComponent<BallController>().steering = horizontal * maxSteering; // left and right
  }
}
