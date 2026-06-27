# 🖥️ Simulation Demonstration Video Flow


---

## 🛠️ Step 1: SLAM Mapping & Teleop (Start of Demo)

### 1. Launch the simulation in SLAM mode (Terminal 1):
```bash
ros2 launch autobot_bringup autobot_navigation.launch.py slam:=True
```

### 2. Open the keyboard teleop to drive around (Terminal 2):
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard --ros-args -r /cmd_vel:=/cmd_vel_teleop
```
*Note: Drive the robot in Gazebo and record RViz showing the grid map growing.*

### 3. Save the map once completed (Terminal 3):
```bash
ros2 run nav2_map_server map_saver_cli -f cafe_world_map
```

---

## 🧭 Step 2: Autonomous Navigation (Middle of Demo)

### 1. Close the previous terminals, then run navigation using the saved map (Terminal 1):
```bash
ros2 launch autobot_bringup autobot_navigation.launch.py slam:=False map:=$(pwd)/src/autobot_navigation/maps/cafe_world_map.yaml
```

### 2. (Optional) Run the Table Goal Publisher script to easily dispatch goals (Terminal 2):
```bash
ros2 run autobot_navigation test_nav_to_pose.py --ros-args -p use_sim_time:=true
```
*Note: You can also use the **2D Goal Pose** tool in RViz to set a destination. Record the robot planning a path and navigating autonomously to the goal while avoiding obstacles.*

---

## 🔌 Step 3: Precision Docking (End of Demo)

### 1. Trigger the docking action server (Terminal 3):
```bash
ros2 action send_goal /dock_robot opennav_docking_msgs/action/DockRobot "{dock_id: 'dock0', use_dock_id: true}"
```
*Note: Record the robot searching for the AprilTag (spinning to locate it) and docking smoothly at the charging station once the camera detects the tag.*
