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

#include "apc_class.h"
#include "collisionDetection.h"

#include <iostream>

#define ROBOTS_DISTANCE_ENV_I 900.
#define ROD_LENGTH_ENV_I 300.
#define ROBOTS_DISTANCE_ENV_II 1200.
#define ROD_LENGTH_ENV_II 500.

namespace ob = ompl::base;
using namespace std;

class StateValidityChecker : public two_robots, public collisionDetection
{
public:
	/** Constructors */
	StateValidityChecker(const ob::SpaceInformationPtr &si, int env = 1) :
		mysi_(si.get()),
		two_robots({-(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II)/2, 0, 0 }, {(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II)/2, 0, PI_}, env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II),
		collisionDetection(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II,0,0,0,env)
			{L = env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II;
			q_temp.resize(6);
			setQ();
			setP();
			}; //Constructor
	StateValidityChecker(int env = 1) :
		two_robots({-(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II)/2, 0, 0 }, {(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II)/2, 0, PI_}, env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II),
		collisionDetection(env==1 ? ROBOTS_DISTANCE_ENV_I : ROBOTS_DISTANCE_ENV_II,0,0,0,env)
			{L = env==1 ? ROD_LENGTH_ENV_I : ROD_LENGTH_ENV_II;
			q_temp.resize(6);
			setQ();
			setP();
			}; //Constructor

	/** Validity check for a vector<double> type  */
	bool isValidRBS(State&, State&, int, int);

	/** Recursive Bi-Section local connection check  */
	bool checkMotionRBS(const ob::State *, const ob::State *, int, int);
	bool checkMotionRBS(State, State, State, State, int, int, int, int);

	/** Reconstruct a local connection using RBS for post-processing  */
	bool reconstructRBS(const ob::State *, const ob::State *, Matrix &, int, int);
	bool reconstructRBS(State, State, State, State, int, int, Matrix &, int, int, int);

	/** Norm distance of 2 vectors while each is separated */
	double normDistanceDuo(State, State, State, State);

	/** Return mid-point of two vectors for the RBS */
	void midpoint(State, State, State, State, State &, State&);

	/** Retrieve state from ob::State to vector<double> */
	void retrieveStateVector(const ob::State *, State &, State &);

	/** Update state to ob::State from vector<double> */
	void updateStateVector(const ob::State *, State, State);

	/** Print ob::State to console */
	void printStateVector(const ob::State *);

	/** Set default OMPL setting */
	void defaultSettings();

	/** Calculate norm distance between two vectors */
	double normDistance(State, State);

	/** Calculate norm distance between two ob::State's */
	double stateDistance(const ob::State *, const ob::State *);

	/** Close chain (project) */
	bool close_chain(const ob::State *, int);

	/** Sample a random configuration */
	State sample_q();
	bool sample_q(ob::State *st);

	/** Project a configuration in the ambient space to the constraint surface (and check collisions and joint limits) */
	bool IKproject(State &, State &, int, int);
	bool IKproject(State &, State &, int);
	bool IKproject(State &, State &, State &, int&, State);

	/** Identify the IK solutions of a configuration using two passive chains */
	State identify_state_ik(const ob::State *, State = {-1, -1});
	State identify_state_ik(State, State, State = {-1, -1});

	/** Join the two robots joint vectors */
	State join_Vectors(State, State);

	/** Decouple the two robots joint vectors */
	void seperate_Vector(State, State &, State &);

	/** Log configuration to path file */
	void log_q(ob::State *);
	void log_q(State, State);

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

	double get_RBS_tol(){
		return RBS_tol;
	}

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
	State q_temp;

	double L;
	Matrix Q;
	Matrix P;

	bool withObs = true; // Include obstacles?
	double RBS_tol = 0.05; // RBS local connection resolution
	int RBS_max_depth = 150; // Maximum RBS recursion depth

};

#endif /* CHECKER_H_ */
