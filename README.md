# CKCplanning
Comparative analysis for Sampling-based motion planning of closed kinematic chains
Author: Avishai Sintov, http://avishaisintov.wordpress.com

## Installation
1. Install OMPL: http://ompl.kavrakilab.org/download.html
2. Install PQP: http://gamma.cs.unc.edu/SSV
3. Install KDL: http://www.orocos.org/wiki/orocos/kdl-wiki
4. Also requires: armadilo, openGL (for the simulation).
5. Change required dependency paths in makefile.

To run a planner:
1) make
2) ./pXXX -h (for options)

XXX - nr: Newton-Raphson projection.
           pcs: Passive chains swap.
           rlx: Relaxation of the closed kinematic chain constraint.
           rss: Sampling of singular configurations.
      
To run simulation:
1) cd simulator
2) ./viz -h (for options)
