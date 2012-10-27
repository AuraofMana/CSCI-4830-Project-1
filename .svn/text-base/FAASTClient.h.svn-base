#pragma once
#include <iostream>
#include <vrpn_Tracker.h>
#include <vector>
class FAASTJoint {
public:
	double pos[3];
	double ori[4];
	FAASTJoint(){
		pos[0]=pos[1]=pos[2] = 0.0;
		ori[0] = 1.0;
		ori[1] = 0.0;
		ori[2] = 0.0;
		ori[3] = 0.0;
	}
};
class FAASTClient {
	friend void VRPN_CALLBACK faast_tracker_handler(void * user, vrpn_TRACKERCB data);
private:
	vrpn_Tracker_Remote * tracker;
	std::vector<FAASTJoint> joints;
private:
	void handleTracker(int joint_num, double pos[3],double ori[4]);
public:
	bool init(std::string name, std::string server, int port);
	void update();
	FAASTJoint getJoint(int joint_num);

};