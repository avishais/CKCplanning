#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "PQP.h"
#include "model.h"
#include "MatVec.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <string.h>

#define ROBOTS_DISTANCE 900
#define ROBOTS_DISTANCE_ENVII 1200
#define ROD_LENGTH 300
#define ROD_LENGTH_ENVII 500

PQP_Model base, link1, link2, link3, link4, link5, link6, EE, table, room, floor1;
Model *base_to_draw, *link1_to_draw, *link2_to_draw, *link3_to_draw, \
*link4_to_draw, *link5_to_draw, *link6_to_draw, *EE_to_draw;

PQP_Model base2, link12, link22, link32, link42, link52, link62, EE2, rod, obs1, obs2, obs3;
Model *base_to_draw2, *link1_to_draw2, *link2_to_draw2, *link3_to_draw2, \
*link4_to_draw2, *link5_to_draw2, *link6_to_draw2, *EE_to_draw2, \
*rod_to_draw, *table_to_draw, *obs1_to_draw, *obs2_to_draw, *obs3_to_draw, \
*room_to_draw, *floor_to_draw;

PQP_REAL R0[3][3],R1[3][3],R2[3][3],T0[3],T1[3],T2[3];
PQP_REAL R3[3][3],R4[3][3],R5[3][3],T3[3],T4[3],T5[5];
PQP_REAL R6[3][3],R7[3][3],T6[3],T7[3],T2_t[3],Ti[3];

PQP_REAL M0[3][3],M1[3][3],M2[3][3],M3[3][3],M4[3][3];
PQP_REAL M5[3][3],M6[3][3],M7[3][3];

PQP_REAL Mobs[3][3], Tobs[3];

bool withObs = true;
int env;

int step;

double oglm[16];

bool step_sim = false;
int sim_velocity;

PQP_REAL R02[3][3],R12[3][3],R22[3][3],T02[3],T12[3],T22[3];
PQP_REAL R32[3][3],R42[3][3],R52[3][3],T32[3],T42[3],T52[5];
PQP_REAL R62[3][3],R72[3][3],T62[3],T72[3],T2_t2[3];

PQP_REAL M02[3][3],M12[3][3],M22[3][3],M32[3][3],M42[3][3];
PQP_REAL M52[3][3],M62[3][3],M72[3][3];

typedef std::vector<std::vector<std::vector< double > > > Matrix_rod;
typedef std::vector<std::vector< double  > > Matrix_robo;
Matrix_rod RodStates;
Matrix_robo RoboStates;

int mode;
double beginx, beginy;
double dis = 2500.0, azim = 180.0, elev = 120.0;
double ddis = 700.0, dazim = 0.0, delev = 0.0;
double rot1 = 0.0, rot2 = 0.0, rot3 = 0.0;  //in radians
double rot4 = 0.0, rot5 = 0.0, rot6 = 0.0;
double rot12 = 0.0, rot22 = 0.0, rot32 = 0.0;  //in radians
double rot42 = 0.0, rot52 = 0.0, rot62 = 0.0;
int visualizeRobots = 1, visualize = 1;
//double offsetX=1092.1, offsetY=3.4, offsetZ=0.1, offsetRot=0.064270;
double offsetX=ROBOTS_DISTANCE, offsetY=0, offsetZ=0, offsetRot=0;

void pm(PQP_REAL M[][3], std::string str) {
	std::cout << str << std::endl;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (fabs(M[i][j])>1e-4)
				std::cout << M[i][j] << " ";
			else
				std::cout << 0 << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl << std::endl;
}

void pv(PQP_REAL V[3], std::string str) {
	std::cout << str << std::endl;

	for (int i = 0; i < 3; i++)
		std::cout << V[i] << " ";
	std::cout << std::endl;
}



void InitViewerWindow()
{
	GLfloat Ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat Diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat Specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat SpecularExp[] = { 50 };
	GLfloat Emission[] = { 0.1f, 0.1f, 0.1f, 1.0f };

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMaterialfv(GL_FRONT, GL_AMBIENT, Ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_FRONT, GL_EMISSION, Emission);

	glMaterialfv(GL_BACK, GL_AMBIENT, Ambient);
	glMaterialfv(GL_BACK, GL_DIFFUSE, Diffuse);
	glMaterialfv(GL_BACK, GL_SPECULAR, Specular);
	glMaterialfv(GL_BACK, GL_SHININESS, SpecularExp);
	glMaterialfv(GL_BACK, GL_EMISSION, Emission);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

	glEnable(GL_COLOR_MATERIAL);

	GLfloat light_position[] = { 10000.0, 10000.0, 10000.0, 10000000.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);
	glClearColor(135.0/255, 194.0/255, 242.0/255, 0.0);  //background color

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_NORMALIZE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.004,0.004,-0.004,0.004,.01,1000000000.0);  //part view cutoff warping

	glMatrixMode(GL_MODELVIEW);
}

void KeyboardCB(unsigned char key, int x, int y) 
{
	switch(key)
	{
	case 'q':
		delete base_to_draw;
		delete link1_to_draw;
		delete link2_to_draw;
		delete link3_to_draw;
		delete link4_to_draw;
		delete link5_to_draw;
		delete link6_to_draw;
		delete EE_to_draw;
		exit(0);
	case '1': rot1 += .1; break;
	case '2': rot2 += .1; break;
	case '3': rot3 += .1; break;
	case '4': rot4 += .1; break;
	case '5': rot5 += .1; break;
	case '6': rot6 += .1; break;
	case '7': rot12 += .1; break;
	case '8': rot22 += .1; break;
	case '9': rot32 += .1; break;
	case '0': rot42 += .1; break;
	case '-': rot52 += .1; break;
	case '=': rot62 += .1; break;
	}

	glutPostRedisplay();
}

void MouseCB(int _b, int _s, int _x, int _y)
{
	if (_s == GLUT_UP)
	{
		dis += ddis;
		azim += dazim;
		elev += delev;
		ddis = 0.0;
		dazim = 0.0;
		delev = 0.0;
		return;
	}

	if (_b == GLUT_RIGHT_BUTTON)
	{
		mode = 0;
		beginy = _y;
		return;
	}
	else
	{
		mode = 1;
		beginx = _x;
		beginy = _y;
	}
}

void MotionCB(int _x, int _y)
{
	if (mode == 0)
	{
		ddis = dis * (_y - beginy)/200.0;
	}
	else
	{
		dazim = (_x - beginx)/5.0;
		delev = (_y - beginy)/5.0;
	}

	glutPostRedisplay();
}

inline void glVertex3v(float V[3]) { glVertex3fv(V); }
inline void glVertex3v(double V[3]) { glVertex3dv(V); }

void BeginDraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0.0, 0.0, -(dis+ddis));
	glRotated(elev+delev, 1.0, 0.0, 0.0);
	glRotated(azim+dazim, 0.0, 1.0, 0.0);
}

void EndDraw()
{
	glFlush();
	glutSwapBuffers();
}

void IdleCB() 
{  
	glutPostRedisplay();
}

void DisplayCB()
{
	BeginDraw();

	// rotation matrix
	MRotZ(M0,0);     //base rotate Z
	MxM(R0,M0,M0);

	MRotZ(M1,rot1);  //link 1 rotate Z
	MxM(R1,R0,M1);

	MRotY(M2,rot2);  //link 2 rotate Y
	MxM(R2,R1,M2);

	MRotY(M3,rot3);  //link 3 rotate Y
	MxM(R3,R2,M3);

	MRotX(M4,rot4);  //link 4 rotate X
	MxM(R4,R3,M4);

	MRotY(M5,rot5);  //link 5 rotate Y
	MxM(R5,R4,M5);

	MRotX(M6,rot6);  //link 6 rotate X
	MxM(R6,R5,M6);

	MRotY(M7,3.1415926/2);
	MxM(R7,R6,M7);

	//define kinematics

	T0[0] =  -offsetX/2;
	T0[1] =  offsetY/2;
	T0[2] =  offsetZ/2;

	MxV(T0,R0,T0);

	if (visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R0,T0);
		glPushMatrix();
		glMultMatrixd(oglm);
		base_to_draw->Draw();
		glPopMatrix();
	}

	T1[0] =  0;
	T1[1] =  0;
	T1[2] =  145*2;

	MxV(T1,R0,T1);
	VpV(T1,T0,T1);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R1,T1);
		glPushMatrix();
		glMultMatrixd(oglm);
		link1_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  0;
	T2[1] =  0;
	T2[2] =  0;

	MxV(T2,R1,T2);
	VpV(T2,T1,T2);

	T2_t[0] = T2[0];
	T2_t[1] = T2[1];
	T2_t[2] = T2[2];

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R2,T2);
		glPushMatrix();
		glMultMatrixd(oglm);
		link2_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  0;
	T2[1] =  0;
	T2[2] =  270;

	MxV(T3,R2,T2);
	VpV(T3,T2_t,T3);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R3,T3);
		glPushMatrix();
		glMultMatrixd(oglm);
		link3_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  134;
	T2[1] =  0;
	T2[2] =  70;

	MxV(T4,R3,T2);
	VpV(T4,T4,T3);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R4,T4);
		glPushMatrix();
		glMultMatrixd(oglm);
		link4_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  168;
	T2[1] =  0;
	T2[2] =  0;

	MxV(T5,R4,T2);
	VpV(T5,T4,T5);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R5,T5);
		glPushMatrix();
		glMultMatrixd(oglm);
		link5_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  72-13/2;
	T2[1] =  0;
	T2[2] =  0;

	MxV(T6,R5,T2);
	VpV(T6,T5,T6);

	if(visualizeRobots == 1){
		glColor3d(0.0,0.0,0.0);
		MVtoOGL(oglm,R6,T6);
		glPushMatrix();
		glMultMatrixd(oglm);
		link6_to_draw->Draw();
		glPopMatrix();
	}

	T2[0] =  13/2+60;
	T2[1] =  0;
	T2[2] =  0;

	MxV(T7,R6,T2);
	VpV(T7,T6,T7);

	if(visualizeRobots == 1){
		glColor3d(1.0,1.0,1.0);//.5,.5);
		MVtoOGL(oglm,R7,T7);
		glPushMatrix();
		glMultMatrixd(oglm);
		EE_to_draw->Draw();
		glPopMatrix();
	}

	//ROBOT 2

	// rotation matrix
	MRotZ(M02,3.14159265/2+offsetRot);     //base rotate Z
	MxM(R02,M02,M02);

	MRotZ(M12,rot12);  //link 1 rotate Z
	MxM(R12,R02,M12);

	MRotY(M22,rot22);  //link 2 rotate Y
	MxM(R22,R12,M22);

	MRotY(M32,rot32);  //link 3 rotate Y
	MxM(R32,R22,M32);

	MRotX(M42,rot42);  //link 4 rotate X
	MxM(R42,R32,M42);

	MRotY(M52,rot52);  //link 5 rotate Y
	MxM(R52,R42,M52);

	MRotX(M62,rot62);  //link 6 rotate X
	MxM(R62,R52,M62);

	MRotY(M72,3.1415826/2);
	MxM(R72,R62,M72);

	//define kinematics

	T02[0] =  -offsetX/2;
	T02[1] =  offsetY/2;
	T02[2] =  offsetZ/2;

	MxV(T02,R02,T02);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R02,T02);
		glPushMatrix();
		glMultMatrixd(oglm);
		base_to_draw2->Draw();
		glPopMatrix();
	}

	T12[0] =  0;
	T12[1] =  0;
	T12[2] =  145*2;

	MxV(T12,R02,T12);
	VpV(T12,T02,T12);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R12,T12);
		glPushMatrix();
		glMultMatrixd(oglm);
		link1_to_draw2->Draw();
		glPopMatrix();
	}

	T22[0] =  0;
	T22[1] =  0;
	T22[2] =  0;

	MxV(T22,R12,T22);
	VpV(T22,T12,T22);

	T2_t2[0] = T22[0];
	T2_t2[1] = T22[1];
	T2_t2[2] = T22[2];

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R22,T22);
		glPushMatrix();
		glMultMatrixd(oglm);
		link2_to_draw2->Draw();
		glPopMatrix();
	}

	T22[0] =  0;
	T22[1] =  0;
	T22[2] =  270;

	MxV(T32,R22,T22);
	VpV(T32,T2_t2,T32);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R32,T32);
		glPushMatrix();
		glMultMatrixd(oglm);
		link3_to_draw2->Draw();
		glPopMatrix();
	}

	T22[0] =  134;
	T22[1] =  0;
	T22[2] =  70;

	MxV(T42,R32,T22);
	VpV(T42,T42,T32);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R42,T42);
		glPushMatrix();
		glMultMatrixd(oglm);
		link4_to_draw2->Draw();
		glPopMatrix();
	}

	T22[0] =  168;
	T22[1] =  0;
	T22[2] =  0;

	MxV(T52,R42,T22);
	VpV(T52,T42,T52);

	if(visualizeRobots == 1){
		glColor3d(0.5,0.5,0.5);
		MVtoOGL(oglm,R52,T52);
		glPushMatrix();
		glMultMatrixd(oglm);
		link5_to_draw2->Draw();
		glPopMatrix();
	}

	T22[0] =  72-13/2;
	T22[1] =  0;
	T22[2] =  0;

	MxV(T62,R52,T22);
	VpV(T62,T52,T62);

	if(visualizeRobots == 1){
		glColor3d(0.0,0.0,0.0);
		MVtoOGL(oglm,R62,T62);
		glPushMatrix();
		glMultMatrixd(oglm);
		link6_to_draw2->Draw();
		glPopMatrix();
	}

	Ti[0]=0;Ti[1]=0;Ti[2]=0;
	if(visualize == 1){
		glColor3d(.93, .69, .13);//94.0/255,48.0/255,13.0/255);//0.0,0.0,1.0);//
		MVtoOGL(oglm,R0,Ti);
		glPushMatrix();
		glMultMatrixd(oglm);
		table_to_draw->Draw();
		glPopMatrix();
	}
	if(1 && visualize == 1){
		glColor3d(0.8,0.9,0.9);
		MVtoOGL(oglm,R0,Ti);
		glPushMatrix();
		glMultMatrixd(oglm);
		room_to_draw->Draw();
		glPopMatrix();
	}
	if(visualize == 1){
		glColor3d(1.0,1.0,0.7);
		MVtoOGL(oglm,R0,Ti);
		glPushMatrix();
		glMultMatrixd(oglm);
		floor_to_draw->Draw();
		glPopMatrix();
	}


	T22[0] =  13/2+60;
	T22[1] =  0.0;
	T22[2] =  0;

	MxV(T72,R62,T22);
	VpV(T72,T62,T72);

	/*pm(R22, "R2");
	pv(T22, "T2");
	pv(T2_t2, "T2_t");
	pm(R32, "R3");
	pv(T32, "T3");
	pm(R42, "R4");
	pv(T42, "T4");
	pm(R52, "R5");
	pv(T52, "T5");
	pm(R62, "R6");
	pv(T62, "T6");
	pv(T72, "T6");*/


	if(visualizeRobots == 1){
		glColor3d(1.0,1.0,1.0);
		MVtoOGL(oglm,R72,T72);
		glPushMatrix();
		glMultMatrixd(oglm);
		EE_to_draw->Draw();
		glPopMatrix();
	}

	// Environment I
	if (withObs && env == 1) {

		// Obs 1
		MRotZ(Mobs,0);
		//MxM(R0,Mobs,Mobs);

		Tobs[0] =  0;
		Tobs[1] =  0;
		Tobs[2] =  0;

		if(visualize == 1 && withObs) {
			glColor3d(1.0,1.0,1.0);
			MVtoOGL(oglm,Mobs,Tobs);
			glPushMatrix();
			glMultMatrixd(oglm);
			obs1_to_draw->Draw();
			glPopMatrix();
		}

		double dObs = 250;

		// Obs 2
		MRotZ(Mobs,0);
		//MxM(R0,Mobs,Mobs);

		Tobs[0] =  dObs;
		Tobs[1] =  dObs;
		Tobs[2] =  0;

		if(visualize == 1 && withObs) {
			glColor3d(1.0,1.0,1.0);
			MVtoOGL(oglm,Mobs,Tobs);
			glPushMatrix();
			glMultMatrixd(oglm);
			obs2_to_draw->Draw();
			glPopMatrix();
		}

		// Obs 1
		MRotZ(Mobs,0);
		//MxM(R0,Mobs,Mobs);

		Tobs[0] =  -dObs;
		Tobs[1] =  -dObs;
		Tobs[2] =  0;

		if(visualize == 1 && withObs) {
			glColor3d(1.0,1.0,1.0);
			MVtoOGL(oglm,Mobs,Tobs);
			glPushMatrix();
			glMultMatrixd(oglm);
			obs3_to_draw->Draw();
			glPopMatrix();
		}
	}

	// Environment II
	if (withObs && env == 2) {

		// Obs 1
		MRotZ(Mobs,0);
		//MxM(R0,Mobs,Mobs);

		Tobs[0] =  0;
		Tobs[1] =  0;
		Tobs[2] =  0;

		if(visualize == 1 && withObs) {
			glColor4f(1.0,0.0,0.0,1);
			MVtoOGL(oglm,Mobs,Tobs);
			glPushMatrix();
			glMultMatrixd(oglm);
			obs1_to_draw->Draw();
			glPopMatrix();
		}

		// Obs 2
		MRotX(Mobs,3.14);
		//MxM(R0,Mobs,Mobs);

		Tobs[0] =  0;
		Tobs[1] =  0;
		Tobs[2] =  790;

		if(visualize == 1 && withObs) {
			glColor4f(1.0,0.0,0.0,1);
			MVtoOGL(oglm,Mobs,Tobs);
			glPushMatrix();
			glMultMatrixd(oglm);
			obs2_to_draw->Draw();
			glPopMatrix();
		}
	}

	/*
  PQP_REAL rel_err=0, abs_err=0;
  int qsize = 2;
  PQP_DistanceResult dres[14];

  // perform tolerance query


  PQP_Distance(&dres[0],R6,T7,&rod,R0,T0,&base,rel_err,abs_err,qsize);
  PQP_Distance(&dres[1],R6,T7,&rod,R1,T1,&link1,rel_err,abs_err,qsize);
  PQP_Distance(&dres[2],R6,T7,&rod,R2,T2_t,&link2,rel_err,abs_err,qsize);
  PQP_Distance(&dres[3],R6,T7,&rod,R3,T3,&link3,rel_err,abs_err,qsize);
  PQP_Distance(&dres[4],R6,T7,&rod,R4,T4,&link4,rel_err,abs_err,qsize);
  PQP_Distance(&dres[5],R6,T7,&rod,R5,T5,&link5,rel_err,abs_err,qsize);
  PQP_Distance(&dres[6],R6,T7,&rod,R6,T6,&link6,rel_err,abs_err,qsize);
  PQP_Distance(&dres[7],R6,T7,&rod,R02,T02,&base2,rel_err,abs_err,qsize);
  PQP_Distance(&dres[8],R6,T7,&rod,R12,T12,&link12,rel_err,abs_err,qsize);
  PQP_Distance(&dres[9],R6,T7,&rod,R22,T2_t2,&link22,rel_err,abs_err,qsize);
  PQP_Distance(&dres[10],R6,T7,&rod,R32,T32,&link32,rel_err,abs_err,qsize);
  PQP_Distance(&dres[11],R6,T7,&rod,R42,T42,&link42,rel_err,abs_err,qsize);
  PQP_Distance(&dres[12],R6,T7,&rod,R52,T52,&link52,rel_err,abs_err,qsize);
  PQP_Distance(&dres[13],R6,T7,&rod,R62,T62,&link62,rel_err,abs_err,qsize);
	 */

	for(int i=0;i<RodStates[step].size();i++){
		PQP_REAL P[3],V[3],D1[3],D2[3],P1[3],P2[3];
		P[0]=RodStates[step][i][0];
		P[1]=RodStates[step][i][1];
		P[2]=RodStates[step][i][2];

		MxVpV(V,R6,P,T7);
		glColor3d(0.0,0.0,0.0);
		glPushMatrix();
		glTranslated(V[0],V[1],V[2]);
		glutSolidSphere(14,15,15); //(8,15,15)
		glPopMatrix();

		/*for (int i=0; i<3; i++) {
    	for (int j=0; j<3; j++)
    		std::cout << R6[i][j] << " ";
    	std::cout << std::endl;
    }
    for (int i=0; i<3; i++)
    	std::cout << T7[i] << std::endl;*/

		//checks some point on rod w/ some point on a link: proof works
		/*
    VcV(P1,dres[9].P1());
    VcV(P2,dres[9].P2());
    MxVpV(D1,R6,P1,T6);
    MxVpV(D2,R22,P2,T2_t2);

    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glVertex3v(D1);
    glVertex3v(D2);
    glEnd();
    glEnable(GL_LIGHTING);
		 */
	}

	EndDraw();
}


void load_models(){

	// initialize the base

	FILE *fp;
	int i, ntris;

	base_to_draw = new Model("Base_r.tris");

	fp = fopen("Base_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Base_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	base.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		base.AddTri(p1,p2,p3,i);
	}
	base.EndModel();
	fclose(fp);

	// initialize link 1

	link1_to_draw = new Model("Link1_r.tris");

	fp = fopen("Link1_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link1_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link1.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link1.AddTri(p1,p2,p3,i);
	}
	link1.EndModel();
	fclose(fp);

	// initialize link2
	link2_to_draw = new Model("Link2_r.tris");

	fp = fopen("Link2_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link2_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link2.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link2.AddTri(p1,p2,p3,i);
	}
	link2.EndModel();
	fclose(fp);

	// initialize link3
	link3_to_draw = new Model("Link3_r.tris");

	fp = fopen("Link3_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link3_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link3.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link3.AddTri(p1,p2,p3,i);
	}
	link3.EndModel();
	fclose(fp);

	// initialize link4
	link4_to_draw = new Model("Link4_r2.tris");

	fp = fopen("Link4_r2.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link4_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link4.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link4.AddTri(p1,p2,p3,i);
	}
	link4.EndModel();
	fclose(fp);

	// initialize link5
	link5_to_draw = new Model("Link5_r.tris");

	fp = fopen("Link5_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link5_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link5.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link5.AddTri(p1,p2,p3,i);
	}
	link5.EndModel();
	fclose(fp);

	// initialize link6
	link6_to_draw = new Model("Link6.tris");

	fp = fopen("Link6.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link6_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link6.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link6.AddTri(p1,p2,p3,i);
	}
	link6.EndModel();
	fclose(fp);

	// initialize EE
	EE_to_draw = new Model("EE_r.tris");

	fp = fopen("EE_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open EE_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	EE.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		EE.AddTri(p1,p2,p3,i);
	}
	EE.EndModel();
	fclose(fp);


	//ROBOT 2

	base_to_draw2 = new Model("Base_r.tris");

	fp = fopen("Base_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Base_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	base2.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		base2.AddTri(p1,p2,p3,i);
	}
	base2.EndModel();
	fclose(fp);

	// initialize link 1

	link1_to_draw2 = new Model("Link1_r.tris");

	fp = fopen("Link1_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link1_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link12.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link12.AddTri(p1,p2,p3,i);
	}
	link12.EndModel();
	fclose(fp);

	// initialize link2
	link2_to_draw2 = new Model("Link2_r.tris");

	fp = fopen("Link2_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link2_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link22.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link22.AddTri(p1,p2,p3,i);
	}
	link22.EndModel();
	fclose(fp);

	// initialize link3
	link3_to_draw2 = new Model("Link3_r.tris");

	fp = fopen("Link3_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link3_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link32.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link32.AddTri(p1,p2,p3,i);
	}
	link32.EndModel();
	fclose(fp);

	// initialize link4
	link4_to_draw2 = new Model("Link4_r2.tris");

	fp = fopen("Link4_r2.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link4_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link42.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link42.AddTri(p1,p2,p3,i);
	}
	link42.EndModel();
	fclose(fp);

	// initialize link5
	link5_to_draw2 = new Model("Link5_r.tris");

	fp = fopen("Link5_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link5_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link52.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link52.AddTri(p1,p2,p3,i);
	}
	link52.EndModel();
	fclose(fp);

	// initialize link6
	link6_to_draw2 = new Model("Link6.tris");

	fp = fopen("Link6.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open Link6_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	link62.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		link62.AddTri(p1,p2,p3,i);
	}
	link62.EndModel();
	fclose(fp);

	// initialize EE
	EE_to_draw2 = new Model("EE_r.tris");

	fp = fopen("EE_r.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open EE_r.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	EE2.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		EE2.AddTri(p1,p2,p3,i);
	}
	EE2.EndModel();
	fclose(fp);

	// initialize table

	table_to_draw = new Model("table.tris");

	fp = fopen("table.tris","r");
	if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
	fscanf(fp,"%d",&ntris);

	table.BeginModel();
	for (i = 0; i < ntris; i++)
	{
		double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
		fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
				&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
		p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
		p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
		table.AddTri(p1,p2,p3,i);
	}
	table.EndModel();
	fclose(fp);

	if (withObs) {

		if (env == 1) {
			// initialize obs1
			obs1_to_draw = new Model("obs.tris");

			fp = fopen("obs.tris","r");
			if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
			fscanf(fp,"%d",&ntris);

			obs1.BeginModel();
			for (i = 0; i < ntris; i++)
			{
				double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
				fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
				PQP_REAL p1[3],p2[3],p3[3];
				p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
				p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
				p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
				obs1.AddTri(p1,p2,p3,i);
			}
			obs1.EndModel();
			fclose(fp);

			// initialize obs2
			obs2_to_draw = new Model("obs.tris");

			fp = fopen("obs.tris","r");
			if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
			fscanf(fp,"%d",&ntris);

			obs2.BeginModel();
			for (i = 0; i < ntris; i++)
			{
				double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
				fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
				PQP_REAL p1[3],p2[3],p3[3];
				p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
				p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
				p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
				obs2.AddTri(p1,p2,p3,i);
			}
			obs2.EndModel();
			fclose(fp);

			// initialize obs3
			obs3_to_draw = new Model("obs.tris");

			fp = fopen("obs.tris","r");
			if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
			fscanf(fp,"%d",&ntris);

			obs3.BeginModel();
			for (i = 0; i < ntris; i++)
			{
				double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
				fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
				PQP_REAL p1[3],p2[3],p3[3];
				p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
				p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
				p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
				obs3.AddTri(p1,p2,p3,i);
			}
			obs3.EndModel();
			fclose(fp);
		}

		if (env == 2) {
			// initialize obs1
			obs1_to_draw = new Model("cone.tris");

			fp = fopen("cone.tris","r");
			if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
			fscanf(fp,"%d",&ntris);

			obs1.BeginModel();
			for (i = 0; i < ntris; i++)
			{
				double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
				fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
				PQP_REAL p1[3],p2[3],p3[3];
				p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
				p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
				p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
				obs1.AddTri(p1,p2,p3,i);
			}
			obs1.EndModel();
			fclose(fp);

			// initialize obs2
			obs2_to_draw = new Model("cone.tris");

			fp = fopen("cone.tris","r");
			if (fp == NULL) { fprintf(stderr,"Couldn't open table.tris\n"); exit(-1); }
			fscanf(fp,"%d",&ntris);

			obs2.BeginModel();
			for (i = 0; i < ntris; i++)
			{
				double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
				fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
						&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
				PQP_REAL p1[3],p2[3],p3[3];
				p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
				p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
				p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
				obs2.AddTri(p1,p2,p3,i);
			}
			obs2.EndModel();
			fclose(fp);
		}

		// initialize room
		room_to_draw = new Model("room.tris");

		fp = fopen("room.tris","r");
		if (fp == NULL) { fprintf(stderr,"Couldn't open room.tris\n"); exit(-1); }
		fscanf(fp,"%d",&ntris);

		room.BeginModel();
		for (i = 0; i < ntris; i++)
		{
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			PQP_REAL p1[3],p2[3],p3[3];
			p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
			p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
			p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
			room.AddTri(p1,p2,p3,i);
		}
		room.EndModel();
		fclose(fp);

		// initialize floor
		floor_to_draw = new Model("floor.tris");

		fp = fopen("floor.tris","r");
		if (fp == NULL) { fprintf(stderr,"Couldn't open floor.tris\n"); exit(-1); }
		fscanf(fp,"%d",&ntris);

		floor1.BeginModel();
		for (i = 0; i < ntris; i++)
		{
			double p1x,p1y,p1z,p2x,p2y,p2z,p3x,p3y,p3z;
			fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&p1x,&p1y,&p1z,&p2x,&p2y,&p2z,&p3x,&p3y,&p3z);
			PQP_REAL p1[3],p2[3],p3[3];
			p1[0] = (PQP_REAL)p1x; p1[1] = (PQP_REAL)p1y; p1[2] = (PQP_REAL)p1z;
			p2[0] = (PQP_REAL)p2x; p2[1] = (PQP_REAL)p2y; p2[2] = (PQP_REAL)p2z;
			p3[0] = (PQP_REAL)p3x; p3[1] = (PQP_REAL)p3y; p3[2] = (PQP_REAL)p3z;
			EE2.AddTri(p1,p2,p3,i);
		}
		floor1.EndModel();
		fclose(fp);

	}

}

void execute_path(int k){

	//std::cout << "Cong.: " << k << std::endl;

	if(k == 0){
		const char* rod_pfile = "../paths/rod.txt";
		const char* robot_pfile = "../paths/path.txt";
		FILE *fro, *fr;
		int i, nlines;

		fr = fopen(robot_pfile,"r");
		if (fr == NULL) { fprintf(stderr,"Couldn't open robot_path.txt\n"); exit(-1); }
		fscanf(fr,"%i",&nlines);  //NOT include number in line count itself
		RoboStates.resize(nlines);

		//std::cout << "Number of configurations in path: " << RoboStates.size() << std::endl;

		for (i = 0; i < nlines; i++)
		{
			double rot1T,rot2T,rot3T,rot4T,rot5T,rot6T;
			double rot52T,rot62T,rot12T,rot22T,rot32T,rot42T;
			fscanf(fr,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
					&rot1T,&rot2T,&rot3T,&rot4T,&rot5T,&rot6T,&rot12T, \
					&rot22T,&rot32T,&rot42T,&rot52T,&rot62T);

			RoboStates[i].resize(12);
			RoboStates[i][0]=rot1T;RoboStates[i][1]=rot2T;RoboStates[i][2]=rot3T;
			RoboStates[i][3]=rot4T;RoboStates[i][4]=rot5T;RoboStates[i][5]=rot6T;
			RoboStates[i][6]=rot12T;RoboStates[i][7]=rot22T;RoboStates[i][8]=rot32T;
			RoboStates[i][9]=rot42T;RoboStates[i][10]=rot52T;RoboStates[i][11]=rot62T;
		}

		fclose(fr);

		fro = fopen(rod_pfile,"r");
		if (fro == NULL) { fprintf(stderr,"Couldn't open rod_path.txt\n"); exit(-1); }
		fscanf(fro,"%i",&nlines);  //DO include number in line count itself

		int config_num = 0;
		RodStates.resize(nlines/500);

		for(i=0;i<nlines/500;i++){
			RodStates[i].resize(500);
			for(int j=0;j<500;j++){
				RodStates[i][j].resize(3);
			}
		}


		for (i = 2; i <= nlines; i++)
		{
			double px,py,pz;
			if (i-501*config_num == 502){
				config_num += 1;
				continue;
			}

			fscanf(fro,"%lf %lf %lf",&px,&py,&pz);

			int index = i-501*config_num-2;

			if (config_num == 0 && i < 500){index=i-2;}

			if (index == 500){std::cout << "error in indexing" << std::endl; break;}

			RodStates[config_num][index][0]=px;
			RodStates[config_num][index][1]=py;
			RodStates[config_num][index][2]=pz;
		}

		fclose(fro);

		if(RodStates.size() != RoboStates.size()){
			std::cout << "error! Non-equal sizes" << std::endl;
		}
	}

	rot1 = RoboStates[k][0];
	rot2 = RoboStates[k][1];
	rot3 = RoboStates[k][2];
	rot4 = RoboStates[k][3];
	rot5 = RoboStates[k][4];
	rot6 = RoboStates[k][5];
	rot12 = RoboStates[k][6];
	rot22 = RoboStates[k][7];
	rot32 = RoboStates[k][8];
	rot42 = RoboStates[k][9];
	rot52 = RoboStates[k][10];
	rot62 = RoboStates[k][11];

	//for (int jj = 0; jj < 12; jj++)
	//	std::cout << RoboStates[k][jj] << " ";
	//std::cout << std::endl;

	rod.BeginModel();

	for (int i=0; i<RodStates[k].size()/3; i++){
		PQP_REAL p1[3],p2[3],p3[3];
		p1[0] = RodStates[k][3*i][0];
		p1[1] = RodStates[k][3*i][1];
		p1[2] = RodStates[k][3*i][2];  //P1
		p2[0] = RodStates[k][3*i+1][0];
		p2[1] = RodStates[k][3*i+1][1];
		p2[2] = RodStates[k][3*i+1][2];  //P2
		p3[0] = RodStates[k][3*i+2][0];
		p3[1] = RodStates[k][3*i+2][1];
		p3[2] = RodStates[k][3*i+2][2];  //P3
		rod.AddTri(p1,p2,p3,i);
	}

	rod.EndModel();

	PQP_DistanceResult dres;
	PQP_REAL rel_err = 0.0;
	PQP_REAL abs_err = 0.0;
	PQP_Distance(&dres,R0,T0,&rod,R0,T0,&base,rel_err,abs_err);

	glutPostRedisplay();
	if(k<RoboStates.size()-1){
		//if (step==1)
		//	sleep(5);

		step+=1;
		glutTimerFunc(sim_velocity,execute_path,k+1);

		if (step_sim) {
			std::cout << step << std::endl;
			std::cin.ignore();
		}
	}

}

int fake_rod() {
	const char* robot_pfile = "../paths/path.txt";
	FILE *fr = fopen(robot_pfile,"r");
	int nlines;
	fscanf(fr,"%i",&nlines);  //NOT include number in line count itself
	std::cout << "Num: " << nlines << std::endl;
	fclose(fr);

	const char* rod_pfile = "../paths/rod.txt";

	FILE *f = fopen(rod_pfile, "w");

	fprintf(f, "%d\n", 501*nlines);

	double L;
	if (env == 1)
		L = ROD_LENGTH;
	else if (env == 2)
		L = ROD_LENGTH_ENVII;

	double dt = L/500;
	for (int i=0; i<nlines; i++) {
		for (int j = 0; j<500; j++)
			fprintf(f, "%f 0 0\n", j*dt);
		fprintf(f, "\n");
	}
	fclose(f);
}

int main(int argc, char **argv)
{
	if (argc == 2 && !strcmp(argv[1],"-h")) {
		std::cout << std::endl;
		std::cout << " ABB IRB-120 dual-arm simulator" << std::endl;
		std::cout << " Syntex: " << std::endl;
		std::cout << "   ./viz <mode> <velocity> <environment>" << std::endl;
		std::cout << "      <mode>: 0 - Continuous motion, 1 - Step by step with the press of a button."  << std::endl;
		std::cout << "      <velocity>: Control the velocity by indicating the time between poses, in (msec)." << std::endl;
		std::cout << "      <environment>: 1 - env. I with three poles, 1 - env. II with two cones (narrow passage)." << std::endl << std::endl;
		return 0;
	}

	if (argc > 1) {
		if (atof(argv[1])==0)
			step_sim = false; 
		else if (atof(argv[1])==1)
			step_sim = true;
		else {
			std::cout << "Invalid entry. Type -h for list of options." << std::endl;
			return 1;
		}

		if (argc > 2)
			sim_velocity = atoi(argv[2]);
		else
			sim_velocity = 80;
		if (argc == 4) {
			env = atoi(argv[3]);
			if (env != 1 && env != 2)
				env = 1;
		}
		else
			env = 1;
	}
	else {
		step_sim = false;
		sim_velocity = 80;
		env = 1;
	}

	fake_rod(); // Create a fake rod file

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

	// create the window

	if (env == 2)
		offsetX = ROBOTS_DISTANCE_ENVII;

	glutCreateWindow("Robot View");

	// load robot meshes
	load_models();

	// set OpenGL graphics state -- material props, perspective, etc.

	InitViewerWindow();

	// set the callbacks
	glutTimerFunc(0,execute_path,0);
	glutDisplayFunc(DisplayCB);
	glutIdleFunc(IdleCB);
	glutMouseFunc(MouseCB);
	glutMotionFunc(MotionCB);
	glutKeyboardFunc(KeyboardCB);
	glutReshapeWindow(1000,1000);

	// Enter the main loop.
	glutMainLoop();
}



