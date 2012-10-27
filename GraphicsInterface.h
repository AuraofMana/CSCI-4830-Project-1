#pragma once
#include <Ogre.h>
#include <string>
using namespace Ogre;
using namespace std;

class GraphicsInterface
{
private:
	Ogre::Root *root;
	std::map<std::string,Ogre::RenderWindow*> render_windows;
	Ogre::SceneNode *root_sn;
	Ogre::Camera *c;
	Ogre::SceneNode *c_sn;
	Ogre::RenderWindow *window;
	Ogre::SceneManager *manager;

public:
	//GraphicsInterface(Ogre::Root *ogreRoot);
	Ogre::Root* init();
	Ogre::RenderWindow* GetWindow(string name);
	void RenderFrame(Ogre::Real timeSinceLastFrame);
	void MoveCameraForward(Ogre::Vector3 vec, Ogre::Real meters);
	void SetUpCamera();
	Ogre::SceneNode *GetRootSceneNode();
	Ogre::SceneManager *GetManager();
};