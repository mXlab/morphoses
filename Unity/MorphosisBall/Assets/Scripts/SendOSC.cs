using UnityEngine;
using System.Collections;

// Sends OSC data from the robot (x, z, qx, qy, qz, qw, speed, steering).
public class SendOSC : MonoBehaviour
{

  public OSC osc;

  private BallController ball_controller;

  // Use this for initialization
  void Start()
  {

  }

  // Update is called once per frame
  void FixedUpdate()
  {

    ExperimentRunner exp = GetComponent<ExperimentRunner>();
    if (exp.IsStarted())
    {
      ball_controller = GetComponent<BallController>();

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
      // inputs
      message.values.Add(ball_controller.speed);
      message.values.Add(ball_controller.steering);
      osc.Send(message);
    }
  }

}
