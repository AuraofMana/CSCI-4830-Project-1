#include "AnimationManager.h"
#include <list>
#include <ogre.h>
using namespace Ogre;
using namespace std;

void AnimationManager::playAnimation(std::string name, bool loop, bool hold, bool update, Ogre::Real weight, IPSAnimationPriority priority){

	if(!this)
		return;
	
	AnimationStateSet * ass = entity->getAllAnimationStates();
	if(!ass->hasAnimationState(name)){
		cout << "no animation found of name " << name << endl;	
		return;
	}
	cout << "playing animation " << name << endl;

	std::list<IPSAnimation>::iterator it = animations.begin();
	for(it = animations.begin();it!=animations.end();it++){
		if(it->name == name){
			cout << "restarting animation " << name << endl;
			animations.erase(it);
			break;
		}
	}

	//make sure this will be the only hold animation (clearly there can be only one, because neither would ever stop...)
	if(hold){
		for(it = animations.begin();it!=animations.end();it++){
			if(it->hold){
				cout << "killing existing hold animation " << it->name << endl;
				it->hold = false;
			}
		}
	}

	IPSAnimation anim(name,loop,hold,update,weight,priority);
	animations.push_back(anim);

	//enable the animation and initialize
	ass->getAnimationState(name)->setEnabled(true);
	ass->getAnimationState(name)->setLoop(loop);
	ass->getAnimationState(name)->setTimePosition(0.0);
	//we don't need to blend anything if their are no other animations
	if(animations.size() == 1){
			ass->getAnimationState(name)->setWeight(weight);
	}
	else{
		ass->getAnimationState(name)->setWeight(0);
	}
}
void AnimationManager::stopAnimation(std::string name){

	std::list<IPSAnimation>::iterator it = animations.begin();
	Ogre::AnimationStateSet *ass = entity->getAllAnimationStates();
	if(!ass->hasAnimationState(name)){
		cout << "no animation found to stop by name " << name << endl;	
		return;
	}
	
	cout << "stopping animation " << name << endl;

	Ogre::AnimationState * as = ass->getAnimationState(name);

	bool changes = false;
	while(true){
		changes = false;
		for(it = animations.begin();it!=animations.end();it++){
			//find the last highest priority animation
			if(it->name == name){
				as->setEnabled(false);
				as->setTimePosition(0.0);
				animations.erase(it);
				changes = true;
				break;
			}
		}
		if(!changes){
			break;
		}
	}
}
void AnimationManager::update(Ogre::Real deltaT){
	if(!this)
		return; //for some reason, an update is getting called before the animation manager is instantiated

	if(animations.size() <= 0){
		return;
	}

	std::list<IPSAnimation>::iterator it = animations.begin();
	std::list<IPSAnimation>::iterator highest = animations.begin();
	
	for(it = animations.begin();it!=animations.end();it++){
		//find the last highest priority animation
		if(it->priority >= highest->priority && it->erase != true){
			highest = it;
		
		}
	}

	//cout << "highest priority animation=" << highest->name << endl;

	//given the highest priority animation, blend down all others, and blend up this animation
	Ogre::AnimationStateSet * ass = entity->getAllAnimationStates();
	for(it = animations.begin();it!=animations.end();it++){
		if(!ass->hasAnimationState(it->name))continue;
		Ogre::AnimationState * as = ass->getAnimationState(it->name);
		Ogre::Real weight = as->getWeight();

		if(it == highest){
			weight += deltaT;
		}
		else{
			weight -= deltaT;
		}
		
		if(weight < 0){
			weight = 0;
		}
		if(weight > it->weight){
			weight = it->weight;
		}
		as->setWeight(weight);
	}

	//add time to each animation that requires an update, and stop those that need to be held at the end
	for(it = animations.begin();it!=animations.end();it++){
		//get the animation state
		if(!ass->hasAnimationState(it->name))continue;
		Ogre::AnimationState * as = ass->getAnimationState(it->name);
		
		

		bool done = false;
		if(as->getLength() < as->getTimePosition()+deltaT){
			//done = true;
		}

		if(done && it->hold){
						
			//as->setTimePosition(as->getLength()-.00000001);
		}
		else if(done){
			it->erase = true;
		}

		if(!done){
			as->addTime(deltaT);
			if(!it->update){
				as->addTime(-deltaT);
			}
		}
	}

	//go through and stop and erase all of the animations that are beyond their time
	bool changes = false;
	while(true){
		changes = false;
		for(it = animations.begin();it!=animations.end();it++){
			if(it->erase){
				
				Ogre::AnimationState * as = ass->getAnimationState(it->name);
				Ogre::Real weight = as->getWeight();
				if(weight <= 0 && !it->loop){
					cout << "stopping animation " << it->name << endl;
					as->setEnabled(false);
					as->setTimePosition(0.0);
					animations.erase(it);
					changes = true;
					break;
				}
			}
		}
		if(!changes){
			break;
		}
	}
}