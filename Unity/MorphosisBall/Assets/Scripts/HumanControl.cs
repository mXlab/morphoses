using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// Simulates the robo-ball dynamics by controlling the speed and steering.
public class HumanControl : MonoBehaviour {

  public float maxSpeed    = 15.0f;
  public float maxSteering = 45.0f;

  void Start ()
	{
	}

	void Update ()
	{
    float vertical = Input.GetAxis("Vertical");
    float horizontal = Input.GetAxis("Horizontal");

    Debug.Log(vertical + "," + horizontal);

    GetComponent<BallController>().speed    = vertical * maxSpeed; //up & fown
    GetComponent<BallController>().steering = horizontal * maxSteering; // left and right
  }
}
