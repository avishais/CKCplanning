/*
 * Checker.h
 *
 *  Created on: Oct 28, 2016
 *      Author: avishai
 */

#ifndef CHECKER_H_
#define CHECKER_H_

#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/base/StateValidityChecker.h>
#include "ompl/base/MotionValidator.h"
#include "ompl/base/State.h"
#include <ompl/config.h>

#include "kdl_class.h"
#include "collisionDetection.h"

#include <iostream>

#define ROBOTS_DISTANCE_ENV_I 900
#define ROD_LENGTH_ENV_I 300
#define ROBOTS_DISTANCE_ENV_II 1200
#define ROD_LENGTH_ENV_II 500

namespace ob = ompl::base;
using namespace std;

class StateValidityChecker : public collisionDetection, public kdl
{
public:
	/** Constructors */
	StateValidityChecker(const ob::SpaceInformationPtr &si, int env = 1) :
		mysi_(si.get()),
		kdl(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II, env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II),
		collisionDetection(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II,0,0,0,env)
			{L = env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II;
			setQ();
			setP();
			}; //Constructor
	StateValidityChecker(int env = 1) :
		kdl(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II, env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II),
		collisionDetection(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II,0,0,0,env)
			{L = env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II;
			setQ();
			setP();
			}; //Constructor

	/** Validity check for a vector<double> type  */
	bool isValidRBS(State&);

	/** Recursive Bi-Section local connection check  */
	bool checkMotionRBS(const ob::State *, const ob::State *);
	bool checkMotionRBS(State, State, int, int);

	/** Reconstruct a local connection using RBS for post-processing  */
	bool reconstructRBS(const ob::State *, const ob::State *, Matrix &);
	bool reconstructRBS(State, State, Matrix &, int, int, int);

	/** Retrieve state from ob::State to vector<double> */
	void retrieveStateVector(const ob::State *, State &);

	/** Update state to ob::State from vector<double> */
	void updateStateVector(const ob::State *, State);

	/** Print ob::State ro console */
	void printStateVector(const ob::State *state);

	/** Set default OMPL setting */
	void defaultSettings();

	/** Calculate norm distance between two vectors */
	double normDistance(State, State);
	double stateDistance(const ob::State*, const ob::State*);

	/** Project a configuration in the ambient space to the constraint surface (and check collisions and joint limits) */
	bool IKproject(const ob::State *, bool = true);
	bool IKproject(State &, bool = true);

	/** Sample a random configuration */
	State sample_q();

	/** Join the two robots joint vectors */
	void Join_States(State&, State, State);

	/** Decouple the two robots joint vectors */
	void seperate_Vector(State, State &, State &);

	/** Return matrix of coordinated along the rod (in rod coordinate frame) */
	Matrix getPMatrix() {
		return P;
	}

	/** Return transformation matrix of rod end-tip in rod coordinate frame (at the other end-point) */
	Matrix getQ() {
		return Q;
	}

	/** Set transformation matrix of rod end-tip in rod coordinate frame (at the other end-point) */
	void setQ() {
		State v(4);

		v = {1,0,0,L};
		Q.push_back(v);
		v = {0,1,0,0};
		Q.push_back(v);
		v = {0,0,1,0};
		Q.push_back(v);
		v = {0,0,0,1};
		Q.push_back(v);
	}

	/** Set matrix of coordinated along the rod (in rod coordinate frame) */
	void setP() {
		State v(3);
		int dl = 20;
		int n = L / dl;
		for (int i = 0; i <= n; i++) {
			v = {(double)i*dl,0,0};
			P.push_back(v);
		}
	}

	/** Performance parameters measured during the planning */
	int isValid_counter;
	int get_isValid_counter() {
		return isValid_counter;
	}

	double iden = 0;
	double get_iden() {
		return iden;
	}

	int get_n() {
		return n;
	}
	double get_RBS_tol() {
		return RBS_tol;
	}

	void log_q(State q, bool New);

	// Performance parameters and handle
	double total_runtime; // Total planning time
	clock_t startTime; // Start clock
	clock_t endTime; // End clock
	int nodes_in_path; // Log nodes in path
	int nodes_in_trees; // Log nodes in both trees
	double PlanDistance; // Norm distance from start to goal configurations
	bool final_solved; // Planning query solved?
	double local_connection_time; // Log LC total time
	int local_connection_count; // Log number of LC attempts
    int local_connection_success_count; // Log number of LC success
    double sampling_time;
    State sampling_counter;

	/** Reset log parameters */
	void initiate_log_parameters() {
		IK_counter = 0;
		IK_time = 0;
		collisionCheck_counter = 0;
		collisionCheck_time = 0;
		isValid_counter = 0;
		nodes_in_path = 0;
		nodes_in_trees = 0;
		local_connection_time = 0;
		local_connection_count = 0;
    	local_connection_success_count = 0;
    	sampling_time = 0;
    	sampling_counter.resize(2);
    	sampling_counter[0] = sampling_counter[1] = 0; // [0/1] - successful/failed sampling
	}

	void LogPerf2file();

private:
	ob::StateSpace *stateSpace_;
	ob::SpaceInformation    *mysi_;

	double L;
	Matrix Q;
	Matrix P;

	double dq = 0.05; // Serial local connection resolution
	bool withObs = true; // Include obstacles?
	double RBS_tol = 0.05; // RBS local connection resolution
	int RBS_max_depth = 150; // Maximum RBS recursion depth
	int n = 12; // Dimension of system
};





#endif /* CHECKER_H_ */
