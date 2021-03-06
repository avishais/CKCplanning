/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2010, Rice University
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Rice University nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Avishai Sintov, Ioan Sucan */

#include "plan_PCS.h"

bool IKturn = true; // Define whether we sample q1 and solve IK for q2 (true) or vice versa.

bool isStateValid(const ob::State *state)
{
	return true;
}

ob::PlannerPtr plan_C::allocatePlanner(ob::SpaceInformationPtr si, plannerType p_type)
{
	switch (p_type)
	{
	case PLANNER_CBIRRT:
	{
		return std::make_shared<og::CBiRRT>(si, maxStep, env);
		break;
	}
	case PLANNER_RRT:
	{
		return std::make_shared<og::RRT>(si, maxStep, env);
		break;
	}
	case PLANNER_PRM:
	{
		return std::make_shared<og::PRM>(si, env);
		break;
	}
	case PLANNER_SBL:
	{
		return std::make_shared<og::SBL>(si, maxStep, env);
		break;
	}
	default:
	{
		OMPL_ERROR("Planner-type enum is not implemented in allocation function.");
		return ob::PlannerPtr(); // Address compiler warning re: no return value.
		break;
	}
	}
}

void plan_C::plan(State c_start, State c_goal, double runtime, plannerType ptype, double max_step) {

	// construct the state space we are planning inz
	ob::StateSpacePtr Q(new ob::RealVectorStateSpace(12)); // Angles of Robot 1 & 2 - R^12

	// set the bounds for the Q=R^12 part of 'Cspace'
	ob::RealVectorBounds Qbounds(12);
	Qbounds.setLow(0, -2.88); // Robot 1
	Qbounds.setHigh(0, 2.88);
	Qbounds.setLow(1, -1.919);
	Qbounds.setHigh(1, 1.919);
	Qbounds.setLow(2, -1.919);
	Qbounds.setHigh(2, 1.22);
	Qbounds.setLow(3, -2.79);
	Qbounds.setHigh(3, 2.79);
	Qbounds.setLow(4, -2.09);
	Qbounds.setHigh(4, 2.09);
	Qbounds.setLow(5, -PI_);// -6.98); // Should be -6.98 but currently the IK won't allow it - this impacts the sampler
	Qbounds.setHigh(5, PI_);// 6.98); // Should be 6.98 but currently the IK won't allow it
	Qbounds.setLow(6, -2.88); // Robot 2
	Qbounds.setHigh(6, 2.88);
	Qbounds.setLow(7, -1.919);
	Qbounds.setHigh(7, 1.919);
	Qbounds.setLow(8, -1.919);
	Qbounds.setHigh(8, 1.22);
	Qbounds.setLow(9, -2.79);
	Qbounds.setHigh(9, 2.79);
	Qbounds.setLow(10, -2.09);
	Qbounds.setHigh(10, 2.09);
	Qbounds.setLow(11, -PI_);// -6.98); // Should be -6.98 but currently the IK won't allow it
	Qbounds.setHigh(11, PI_);// 6.98); // Should be 6.98 but currently the IK won't allow it

	// set the bound for the compound space
	Q->as<ob::RealVectorStateSpace>()->setBounds(Qbounds);

	// construct a compound state space using the overloaded operator+
	ob::StateSpacePtr Cspace(Q);

	// construct an instance of  space information from this state space
	ob::SpaceInformationPtr si(new ob::SpaceInformation(Cspace));

	// set state validity checking for this space
	//si->setStateValidityChecker(ob::StateValidityCheckerPtr(new myStateValidityCheckerClass(si)));
	si->setStateValidityChecker(std::bind(&isStateValid, std::placeholders::_1));
	si->setStateValidityCheckingResolution(0.02); // 3% ???

	// create start state
	ob::ScopedState<ob::RealVectorStateSpace> start(Cspace);
	for (int i = 0; i < 12; i++) {
		start->as<ob::RealVectorStateSpace::StateType>()->values[i] = c_start[i]; // Access the first component of the start a-state
	}

	// create goal state
	ob::ScopedState<ob::RealVectorStateSpace> goal(Cspace);
	for (int i = 0; i < 12; i++) {
		goal->as<ob::RealVectorStateSpace::StateType>()->values[i] = c_goal[i]; // Access the first component of the goal a-state
	}

	// create a problem instance
	ob::ProblemDefinitionPtr pdef(new ob::ProblemDefinition(si));

	// set the start and goal states
	pdef->setStartAndGoalStates(start, goal);
	pdef->print();

	maxStep = max_step;
	// create a planner for the defined space
	// To add a planner, the #include library must be added above
	ob::PlannerPtr planner = allocatePlanner(si, ptype);

	// set the problem we are trying to solve for the planner
	planner->setProblemDefinition(pdef);

	// perform setup steps for the planner
	planner->setup();

	//planner->printSettings(std::cout); // Prints some parameters such as range
	//planner->printProperties(std::cout); // Prints some decisions such as multithreading, display approx solutions, and optimize?

	// print the settings for this space
	//si->printSettings(std::cout); // Prints state space settings such as check resolution, segmant count factor and bounds
	//si->printProperties(std::cout); // Prints state space properties, average length, dimension ...

	// print the problem settings
	//pdef->print(std::cout); // Prints problem definition such as start and goal states and optimization objective

	// attempt to solve the problem within one second of planning time
	clock_t begin = clock();
	ob::PlannerStatus solved = planner->solve(runtime);
	clock_t end = clock();
	cout << "Runtime: " << double(end - begin) / CLOCKS_PER_SEC << endl;

	if (solved) {
		// get the goal representation from the problem definition (not the same as the goal state)
		// and inquire about the found path
		//ob::PathPtr path = pdef->getSolutionPath();

		// print the path to screen
		//path->print(std::cout);  // Print as vectors

		// Save path to file
		//std::ofstream myfile;
		//myfile.open("pathPCS.txt");
		//og::PathGeometric& pog = static_cast<og::PathGeometric&>(*path); // Transform into geometric path class
		//pog.printAsMatrix(myfile); // Print as matrix to file
		//myfile.close();

		std::cout << "Found solution:" << std::endl;
		solved_bool = true;
	}
	else {
		std::cout << "No solutions found" << std::endl;
		solved_bool = false;
	}
}

int errormsg() {
	cout << "Error: incorrect inputs, use -h for options." << endl;
	return 1;
}

int main(int argn, char ** args) {
	std::cout << "OMPL version: " << OMPL_VERSION << std::endl;

	if (argn == 1 || argn == 3 || argn > 4)
		return errormsg();

	if (argn == 2) {
		if (!strcmp(args[1],"-h")) {
			std::cout << std::endl;
			std::cout << " ABB IRB-120 dual-arm planner using PCS" << std::endl;
			std::cout << " Syntex: " << std::endl;
			std::cout << "   ./pnr <runtime> <planner> <environment>" << std::endl;
			std::cout << "       <runtime>: maximum runtime for planning (sec)."  << std::endl;
			std::cout << "       <planner>: planner to use:" << std::endl;
			std::cout << "                   1 - Constrained Bi-directional RRT (CBi-RRT)" << std::endl;
			std::cout << "                   2 - Rapidly exploring Random Tree (RRT)" << std::endl;
			std::cout << "                   3 - Probabilistic RoadMap (PRM)" << std::endl;
			std::cout << "                   4 - Single-query Bi-directional Lazy collision checking planner (SBL)" << std::endl;
			std::cout << "       <environment>: 1 - env. I with three poles, 1 - env. II with two cones (narrow passage)." << std::endl;
			std::cout << "       default: CBi-RRT, 1 sec, env. I."  << std::endl << std::endl;
			return 0;
		}
		else
			return errormsg();
	}

	double runtime; // Maximum allowed runtime
	plannerType ptype; // Planner type
	string plannerName;
	int env; // Tested environment index
	State step(2); // Optimal maximum step size

	runtime = atof(args[1]);
	switch (atoi(args[2])) {
	case 1 :
		ptype = PLANNER_CBIRRT;
		plannerName = "CBiRRT";
		step = {2.6, 1};
		break;
	case 2 :
		ptype = PLANNER_RRT;
		plannerName = "RRT";
		step = {2.6, 0.8};
		break;
	case 3 :
		ptype = PLANNER_PRM;
		plannerName = "PRM";
		step = {0, 0};
		break;
	case 4 :
		ptype = PLANNER_SBL;
		plannerName = "SBL";
		step = {0.8, 0.6};
		break;
	default :
		return errormsg();
	}
	env = atoi(args[3]);
	if (env != 1 && env != 2)
		return errormsg();

	plan_C Plan;

	srand (time(NULL));

	State c_start, c_goal;
	if (env == 1) {
		c_start = {0.5236, 1.7453, -1.8326, -1.4835,	1.5708,	0, 1.004278, 0.2729, 0.9486, -1.15011, 1.81001, -1.97739};
		c_goal = {0.5236, 0.34907, 0.69813, -1.3963, 1.5708, 0, 0.7096, 1.8032, -1.7061, -1.6286, 1.9143, -2.0155}; // Robot 2 no backflip - Elbow down
		Plan.set_environment(1);
	}
	else if (env == 2) {
		c_start = {1.1, 1.1, 0, 1.24, -1.5708, 0, -0.79567, 0.60136, 0.43858, -0.74986, -1.0074, -0.092294};
		c_goal = {-1.1, 1.35, -0.2, -1, -1.9, 0, 0.80875, 0.72363, -0.47891, -1.0484, 0.73278, 1.7491};
		Plan.set_environment(2);
	}

	Plan.plan(c_start, c_goal, runtime, ptype, step[env-1]);

	std::cout << std::endl << std::endl;

	return 0;
}

