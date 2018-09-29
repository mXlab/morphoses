using UnityEngine;
using System.Collections;

// Sends OSC data from the robot (x, z, qx, qy, qz, qw).
public class SendOSC : MonoBehaviour {

	public OSC osc;

	// Use this for initialization
	void Start () {

	}

	// Update is called once per frame
	void FixedUpdate () {

		ExperimentRunner exp = GetComponent<ExperimentRunner>();
		if (exp.IsStarted()) {
		  OscMessage message = new OscMessage();

	    message.address = "/morphoses/data";
			// Experiment.
			message.values.Add(exp.GetId());
			// Time.
			message.values.Add((float)exp.GetTime());
			// Position.
			message.values.Add(transform.position.x);
			message.values.Add(transform.position.z);
			// Quaternions.
	    message.values.Add(transform.rotation.x);
			message.values.Add(transform.rotation.y);
			message.values.Add(transform.rotation.z);
			message.values.Add(transform.rotation.w);
	    osc.Send(message);
		}
  }

}
