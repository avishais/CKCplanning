/*
 * plan_C_space.h
 *
 *  Created on: Nov 10, 2016
 *      Author: avishai
 */

#ifndef PLAN_C_SPACE_H_
#define PLAN_C_SPACE_H_

// OMPL
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/SimpleSetup.h>
#include <ompl/config.h>

// Modified and custom planners
#include "../planners/CBiRRT_PCS.h"
#include "../planners/RRT_PCS.h"
#include "../planners/PRM_PCS.h"
#include "../planners/SBL_PCS.h"

// Standard libraries
#include <iostream>
#include <fstream>

namespace ob = ompl::base;
namespace og = ompl::geometric;
using namespace std;

// An enum of available planners
enum plannerType
{
	PLANNER_CBIRRT,
	PLANNER_RRT,
	PLANNER_PRM,
	PLANNER_SBL
};

bool isStateValid(const ob::State *state);

// Prototypes
class plan_C
{
public:

	void plan(State, State, double, plannerType = PLANNER_CBIRRT, double = 2);

	// Construct the planner specified by our command line argument.
	// This helper function is simply a switch statement.
	ob::PlannerPtr allocatePlanner(ob::SpaceInformationPtr, plannerType);

	bool solved_bool;
	double total_runtime;

	double maxStep;

	int env;
	void set_environment(int env_index) {
		env = env_index;
	}
};

#endif /* PLAN_C_SPACE_H_ */
