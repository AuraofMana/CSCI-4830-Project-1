#pragma once

#include <al.h>
#include <alc.h>
#include <alut.h>

#include <vector>
#include <map>
#include <string>
#include <iostream>

//helper classes, representing a sound source and listener
class ALSource {
public:
	ALuint id;
	ALfloat pos[3];
	ALfloat vel[3];
	std::string name;
};

class ALListener {
public:
	ALfloat pos[3];
	ALfloat vel[3];
	ALfloat up[3];  //these two define the coordinate system for orientation (note, you only need two vectors)
	ALfloat forward[3]; 
};


class OpenALSoundSystem {

private:
	//maps between names and their OpenAL counterpart objects
	std::map<std::string, ALuint> sounds;
	std::map<std::string, ALSource> sources;
	ALListener listener; //you only have 1 listener (the user)

public:

	OpenALSoundSystem();
	~OpenALSoundSystem();

	//you must call this prior to doing anything else
	void init();

	//you must call this every frame to update any information you set
	void update();

	//this actually binds a source to a sound (you can bind the same sound to multiple sources)
	void assignSourceSound(std::string sourceName, std::string soundName, float pitch = 1.0f, float gain = 1.0f, int loop = 0);
	
	//this creates a source at an initial position and velocity
	void createSource(std::string name, double pos[3],double vel[3]);
	//this sets the single listener's position, velocity, and orientation
	void setListenerData(double pos[3],double vel[3],double forward[3], double up[3]);
	//this sets a sources position and velocity (remember to call update)
	void setSourceData(std::string name, double pos[3], double vel[3]);
	//this creates a new sound, loading it from a file (must be a .wav)
	void createSound(std::string filename, std::string name);
	//play a source
	void playSound(std::string name);
	//stop a source
	void stopSound(std::string name);
	//pause a source
	void pauseSound(std::string name);

};