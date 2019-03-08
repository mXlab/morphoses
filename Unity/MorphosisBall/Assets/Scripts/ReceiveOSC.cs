using UnityEngine;
using System.Collections;
using System.Collections.Generic;

// Receives OSC messages (x, z, qx, qy, qz, qw) and directly assigns them to the object.
public class ReceiveOSC : MonoBehaviour {

  public OSC osc;


  public float maxSpeed = 15.0f;
  public float maxSteering = 45.0f;

	// Use this for initialization
	void Start () {
	   osc.SetAddressHandler("/motor/1", OnMotor1);
       osc.SetAddressHandler("/motor/2", OnMotor2);
       osc.SetAddressHandler("/morphoses/action", OnAction);
       osc.SetAddressHandler("/morphoses/transform", OnDirectData);
    }

	// Update is called once per frame
	void Update () {

	}

	void OnMotor1(OscMessage message) {
		int speed = message.GetInt (0);

		GetComponent<BallController>().speed = (speed / 255.0f) * maxSpeed;
	}

	void OnMotor2(OscMessage message) {
		int steering = message.GetInt (0);

		GetComponent<BallController>().steering = steering;
	}

	void OnAction(OscMessage message) {
		float speed = message.GetFloat (0);
		float steering = message.GetFloat (1);

		Debug.Log("Received new speed: " + speed + " and steering: " + steering);

		GetComponent<BallController>().speed = speed;
		GetComponent<BallController>().steering = steering;
	}

  void OnDirectData(OscMessage message) {
    float x = message.GetFloat(0);
    float z = message.GetFloat(1);
    float qx = message.GetFloat(2);
    float qy = message.GetFloat(3);
    float qz = message.GetFloat(4);
    float qw = message.GetFloat(5);
    transform.position = new Vector3 (x, transform.position.y, z);
    transform.rotation = new Quaternion (qx, qy, qz, qw);
  }

}
