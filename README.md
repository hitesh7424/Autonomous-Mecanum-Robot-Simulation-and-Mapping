# Autonomous-Mecanum-Robot-Simulation-and-Mapping

ROOKIES CONTESTS : Hackathon Challenge : Autonomous Mecanum Robot Simulation and Mapping

## commands used to create packages

```bash
ros2 pkg create --build-type ament_cmake --license MIT autobot
ros2 pkg create --build-type ament_cmake --license MIT autobot_bringup
ros2 pkg create --build-type ament_cmake --license MIT autobot_description
ros2 pkg create --build-type ament_cmake --license MIT autobot_gazebo
ros2 pkg create --build-type ament_cmake --license MIT mecanum_drive_controller
ros2 pkg create --build-type ament_cmake --license MIT autobot_docking
ros2 pkg create --build-type ament_cmake --license MIT autobot_localization
ros2 pkg create --build-type ament_cmake --license MIT autobot_msgs
ros2 pkg create --build-type ament_cmake --license MIT autobot_navigation
```

## command to update the ros dependency

```bash
rosdep update
rosdep install --from-paths src --ignore-src -y
```

## colcon build

```bash
colcon build --symlink-install --allow-overriding mecanum_drive_controller
```

## launch commands

```bash
ros2 launch autobot_gazebo autobot.gazebo.launch.py
```

```bash
ros2 launch autobot_description robot_state_publisher.launch.py
```

ros2 launch nav2_bringup tb3_simulation_launch.py headless:=False

ros2 launch nav2_bringup tb4_simulation_launch.py slam:=True headless:=False


ros2 launch autobot_gazebo autobot.gazebo.launch.py \
    enable_odom_tf:=true \
    headless:=False \
    load_controllers:=true \
    world_file:=cafe.world \
    use_rviz:=true \
    use_robot_state_pub:=true \
    use_sim_time:=true \
    x:=0.0 \
    y:=0.0 \
    z:=0.20 \
    roll:=0.0 \
    pitch:=0.0 \
    yaw:=0.0

gz service -s /gui/move_to/pose --reqtype gz.msgs.GUICamera --reptype gz.msgs.Boolean --timeout 2000 --req "pose: {position: {x: 0.0, y: -2.0, z: 2.0} orientation: {x: -0.2706, y: 0.2706, z: 0.6533, w: 0.6533}}"

ros2 launch autobot_bringup autobot_navigation.launch.py



ros2 launch autobot_bringup autobot_navigation.launch.py \
    enable_odom_tf:=false \
    headless:=False \
    load_controllers:=true \
    world_file:=cafe.world \
    use_rviz:=true \
    use_robot_state_pub:=true \
    use_sim_time:=true \
    x:=0.0 \
    y:=0.0 \
    z:=0.20 \
    roll:=0.0 \
    pitch:=0.0 \
    yaw:=0.0 \
    "$SLAM_ARG" \
    map:=/home/hitesh/Autonomous-Mecanum-Robot-Simulation-and-Mapping/src/autobot_navigation/maps/cafe_world_map.yaml


ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args   -p stamped:=true   -r cmd_vel:=/mecanum_drive_controller/cmd_vel

ros2 run nav2_map_server map_saver_cli -f cafe_world_map

ros2 topic echo /clicked_point

ros2 run autobot_navigation test_nav_to_pose.py --ros-args -p use_sim_time:=true

ros2 launch autobot_bringup autobot_navigation.launch.py slam:=False map:=/home/hitesh/Autonomous-Mecanum-Robot-Simulation-and-Mapping/src/autobot_navigation/maps/cafe_world_map.yaml

ros2 topic pub /goal_pose/goal geometry_msgs/PoseStamped "{header: {stamp: {sec: 0, nanosec: 0}, frame_id: 'map'}, pose: {position: {x: 2.0, y: 4.0, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.707, w: 0.707}}}" --once

ros2 topic pub /stop/navigation/go_to_goal_pose std_msgs/msg/Bool "data: true"

ros2 topic pub /cmd_vel_teleop geometry_msgs/msg/Twist "{linear: {x: 0.2, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}" --rate 10

ros2 topic pub /cancel_assisted_teleop std_msgs/Bool "data: true" --once