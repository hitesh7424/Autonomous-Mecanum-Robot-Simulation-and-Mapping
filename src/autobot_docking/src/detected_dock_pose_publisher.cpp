/**
 * @file detected_dock_pose_publisher.cpp
 * @brief Publishes the pose of an AprilTag located on/near a docking station for the Autonomous Mecanum Robot (Autobot)
 *
 * This program detects an AprilTag that is mounted near or on a docking station and publishes
 * its pose relative to the camera optical frame. The optical frame follows the typical computer
 * vision convention where:
 *   - Z forward (pointing out from the camera)
 *   - X right
 *   - Y down
 *
 * The node subscribes to TF2 transforms published by the AprilTag detection system between the
 * camera's optical frame and the detected tag's frame. It then republishes these transforms as
 * PoseStamped messages that can be used by the Nav2 docking system to compute the actual
 * docking pose.
 *
 * Subscription Topics:
 *     /tf (tf2_msgs/TFMessage): Transform tree containing camera optical frame to tag transforms
 *
 * Publishing Topics:
 *     /detected_dock_pose (geometry_msgs/PoseStamped): Pose of the detected AprilTag relative
 *                                                      to the camera optical frame
 *
 * Parameters:
 *     parent_frame (string, default: "rgbd_camera_depth_optical_frame"): Name of the camera's optical frame
 *     child_frame (string, default: "tag36h11:0"): Name of the AprilTag frame
 *     publish_rate (double, default: 10.0): How often to publish the tag pose in Hz
 */

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "action_msgs/msg/goal_status_array.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"

/**
 * @brief A ROS2 node that publishes AprilTag poses relative to the camera optical frame
 *
 * This node listens for transforms between the camera's optical frame (rgbd_camera_depth_optical_frame)
 * and an AprilTag frame (tag36h11:0). The optical frame is important as it follows standard
 * computer vision conventions and is the frame in which the AprilTag detector operates.
 *
 * The AprilTag is mounted near or on a docking station, and this node publishes the tag's pose
 * in the optical frame coordinate system. The Nav2 docking system can then use this tag pose
 * as a reference to compute the actual docking position.
 */
class DetectedDockPosePublisher : public rclcpp::Node
{
public:
  /**
   * @brief Constructor for the DetectedDockPosePublisher node
   *
   * Initializes the node, sets up parameters, and creates publishers and transform listeners
   */
  DetectedDockPosePublisher()
  : Node("detected_dock_pose_publisher")
  {
    // Declare parameters with default values and documentation
    this->declare_parameter("parent_frame", "rgbd_camera_depth_optical_frame");
    this->declare_parameter("child_frame", "tag36h11:0");
    this->declare_parameter("publish_rate", 10.0);  // Hz

    // Get the values of our parameters
    parent_frame_ = this->get_parameter("parent_frame").as_string();
    child_frame_ = this->get_parameter("child_frame").as_string();
    double publish_rate = this->get_parameter("publish_rate").as_double();

    // Create a transform buffer to store and look up transforms
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());

    // Create a transform listener to receive transforms
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Create a publisher for the dock pose
    dock_pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>(
      "detected_dock_pose", 10);

    // Create a publisher for cmd_vel to rotate and search for AprilTag
    cmd_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

    // Create a subscriber to dock_robot action status
    action_status_sub_ = this->create_subscription<action_msgs::msg::GoalStatusArray>(
      "dock_robot/_action/status",
      10,
      std::bind(&DetectedDockPosePublisher::status_callback, this, std::placeholders::_1));

    // Create a timer that will trigger pose updates
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(static_cast<int>(1000.0 / publish_rate)),
      std::bind(&DetectedDockPosePublisher::timer_callback, this));

    // Log that we've successfully initialized
    RCLCPP_INFO(this->get_logger(),
      "Detected dock pose publisher initialized with parent frame: '%s' and child frame: '%s'",
      parent_frame_.c_str(), child_frame_.c_str());
  }

private:
  /**
   * @brief Timer callback that publishes the latest dock pose
   *
   * This function is called periodically to:
   * 1. Look up the latest transform between the camera and AprilTag
   * 2. Convert the transform into a pose message
   * 3. Publish the pose for the docking system to use
   */
  void status_callback(const action_msgs::msg::GoalStatusArray::SharedPtr msg)
  {
    bool active = false;
    for (const auto & status : msg->status_list) {
      if (status.status == 1 || status.status == 2) { // GoalStatus::STATUS_ACCEPTED or STATUS_EXECUTING
        active = true;
        break;
      }
    }
    
    if (active && !docking_active_) {
      RCLCPP_INFO(this->get_logger(), "Docking action active. Preparing to search for AprilTag if not in view.");
      search_start_time_ = this->get_clock()->now();
      search_state_ = 1; // Start searching state
    } else if (!active && docking_active_) {
      RCLCPP_INFO(this->get_logger(), "Docking action inactive. Stopping any search rotation.");
      search_state_ = 0; // Reset to idle
      publish_zero_velocity();
    }
    docking_active_ = active;
  }

  void publish_zero_velocity()
  {
    geometry_msgs::msg::Twist twist;
    twist.linear.x = 0.0;
    twist.linear.y = 0.0;
    twist.linear.z = 0.0;
    twist.angular.x = 0.0;
    twist.angular.y = 0.0;
    twist.angular.z = 0.0;
    cmd_vel_pub_->publish(twist);
  }

  void timer_callback()
  {
    // Create a new pose message
    geometry_msgs::msg::PoseStamped dock_pose;
    // Set the timestamp to now
    dock_pose.header.stamp = this->get_clock()->now();
    // The frame ID should match the frame we want the pose expressed in
    dock_pose.header.frame_id = parent_frame_;

    bool detected = false;
    try {
      // Look up the transform
      geometry_msgs::msg::TransformStamped transform = tf_buffer_->lookupTransform(
        parent_frame_,
        child_frame_,
        tf2::TimePointZero // get latest transform
      );

      // Copy the translation from the transform to the pose
      dock_pose.pose.position.x = transform.transform.translation.x;
      dock_pose.pose.position.y = transform.transform.translation.y;
      dock_pose.pose.position.z = transform.transform.translation.z;

      // Copy the rotation from the transform to the pose
      dock_pose.pose.orientation = transform.transform.rotation;

      // Publish the dock pose for the navigation system to use
      dock_pose_pub_->publish(dock_pose);
      detected = true;
      last_detection_time_ = this->get_clock()->now();

      if (search_state_ == 1) {
        RCLCPP_INFO(this->get_logger(), "AprilTag detected! Stopping search rotation.");
        publish_zero_velocity();
        search_state_ = 0; // reset to idle
      }
    }
    catch (const tf2::TransformException & ex) {
      // If we can't get the transform, log it at debug level to avoid spamming
      RCLCPP_DEBUG(this->get_logger(), "Could not get transform: %s", ex.what());
    }

    if (!detected) {
      // Manage search rotation if docking is active
      if (docking_active_ && search_state_ == 1) {
        rclcpp::Time now = this->get_clock()->now();
        double time_since_last_detect = 0.0;
        if (last_detection_time_.nanoseconds() > 0) {
          time_since_last_detect = (now - last_detection_time_).seconds();
        } else {
          time_since_last_detect = (now - search_start_time_).seconds();
        }

        if (time_since_last_detect > 1.0) {
          double elapsed_search = (now - search_start_time_).seconds();
          if (elapsed_search > 20.0) { // Rotate up to 20 seconds
            RCLCPP_WARN(this->get_logger(), "AprilTag search timeout exceeded (20s). Stopping search rotation.");
            publish_zero_velocity();
            search_state_ = 2; // Timeout state
          } else {
            // Change rotation direction every 4 seconds:
            // 0s-4s: Left (yaw_vel = 0.3)
            // 4s-8s: Right (yaw_vel = -0.3)
            // 8s-12s: Left (yaw_vel = 0.3)
            // etc.
            int step = static_cast<int>(elapsed_search) / 4;
            int current_dir = (step % 2 == 0) ? 1 : -1;
            if (current_dir != search_direction_) {
              search_direction_ = current_dir;
              RCLCPP_INFO(this->get_logger(), "Searching for AprilTag... changing spin direction to: %s", 
                          search_direction_ > 0 ? "LEFT (counter-clockwise)" : "RIGHT (clockwise)");
            }

            geometry_msgs::msg::Twist twist;
            twist.angular.z = 0.3 * search_direction_;
            cmd_vel_pub_->publish(twist);
          }
        }
      }
    }
  }

  // Frame names from parameters
  std::string parent_frame_; ///< Name of the camera frame
  std::string child_frame_;  ///< Name of the AprilTag frame

  // Subscriptions and Publishers for tag search
  rclcpp::Subscription<action_msgs::msg::GoalStatusArray>::SharedPtr action_status_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;

  // Search state variables
  bool docking_active_{false};
  int search_state_{0}; // 0 = idle, 1 = searching, 2 = timeout
  int search_direction_{1}; // 1 = left, -1 = right
  rclcpp::Time last_detection_time_{0, 0, RCL_ROS_TIME};
  rclcpp::Time search_start_time_{0, 0, RCL_ROS_TIME};

  // ROS infrastructure
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;        ///< Buffer for storing transforms
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_; ///< Listener for transforms
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr dock_pose_pub_; ///< Publisher for dock poses
  rclcpp::TimerBase::SharedPtr timer_;               ///< Timer for periodic publishing
};

/**
 * @brief Main function that starts the dock pose publisher node
 *
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return int Exit code (0 if successful)
 */
int main(int argc, char * argv[])
{
  // Initialize ROS
  rclcpp::init(argc, argv);
  // Create and spin (run) the node
  rclcpp::spin(std::make_shared<DetectedDockPosePublisher>());
  // Clean up ROS and exit
  rclcpp::shutdown();
  return 0;
}