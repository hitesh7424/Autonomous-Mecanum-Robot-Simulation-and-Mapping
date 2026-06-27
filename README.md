# Autonomous-Mecanum-Robot-Simulation-and-Mapping

ROOKIES CONTESTS : Hackathon Challenge : Autonomous Mecanum Robot Simulation and Mapping

An autonomous mecanum-wheeled robot (Autobot) simulation workspace built with **ROS 2 (Jazzy)** and **Gazebo**. This project supports sensor fusion, SLAM mapping, autonomous navigation using Nav2, and AprilTag-based docking.

---

## 🚀 Features

- **Mecanum Drive Control**: Customized mecanum drive controller simulation.
- **Sensor Fusion / Localization**: Extended Kalman Filter (EKF) combining wheel odometry and IMU data.
- **SLAM / Mapping**: Cartographer/Nav2 Toolbox mapping integration.
- **Autonomous Navigation**: Waypoint-following and goal-pose navigation via Nav2.
- **Precision Docking**: AprilTag detection and autonomous docking capabilities.

---

## 🛠️ Prerequisites & Installation

### 1. System Requirements
- **OS**: Ubuntu 24.04 LTS (recommended)
- **ROS 2**: Jazzy Jalisco
- **Simulator**: Gazebo (Gz Sim)

### 2. Install Dependencies
Ensure your package list is up-to-date and install the required ROS 2 control and simulation dependencies:

```bash
sudo apt update
rosdep update
rosdep install --from-paths src --ignore-src -y
sudo apt install ros-jazzy-gz-ros2-control
```

---

## 📦 Build Instructions

Build the workspace using `colcon`. If you are overriding the default mecanum controller package, use the following command:

```bash
colcon build --symlink-install --allow-overriding mecanum_drive_controller
source install/setup.bash
```

---

## ⏱️ Quick Start & Usage

Follow these steps sequentially to run mapping or navigation.

### 1. Launch Simulation & Navigation (with SLAM)
To launch the Gazebo simulation environment, EKF localization, Nav2, and RViz visualization in **SLAM mode** to create a map:

```bash
ros2 launch autobot_bringup autobot_navigation.launch.py slam:=True
```

### 2. Teleoperation (Manual Control)
In a new terminal (with workspace sourced), run the teleop node to drive the robot around and build your map:

```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

### 3. Save the Map
Once the map is completed, save it using the `map_saver_cli`:

```bash
ros2 run nav2_map_server map_saver_cli -f cafe_world_map
```

### 4. Autonomous Navigation (with Pre-saved Map)
To launch autonomous navigation using your pre-saved map:

```bash
ros2 launch autobot_bringup autobot_navigation.launch.py slam:=False map:=$(pwd)/src/autobot_navigation/maps/cafe_world_map.yaml
```
*Send goals using the **2D Goal Pose** tool in RViz, or by publishing directly to the `/goal_pose` topic.*

### 5. Advanced Commands & Test Utilities
- **Manually Publish a Navigation Goal Pose**:
  ```bash
  ros2 topic pub /goal_pose/goal geometry_msgs/PoseStamped "{header: {stamp: {sec: 0, nanosec: 0}, frame_id: 'map'}, pose: {position: {x: 2.0, y: 4.0, z: 0.0}, orientation: {x: 0.0, y: 0.0, z: 0.707, w: 0.707}}}" --once
  ```
- **Stop/Cancel Navigation**:
  ```bash
  ros2 topic pub /stop/navigation/go_to_goal_pose std_msgs/msg/Bool "data: true"
  ```
- **Test Navigation to Pose Script**:
  ```bash
  ros2 run autobot_navigation test_nav_to_pose.py --ros-args -p use_sim_time:=true
  ```
- **View Camera Feed**:
  ```bash
  ros2 run rqt_image_view rqt_image_view /rgbd_camera/color/image_raw
  ```

---

## 📂 Repository Structure

* **`autobot_bringup`**: Contains top-level launch files coordinating localization, simulation, and navigation.
* **`autobot_description`**: URDF, XACRO configurations, meshes, and RViz visualization setups.
* **`autobot_gazebo`**: Gazebo world files, launch files, and plugins configuration.
* **`autobot_localization`**: Extended Kalman Filter (EKF) configuration and launch files.
* **`autobot_navigation`**: Nav2 costmap, behavior tree, and path planner parameter files.
* **`autobot_docking`**: AprilTag detection and autonomous docking nodes.
* **`autobot_msgs`**: Custom ROS 2 message and service definitions.
* **`mecanum_drive_controller`**: Customized controller for mecanum kinematics.