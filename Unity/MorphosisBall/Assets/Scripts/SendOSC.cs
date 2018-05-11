﻿using UnityEngine;
using System.Collections;

public class SendOSC : MonoBehaviour {

	public OSC osc;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void FixedUpdate () {

	  OscMessage message = new OscMessage();

        message.address = "/quat";
        message.values.Add(transform.rotation.x);
		message.values.Add(transform.rotation.y);
		message.values.Add(transform.rotation.z);
		message.values.Add(transform.rotation.w);
        osc.Send(message);

    }


}