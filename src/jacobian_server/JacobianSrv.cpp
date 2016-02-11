#include "JacobianSrv.hpp"

JacobianSrv::JacobianSrv(string param_description){
	// initialize robot model
	robot_model_loader::RobotModelLoader robot_model_loader("robot_description");
	robot_model::RobotModelPtr kinematic_model = robot_model_loader.getModel();
	// initialize the robot state
	this->kinematic_state = robot_state::RobotStatePtr(new robot_state::RobotState(kinematic_model));
	this->kinematic_state->setToDefaultValues();
	// fill the map of groups
	this->groups["head"] = kinematic_model->getJointModelGroup("head");
	this->groups["right_arm"] = kinematic_model->getJointModelGroup("right_arm");
	this->groups["left_arm"] = kinematic_model->getJointModelGroup("left_arm");
}

JacobianSrv::~JacobianSrv(){}

bool JacobianSrv::computeJacobian(human_moveit_config::GetJacobian::Request  &req,
		                 		  human_moveit_config::GetJacobian::Response &res){
	ROS_INFO("Received new request");
	// get the corrrect group
	const robot_state::JointModelGroup* joint_model_group = this->groups[req.group_name];
	// get the joint values
	std::vector<double> joint_values = req.joint_state.position;
	// set the joints values for the group
	this->kinematic_state->setJointGroupPositions(joint_model_group, joint_values);
	// get the reference point
	Eigen::Vector3d reference_point_position;
	reference_point_position[0] = req.reference_point.x;
	reference_point_position[1] = req.reference_point.y;
	reference_point_position[2] = req.reference_point.z;
	// compute the jacobian
	Eigen::MatrixXd jacobian;
	kinematic_state->getJacobian(joint_model_group, 
		                         this->kinematic_state->getLinkModel(req.link_name),
                                 reference_point_position,
                                 jacobian);
	// convert the jacobian to a 1-D array
	int nb_cols = jacobian.cols();
	std::vector<double> jacobian_vect(jacobian.rows()*nb_cols, 0);
	// fill the jacobian with the values
	for (size_t i = 0, nRows = jacobian.rows(), nCols = jacobian.cols(); i < nCols; ++i)
   		for (size_t j = 0; j < nRows; ++j){
   			jacobian_vect[j*nCols + i] = jacobian(j,i);
   		}
   	// send back the result
   	res.nb_joints = nb_cols;
   	res.jacobian = jacobian_vect;

   	ROS_INFO("Jacobian computed and sent");
	return true;
}
