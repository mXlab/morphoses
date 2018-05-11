using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Movement : MonoBehaviour {

	public float speed;
    public float steering;

	void Start ()
	{
	}
	/*
	void Update () 
	{
//        speed    = Input.GetAxis("Vertical")   * Mathf.PI * 0.1f * Time.deltaTime;//up & Down
//        steering = Input.GetAxis("Horizontal") * Mathf.PI * 0.25f;//Left and right
        GetComponent<BallRotationController>().phi  += speed;
        GetComponent<BallRotationController>().theta = steering ;

        //Fire Button
        if (Input.GetButton ("Fire1"))
		{
			Debug.Log ("Fire Button was pressed");
		}

    }

	void Debuger()
    {
        //Show position
        Debug.Log("Position" + "X:" + this.gameObject.transform.position.x + "  Y:" + this.gameObject.transform.position.y + "  Z:" + this.gameObject.transform.position.z);

        //show rotation
        Debug.Log("Rotation" + "X:" + this.gameObject.transform.rotation.x + "  Y:" + this.gameObject.transform.rotation.y + "  Z:" + this.gameObject.transform.rotation.z);

    }
	*/
}
