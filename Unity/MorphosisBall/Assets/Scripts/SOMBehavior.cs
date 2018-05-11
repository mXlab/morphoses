using System.Collections.Generic;
using System.Collections;
using UnityEngine;
//using DisruptorUnity3d;

public class SOMBehavior : MonoBehaviour {

	SOM som = new SOM(6, 11, 11, 0.1f);

//	RingBuffer<Vector2> buffer = new RingBuffer<Vector3> (1000);

	public float learningRate = 0.01f;

	// Use this for initialization
	void Start () {
		som.setup ();
	}
	
	// Update is called once per frame
	void FixedUpdate () {
		som.learningRate = learningRate;

		// Collect data.
		List<float> input = new List<float>();

		// Position.
		input.Add (transform.position.x * 0.1f);
//		input.Add (transform.position.y);
		input.Add (transform.position.z * 0.1f);

		// Ground covered.
		/*
		Vector2 currentPosition = new Vector2 (transform.position.x, transform.position.z);
		Vector2 prevPosition;
		if (buffer.Count == buffer.Capacity)// buffer full
			prevPosition = buffer.Dequeue ();
		else
			prevPosition = buffer [0];
		buffer.Enqueue (currentPosition);
		float distance = Vector2.Distance (currentPosition, prevPosition);
		input.Add (distance * 0.1f);
		Debug.Log (distance);*/

		// Rotation.
//		input.Add (transform.eulerAngles.x / Mathf.PI);
//		input.Add (transform.eulerAngles.y / Mathf.PI);
//		input.Add (transform.eulerAngles.z / Mathf.PI);

		// Quaternion
		input.Add (transform.rotation.x);
		input.Add (transform.rotation.y);
		input.Add (transform.rotation.z);
		input.Add (transform.rotation.w);
		Debug.Log ("Quaternion : " + transform.rotation.x + "," + transform.rotation.y + "," + transform.rotation.z + "," + transform.rotation.w);

		// Distance covered.

		// Process data.
		int cluster = som.step(input);

		// Map cluster to action.
		int speedCluster    = som.getX(cluster);
		int steeringCluster = som.getY(cluster);
		float speed    = (speedCluster - 5) / 5.0f * (10.0f);
		float steering = (steeringCluster - 5) / 5.0f * (45.0f);

		Debug.Log (" " + cluster + " => " + speedCluster + "," + steeringCluster);

		GetComponent<BallController> ().speed = speed;
		GetComponent<BallController> ().steering = steering;
	}
}
