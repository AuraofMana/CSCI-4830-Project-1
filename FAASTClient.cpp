#include "FAASTClient.h"

void VRPN_CALLBACK faast_tracker_handler(void * user, vrpn_TRACKERCB data){
	FAASTClient * faast = (FAASTClient *) user;
	faast->handleTracker(data.sensor,data.pos,data.quat);
}

void FAASTClient::handleTracker(int num, double pos[3],double ori[4]){
	joints[num].ori[0] = ori[0];
	joints[num].ori[1] = ori[1];
	joints[num].ori[2] = ori[2];
	joints[num].ori[3] = ori[3];
	joints[num].pos[0] = pos[0];
	joints[num].pos[1] = pos[1];
	joints[num].pos[2] = pos[2];
	//std::cout << "num=" << num << "pos = " << pos[0] << "," <<pos[1] << "," << pos[2] << std::endl;
}
bool FAASTClient::init(std::string name, std::string server, int port){
	char temp[1000];
	sprintf_s(temp,1000,"%s@%s:%d",name.c_str(),server.c_str(),port);
	tracker = new vrpn_Tracker_Remote(temp);
	tracker->register_change_handler(this,faast_tracker_handler);
	this->joints.clear();
	for(int i=0;i<24;i++){
		joints.push_back(FAASTJoint());
	}
	return true;
}
void FAASTClient::update(){
	tracker->mainloop();
}
FAASTJoint FAASTClient::getJoint(int num){
	return joints[num];
}