#pragma once
#include <iostream>
#include <Ogre.h>

typedef enum {LOW, MEDIUM, HIGH} IPSAnimationPriority;

class IPSAnimation {

public:
	bool update;
	bool loop;
	bool hold;
	bool erase;
	Ogre::Real weight;
	std::string name;
	IPSAnimationPriority priority;
	IPSAnimation(): loop(false),hold(false),name(""),priority(LOW), update(true), weight(1.0), erase(false) {}
	IPSAnimation(std::string name, bool loop, bool hold, bool update, Ogre::Real weight, IPSAnimationPriority priority): name(name), loop(loop), hold(hold), update(update), weight(weight), priority(priority){}
};
class AnimationManager{
	private:
			Ogre::Entity * entity; //this is the entity the animation manager is controlling
			std::list<IPSAnimation> animations;

	public:
		void playAnimation(std::string name, bool loop=false, bool hold=false, bool update = true, Ogre::Real weight = 1.0, IPSAnimationPriority priority = LOW);
		void stopAnimation(std::string name);
		void update(Ogre::Real deltaT);
		AnimationManager(Ogre::Entity * e): entity(e){}
};