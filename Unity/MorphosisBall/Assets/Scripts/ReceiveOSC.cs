using UnityEngine;
using System.Collections;

public class ReceiveOSC : MonoBehaviour {
    
   	public OSC osc;


	// Use this for initialization
	void Start () {
	   osc.SetAddressHandler("/motor/1", OnMotor1);
       osc.SetAddressHandler("/motor/2", OnMotor2);
       osc.SetAddressHandler("/morphoses/data", OnDirectData);
    }
	
	// Update is called once per frame
	void Update () {
	
	}

	void OnMotor1(OscMessage message) {
		int speed = message.GetInt (0);

		GetComponent<BallController>().speed = (speed / 255.0f) * 10.0f;
	}

	void OnMotor2(OscMessage message) {
		int steering = message.GetInt (0);

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
