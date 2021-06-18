# Model prototype description

In this section I describe the model prototype designed for reinforcement learning within the Morphoses ball. The model was specifically designed to address Morphoses' (1) interactive, short-time learning use case, and (2) material-driven, noisy data flows coming from the robotic ball.

## State-action formalisation

The robot's state consists in its polar coordinates relative to the target (dist, dir), discretised into six different values. dist equals to 1 when the robot is in a neighborhood of the target, and 0 when it is not; dir can be front, back, left, or right.

In the current implementation, state computing only relies on RTLS data. To get dist, I first compute the Euclidean distance between robot position and target position, and convert it to 1 or 0 when it is higher or lower than a target radius, respectively. To get dir, I first compute the robot's instantaneous orientation by subtracting the robot's previous position to its current position. I then compute the vector between the robot's current position and the target point position. Normalizing the two vectors, computing their dot product, and taking its arc cosine gives the angle between the robot and the target point. To transform this continuous angular value into a discretised state, I divide it into four 90-degree angular extents respectively corresponding to the robot's instantaneous forward, backward, left, and right direction. This discretisation reduces state space dimensionality, thus enabling to compensate for the noisy angular data obtained from the RTLS. Future implementation for the robot's direction may rely on the robot's quaternions obtained from the robot's IMU.

From each of these states, four elementary actions are available: going forward, going backward, oscillating left, oscillating right.

In the current implementation, going forward or backward consists in commanding the robot's motor to go at a +15 or -15 degrees per second speed. Oscillating left or right consists in commanding the robot's motor to take a +45 or -45 degree steer. This basic formalisation reduces action space dimensionality, and compensate for the displacement randomness due to the irregularity of the ball's skin. Typically, going forward might make the robot go forward plus some random deviation to the left or right, which could be learned using machine learning. Importantly, oscillating left and right should let the robot stay at the same position. This enables the robot to stabilise its position by accounting for a "not moving" behaviour. Future implementation may experiment with only one action instead of two for oscillating—for the moment, this choice was only due to the bin implementation of the preprocessing script.

## Reward function

The reward function possesses two levels, and is based on the distance between the robot and the target. The first level operates when the robot is outside a neighborhood of the target, defined by the target radius. Within this first level, the robot gets a strong punishment when it moves away from the target, as well as when it stands still far away from the target. The robot gets a medium-low reward when it gets closer to the target. The second level operates when the robot is inside a neighborhood of the target. Within this second level, the robot still gets a strong punishment when it moves away from the target. Yet, it gets a strong reward when it stands still close to the target. Lastly, it gets a medium-high reward when it gets closer to the target.

Crucially, the resulting state-action-reward formalisation is symmetrical relative to the robot-target system. This means that by design, what the robot will learn relative to one target position will be applicable to another target position. This drastically reduces the amount of exploration needed to learn a representation of the environment, which is one of the requirements of the interactive, short-time learning use case of the Morphoses ball. Future work may experiment with continuous reward functions relative to the distance between robot and target (e.g., proportional), as well as with continuous distance and angle values for states.

## Learning algorithm

A tabular model enables to learn action values at each discrete state on a digital level. Experiments were made using a Boltzmann policy, with epsilon set at 0.1, and a discount rate at 0.7. The learning rate was set at 0.1. The current implementation relies on tile coding, with a number of state tiles corresponding to the number of state directions—i.e., four.

Two additional elements were added to adapt the algorithm to the material dimension of the learning task. The first element is a time structure, which typically enables to reduce the randomness of each robot displacement by adding pauses between each robot action. Time structure is defined through a time step—defining the duration of each robot action, and a balance time—defining the duration of each robot pause, during which the robot gets zero speed and steer to gain a balance at its given position. In the experiments, time step and balance time were respectively set at two and one seconds. Future implementations may introduce small time variations in the time structure in case the resulting experience would be perceived as too "robotic" by the audience.

The second and last element is a virtual fence, which typically enables to prevent the robot from hitting the walls of its exhibition space. In the current implementation, the virtual fence has a rectangular shape, and is thus defined by boundary vertical and horizontal values. These boundaries were set to half a meter from the walls. When the robot crosses the virtual fence, it automatically takes a corrective action that has it go back within the exhibition space, by simply having its motor take an opposite speed value during six seconds. Future implementation may experiment with more diverse fence shapes—e.g., circular—depending on the design of the exhibition space, as well as with more complex corrective actions—e.g., taking into account the robot's orientation to choose which speed and steer values to take to get back within the exhibition space.