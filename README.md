# Autonomous-Mecanum-Robot-Simulation-and-Mapping

ROOKIES CONTESTS : Hackathon Challenge : Autonomous Mecanum Robot Simulation and Mapping



## commands used to create packages

ros2 pkg create --build-type ament_cmake --license MIT autobot
ros2 pkg create --build-type ament_cmake --license MIT autobot_bringup
ros2 pkg create --build-type ament_cmake --license MIT autobot_description
ros2 pkg create --build-type ament_cmake --license MIT autobot_gazebo


## command to update the ros dependency

rosdep install --from-paths src --ignore-src -y
