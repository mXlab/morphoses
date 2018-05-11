using UnityEngine;
using System.Collections;

public class ReceiveOSC : MonoBehaviour {
    
   	public OSC osc;


	// Use this for initialization
	void Start () {
	   osc.SetAddressHandler("/motor/1", OnMotor1);
       osc.SetAddressHandler("/motor/2", OnMotor2);
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

}
