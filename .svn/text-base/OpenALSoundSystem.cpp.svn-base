#include "OpenALSoundSystem.h"

using namespace std;

OpenALSoundSystem::OpenALSoundSystem(){
		double pos[3] = {0,0,0};
		double vel[3] = {0,0,0};
		double forward[3] = {0,0,-1};
		double up[3] = {0,1,0};
		setListenerData(pos,vel,forward,up);
		 

	}

	OpenALSoundSystem::~OpenALSoundSystem(){
		std::map<std::string,ALuint>::iterator it = sounds.begin();
		for(;it!=sounds.end();it++){
			alDeleteBuffers(1,&(it->second));
		}
		std::map<std::string,ALSource>::iterator it2 = sources.begin();
		for(;it2!=sources.end();it2++){
			alDeleteSources(1,&(it2->second.id));
		}
		alutExit();
		sounds.clear();
		sources.clear();

	}
	void OpenALSoundSystem::init(){
		 alutInit(NULL,NULL);
		 alDopplerFactor(1); //normal doppler effect, set to zero to disable
		 alDopplerVelocity(300); //just force 300 meters/second as the speed of sound

	}

	void OpenALSoundSystem::update(){

		std::map<std::string, ALSource>::iterator it = sources.begin();
		for(;it!=sources.end();it++){
			ALuint source_id = it->second.id;
			ALfloat * source_pos = it->second.pos;
			ALfloat * source_vel = it->second.vel;
			alSourcefv(source_id, AL_POSITION, source_pos);
			alSourcefv(source_id, AL_VELOCITY, source_vel);
		}

		alListenerfv(AL_POSITION,    listener.pos);
		alListenerfv(AL_VELOCITY,    listener.vel);
		float ori[6];
		for(int i = 0;i<3;i++){
			ori[i] = listener.forward[i];
			ori[3+i] = listener.up[i];
		}
		alListenerfv(AL_ORIENTATION, ori);

	}

	void OpenALSoundSystem::assignSourceSound(std::string sourceName, std::string soundName, float pitch, float gain, int loop){

		ALSource source = sources[sourceName];
		ALuint source_id = source.id;
		ALuint sound_id = sounds[soundName];

		alSourcei (source_id, AL_BUFFER,   sound_id   );
		alSourcef (source_id, AL_PITCH,    pitch     );
		alSourcef (source_id, AL_GAIN,     gain     );
		alSourcei (source_id, AL_LOOPING,  loop     );

	}

	void OpenALSoundSystem::createSource(std::string name, double pos[3],double vel[3]){
		ALSource s;
		s.name = name;
		for(int i =0;i<3;i++){
			s.pos[i] = pos[i];
			s.vel[i] = vel[i];
		}
		ALuint source;
		  // Bind buffer with a source.
		alGenSources(1, &source);

		if (alGetError() != AL_NO_ERROR){
			std::cerr << "error creating source" << std::endl;
			return;
		}
		s.id = source;
		sources[name] = s;

	}

	void OpenALSoundSystem::setListenerData(double pos[3],double vel[3],double forward[3], double up[3]){
		for(int i =0;i<3;i++){
			listener.pos[i] = pos[i];
			listener.vel[i] = vel[i];
			listener.forward[i] = forward[i];
			listener.up[i] = up[i];
		}
	}

	void OpenALSoundSystem::setSourceData(std::string name, double pos[3], double vel[3]){
		ALSource source = sources[name];
		for(int i =0;i<3;i++){
			source.pos[i] = pos[i];
			source.vel[i] = vel[i];
			//std::cout << source.pos[i] << ",";
		}
		//std::cout << std::endl;
		sources[name] = source;
	}
	void OpenALSoundSystem::createSound(std::string filename, std::string name){

		ALenum format;
		ALsizei size;
		ALvoid* data;
		ALsizei freq;
		ALboolean loop;
		ALuint buffer;
		// Load wav data into a buffer.
		alGenBuffers(1, &buffer);
		
		if (alGetError() != AL_NO_ERROR){
			std::cerr << "Error loading" << filename << std::endl;
			return;
		}
		alutLoadWAVFile((ALbyte *)filename.c_str(), &format, &data, &size, &freq, &loop);
		if (alGetError() != AL_NO_ERROR){
			std::cerr << "Error loading" << filename << std::endl;
			return;
		}
		alBufferData(buffer, format, data, size, freq);
		if (alGetError() != AL_NO_ERROR){
			std::cerr << "Error loading" << filename << std::endl;
			return;
		}
		alutUnloadWAV(format, data, size, freq);
		if (alGetError() != AL_NO_ERROR){
			std::cerr << "Error loading" << filename << std::endl;
			return;
		}

		sounds[name] = buffer;
	}

	void OpenALSoundSystem::playSound(std::string name){

		alSourcePlay(sources[name].id);

	}
	void OpenALSoundSystem::stopSound(std::string name){
		alSourceStop(sources[name].id);

	}
	void OpenALSoundSystem::pauseSound(std::string name){
		alSourcePause(sources[name].id);
	}