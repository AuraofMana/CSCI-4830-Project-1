#include <btBulletDynamicsCommon.h>
#include <ogre.h>
#include "FAASTClient.h"
#include "PhysicsInterface.h"
#include "GraphicsInterface.h";
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include "KinematicMotionState.h"
#include "BtOgrePG.h"
#include "BtOgreGP.h"
#include "BtOgreExtras.h"
#include "DotSceneLoader.h"
#include "GameTimer.h"
#include "AnimationManager.h"
#include "OpenALSoundSystem.h"

using namespace Ogre;
using namespace std;

const float FIRE_WAIT_TIME = 250; //0.25 second
float fireTimer = FIRE_WAIT_TIME;
SceneNode * projectile_sn = NULL;
const int MOVEMENT_SPEED = 5000;
const int JETPACK_SPEED = 5000;
const int CAMERA_DEGREE = 5;
const int SOUND_FADE = 300;
const float ACC_GRAVITY = 9.8;

void configureChildren(Ogre::SceneNode *child, btDiscreteDynamicsWorld* dynamicsWorld, bool shadows = false)
{
	for(int i = 0; i < child->numChildren(); i++)
		configureChildren((SceneNode*)child->getChild(i), dynamicsWorld);

	for(int i = 0; i < child->numAttachedObjects(); i++)
	{
		Entity *ent = (Entity*)child->getAttachedObject(i);
		ent->setCastShadows(shadows);
		//ent->setVisible(false);

		if(dynamicsWorld != NULL)
		{
			BtOgre::StaticMeshToShapeConverter converter(ent, child->_getFullTransform());
			btCollisionShape *shp = converter.createConvex();
			btMotionState* st = new BtOgre::RigidBodyState(child, btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)), btTransform(btQuaternion(0,0,0,1),BtOgre::Convert::toBullet(-converter.getCenter())));
			btRigidBody *body = new btRigidBody(0, st, shp, btVector3(0,0,0));
			dynamicsWorld->addRigidBody(body);
		}
	}
}

void loadMeshes(Ogre::SceneNode *sn, string FolderName, Ogre::SceneManager *man, Ogre::Real scale = .1f, btDiscreteDynamicsWorld* dynamicsWorld = NULL, bool shadows = false, Ogre::String prepend = "")
{
	if(prepend == "")
		prepend = FolderName;
	sn->setScale(scale,scale,scale);

	DotSceneLoader loader;
	loader.parseDotScene(FolderName + ".xml", "General", man, sn, prepend);
	
	configureChildren(sn, dynamicsWorld, shadows);

}

int KinectToRobotMap [18];

void UpdateRobotJoints(Ogre::Entity *robot, FAASTClient kinect)
{
	KinectToRobotMap[0] = -1;
	KinectToRobotMap[1] = -1;
	KinectToRobotMap[2] = -1;
	KinectToRobotMap[3] = -1;
	KinectToRobotMap[4] = -1;
	KinectToRobotMap[5] = -1;
	KinectToRobotMap[6] = -1;
	KinectToRobotMap[7] = -1;
	KinectToRobotMap[8] = -1;
	KinectToRobotMap[9] = -1;
	KinectToRobotMap[10] = -1;
	KinectToRobotMap[11] = -1;
	KinectToRobotMap[12] = -1;
	KinectToRobotMap[13] = 6;
	KinectToRobotMap[14] = -1;
	KinectToRobotMap[15] = -1;
	KinectToRobotMap[16] = -1;
	KinectToRobotMap[17] = -1;

	Ogre::Skeleton *skeleton = robot->getSkeleton();
	for(int i = 0; i < skeleton->getNumBones(); i++)
	{
		if(KinectToRobotMap[i] < 0) continue;
		Ogre::Bone *bone = skeleton->getBone(i);
		bone->setManuallyControlled(true);
		Quaternion origOri = bone->getOrientation();
		FAASTJoint joint = kinect.getJoint(KinectToRobotMap[i]);
		//Ogre::Quaternion jointOri(.5,0,0,.5);
		//Ogre::Quaternion jointOri(joint.ori[0], -joint.ori[1], 0, joint.ori[3]);
		//bone->setOrientation(jointOri);
	}
}

btRigidBody* AddPhysicsToEntity(Ogre::SceneNode* sn, Ogre::Entity *ent, btDiscreteDynamicsWorld *dynamicsWorld)
{
	//building_sn->attachObject(ent);
	//building_sn->setVisible(false);
	BtOgre::StaticMeshToShapeConverter converter(ent, sn->_getFullTransform());
	btCollisionShape *shp = converter.createConvex();
	btMotionState* st = new BtOgre::RigidBodyState(sn, btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)), btTransform(btQuaternion(0,0,0,1),BtOgre::Convert::toBullet(-converter.getCenter())));
	btRigidBody *body = new btRigidBody(0, st, shp, btVector3(0,0,0));
	dynamicsWorld->addRigidBody(body);

	return body;
}

int main()
{
	std::list<btRigidBody*> PhysicsObjects;
	std::list<SceneNode*> OgreObjects;

	Ogre::Root *root;
	btDiscreteDynamicsWorld* dynamicsWorld;

	btRigidBody *rightArm;
	btRigidBody *leftArm;
	btRigidBody *player;

	Ogre::SceneNode *rightArmOgre;
	Ogre::SceneNode *leftArmOgre;

	FAASTClient kinect;

	BtOgre::DebugDrawer* dbgdraw;

	OpenALSoundSystem sound;

	//Set up OpenAL
	sound.init();

	//Set up physics
	PhysicsInterface physInterface;
	dynamicsWorld = physInterface.init();
	dynamicsWorld->setGravity(btVector3(0, -ACC_GRAVITY, 0));

	btCollisionShape *armShape = new btSphereShape(.2);
	KinematicMotionState *rArmMotionState = new KinematicMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0, 2, 0)));
	btRigidBody::btRigidBodyConstructionInfo rArmRigidBodyCI(0, rArmMotionState, armShape, btVector3(0,0,0));
	rightArm = new btRigidBody(rArmRigidBodyCI);
	//Make the arm kinematic
	rightArm->setCollisionFlags(rightArm->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	rightArm->setActivationState(DISABLE_DEACTIVATION);
	dynamicsWorld->addRigidBody(rightArm);
	player = physInterface.initPlayer();

	//Set up OGRE
	GraphicsInterface graphInterface;
	root = graphInterface.init();
	Ogre::RenderWindow *ogreWindow = graphInterface.GetWindow("Project 1");
	graphInterface.SetUpCamera();
	graphInterface.GetManager()->setSkyDome(true, "Skybox");
	//graphInterface.GetManager()->setSkyPlane(true, Ogre::Plane(Vector3::UNIT_Y, 10), "Ocean", 15000, 1000);
	loadMeshes(graphInterface.GetRootSceneNode()->createChildSceneNode("CityNode"), "City", graphInterface.GetManager(), .1, dynamicsWorld, true);
	SceneNode *acc = graphInterface.GetRootSceneNode()->createChildSceneNode("ACCNode");
	acc->setPosition(0, 500, 0);
	loadMeshes(acc, "ACC", graphInterface.GetManager(), 15, dynamicsWorld, true);
	sound.createSound("Sounds/Boeing_737.wav", "Boeing");
	double spawnPos[3] = {0, 500 / SOUND_FADE, 0};
	double spawnVel[3] = {0, 0, 0};
	sound.createSource("Spawn", spawnPos, spawnVel);
	sound.assignSourceSound("Spawn", "Boeing", 1, 1, 1);
	sound.playSound("Spawn");

	//Escort1
	SceneNode *acc2 = graphInterface.GetRootSceneNode()->createChildSceneNode("ACCNode2");
	acc2->setPosition(-1000, 250, 60);
	loadMeshes(acc2, "ACC", graphInterface.GetManager(), 15, NULL, true, "ACC2");
	sound.createSource("Escort1", spawnPos, spawnVel);
	sound.assignSourceSound("Escort1", "Boeing", 1, 1, 1);
	sound.playSound("Escort1");

	//Escort2
	SceneNode *acc3 = graphInterface.GetRootSceneNode()->createChildSceneNode("ACCNode3");
	acc3->setPosition(-2080, 250, -1950);
	acc3->rotate(Vector3::UNIT_Y, Degree(-90));
	loadMeshes(acc3, "ACC", graphInterface.GetManager(), 15, NULL, true, "ACC3");
	sound.createSource("Escort2", spawnPos, spawnVel);
	sound.assignSourceSound("Escort2", "Boeing", 1, 1, 1);
	sound.playSound("Escort2");

	//Load our mesh
	SceneNode *OurMesh = graphInterface.GetRootSceneNode()->createChildSceneNode("OurMesh");
	Entity *OurEnt = graphInterface.GetManager()->createEntity("OurMesh", "OurMesh.mesh");
	OurMesh->attachObject(OurEnt);

	//Set up debug boxes
	dbgdraw = new BtOgre::DebugDrawer(graphInterface.GetRootSceneNode(), dynamicsWorld);
	dynamicsWorld->setDebugDrawer(dbgdraw);

	//Create floor
	/*Ogre::Entity *ground = root->getSceneManager("main")->createEntity("ground", "cube.mesh");
	ground->setMaterialName("Ocean");
	ground->setCastShadows(false);
	Ogre::SceneNode *ground_sn = graphInterface.GetRootSceneNode()->createChildSceneNode("ground");
	ground_sn->scale(1000, .001, 1000);
	ground_sn->attachObject(ground);
	ground_sn->setPosition(0,-50,0);*/

	btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0,1,0), 1);
	btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,-1,0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0,0,0));
	btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);
	groundRigidBody->setRestitution(.5f);
	dynamicsWorld->addRigidBody(groundRigidBody);

	//Create Robot
	Ogre::SceneNode *robot_sn = graphInterface.GetRootSceneNode()->createChildSceneNode("robot");
	Ogre::Entity *robot = root->getSceneManager("main")->createEntity("robot", "robot.mesh");
	robot_sn->setPosition(-1235, 0, -300);
	robot_sn->attachObject(robot);
	robot_sn->setScale(.1, .1, .1);
	robot_sn->rotate(Vector3::UNIT_Y, Degree(-90));
	btRigidBody* robotBody = AddPhysicsToEntity(robot_sn, robot, dynamicsWorld);
	AnimationManager *am = new AnimationManager(robot);
	am->playAnimation("Walk", true, false, true, 1.0);

	//Create Arms
	Ogre::SceneNode* playerNode = (Ogre::SceneNode*)graphInterface.GetRootSceneNode()->getChild("Player");
	Ogre::SceneNode* mainCamera = (Ogre::SceneNode*)playerNode->getChild("main_camera");
	Ogre::SceneNode *cameraPitch = (Ogre::SceneNode*)mainCamera->getChild("cameraPitch");
	/*Ogre::Entity *rArmEntity = root->getSceneManager("main")->createEntity("arm", "sphere.mesh");
	rightArmOgre = mainCamera->createChildSceneNode("rightArm");
	rightArmOgre->attachObject(rArmEntity);
	rightArmOgre->scale(.01, .01, .01);
	rightArmOgre->setPosition(0, 2, -5);
	rightArmOgre->setVisible(false);*/

	//Add head light
	Ogre::Light *headLight = graphInterface.GetManager()->createLight("headLight");
	headLight->setType(Ogre::Light::LT_SPOTLIGHT);
	headLight->setDirection(Ogre::Vector3(0,0,-1));
	cameraPitch->attachObject(headLight);

	//Add lights to escorts
	Ogre::Light *acc2Light = graphInterface.GetManager()->createLight("acc2Light");
	acc2Light->setType(Ogre::Light::LT_SPOTLIGHT);
	acc2Light->setDirection(-Ogre::Vector3::UNIT_Y);
	acc2Light->setSpotlightFalloff(20);
	acc2->attachObject(acc2Light);

	//Add lights to escorts
	Ogre::Light *acc3Light = graphInterface.GetManager()->createLight("acc3Light");
	acc3Light->setType(Ogre::Light::LT_SPOTLIGHT);
	acc3Light->setDirection(-Ogre::Vector3::UNIT_Y);
	acc3->attachObject(acc3Light);

	//Add point light
	/*Ogre::Light *sunLight = graphInterface.GetManager()->createLight("sunLight");
	sunLight->setType(Ogre::Light::LT_DIRECTIONAL);
	sunLight->setPosition(0,100, 10000);
	graphInterface.GetRootSceneNode()->attachObject(sunLight);*/

	//Add some blocks
	btCollisionShape *blockShape = new btBoxShape(btVector3(1,.5,1));
	btVector3 fallInertia(0,0,0);

	/*for(int i = 0; i < 1000; i++)//physics boxes
	{
		btDefaultMotionState *boxMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(i / 10 * 2 + (i % 2) - i / 500 * 100, i % 10 * 1 + 1, 5 + i / 500 * 2)));
		btScalar boxMass = 100;
		blockShape->calculateLocalInertia(boxMass, fallInertia);
		btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(boxMass, boxMotionState, blockShape, fallInertia);
		btRigidBody *newBox = new btRigidBody(boxRigidBodyCI);
		newBox->setRestitution(.5f);

		dynamicsWorld->addRigidBody(newBox);
		PhysicsObjects.push_back(newBox);
	}

	for(int i = 0; i < PhysicsObjects.size(); i++)//ogre boxes
	{
		Ogre::SceneNode *root_sn = graphInterface.GetRootSceneNode();
		Ogre::SceneManager *manager = graphInterface.GetManager();
		char name[10];
		sprintf(name, "box%d", i);
		Ogre::Entity *boxO = manager->createEntity(name, "cube.mesh");
		//boxO->setMaterialName("BeachStones");
		SceneNode * box_sn = root_sn->createChildSceneNode(name);
		box_sn->scale(.02, .01, .02);
		box_sn->attachObject(boxO);
		boxO->setCastShadows(true);
		boxO->setMaterialName("M_NoLighting");

		//boxesO[i] = box_sn;
		OgreObjects.push_back(box_sn);
	}*/

	//Set up Kinect
	kinect.init("Tracker0", "localhost", 3883);

	//Allow for keyboard control
	size_t hWnd = 0;
	ogreWindow->getCustomAttribute("WINDOW", &hWnd);
	OIS::InputManager *m_InputManager = OIS::InputManager::createInputSystem(hWnd);
	OIS::Keyboard *m_Keyboard = static_cast<OIS::Keyboard*>(m_InputManager->createInputObject(OIS::OISKeyboard, false));

	int projectileNum = 0;

	//Main loop
	GameTimer timer;
	bool escort1MovingUp = true;
	int escort1Turning = 0;
	bool escort2MovingUp = true;
	int escort2Turning = 0;
	bool robotMovingUp = true;
	int robotTurning = 0;
	while(1)
	{
		//double elapsed = timer.getElapsedTimeSec();
		double elapsed = 1/60.f;
		graphInterface.RenderFrame(elapsed);//Draw frames
		Ogre::WindowEventUtilities::messagePump();
		dynamicsWorld->stepSimulation(elapsed, 10);//Do physics
		//dbgdraw->step();
		sound.update();

		am->update(elapsed);

		kinect.update();
		Ogre::Vector3 movement(0,0,0);
		btVector3 playerForce(0,0,0);

		//Keyboard check
		m_Keyboard->capture();
		if(m_Keyboard->isKeyDown(OIS::KC_W))
			playerForce.setZ(-MOVEMENT_SPEED);
		if(m_Keyboard->isKeyDown(OIS::KC_S))
			playerForce.setZ(MOVEMENT_SPEED);
		if(m_Keyboard->isKeyDown(OIS::KC_A))
			playerForce.setX(-MOVEMENT_SPEED);
		if(m_Keyboard->isKeyDown(OIS::KC_D))
			playerForce.setX(MOVEMENT_SPEED);
		if(m_Keyboard->isKeyDown(OIS::KC_UP))
			cameraPitch->pitch(Degree(CAMERA_DEGREE));
		if(m_Keyboard->isKeyDown(OIS::KC_DOWN))
			cameraPitch->pitch(Degree(-CAMERA_DEGREE));
		if(m_Keyboard->isKeyDown(OIS::KC_RCONTROL))
			cout << playerNode->getPosition() << endl;

		//Move the player forward, or cancel any forces
		if(playerForce.length() > 10)
		{
			Ogre::Vector3 playerVec(playerForce.x(), playerForce.y(), playerForce.z());
			playerVec = mainCamera->getOrientation() * playerVec;
			playerForce = btVector3(playerVec.x, playerVec.y, playerVec.z);
			player->applyCentralForce(playerForce);
		}
		else
		{
			btVector3 linearVel = player->getLinearVelocity();
			linearVel.setX(linearVel.x() * .7);
			linearVel.setZ(linearVel.z() * .7);
			player->setLinearVelocity(linearVel);
		}

		//Turn on/off the player head light
		if(m_Keyboard->isKeyDown(OIS::KC_F))
			headLight->setVisible(false);
		if(m_Keyboard->isKeyDown(OIS::KC_G))
			headLight->setVisible(true);

		//Move the first search plane
		Vector3 acc2Translate( escort1MovingUp ? 2 : -2, 0, 0);
		if(escort1Turning == 0) acc2->translate(acc2Translate);
		else
		{
			acc2->rotate(Vector3::UNIT_Y, Degree(1));
			escort1Turning--;
		}
		if(escort1MovingUp && acc2->getPosition().x > 500 && escort1Turning == 0)
		{
			escort1MovingUp = false;
			escort1Turning = 180;
		}
		else if(!escort1MovingUp && acc2->getPosition().x < -1000 && escort1Turning == 0)
		{
			escort1MovingUp = true;
			escort1Turning = 180;
		}

		//Move the second search plane
		Vector3 acc3Translate(0, 0, escort2MovingUp ? 2 : -2);
		if(escort2Turning == 0) acc3->translate(acc3Translate);
		else
		{
			acc3->rotate(Vector3::UNIT_Y, Degree(1));
			escort2Turning--;
		}
		if(escort2MovingUp && acc3->getPosition().z > 1080 && escort2Turning == 0)
		{
			escort2MovingUp = false;
			escort2Turning = 180;
		}
		else if(!escort2MovingUp && acc3->getPosition().z < -1950 && escort2Turning == 0)
		{
			escort2MovingUp = true;
			escort2Turning = 180;
		}

		//Move robot
		Vector3 robotTranslate(0, 0, robotMovingUp ? .2 : -.2);
		if(robotTurning == 0)
		{
				robot_sn->translate(robotTranslate);
				robotBody->translate(btVector3(robotTranslate.x, robotTranslate.y, robotTranslate.z));
		}
		else
		{
			robot_sn->rotate(Vector3::UNIT_Y, Degree(1));
			robotTurning--;
			if(robotTurning == 0)
				am->playAnimation("Walk", true, false, true, 1.0);
		}
		if(robotMovingUp && robot_sn->getPosition().z > -300 && robotTurning == 0)
		{
			robotMovingUp = false;
			robotTurning = 180;
			am->stopAnimation("Walk");
		}
		else if(!robotMovingUp && robot_sn->getPosition().z < -1560 && robotTurning == 0)
		{
			robotMovingUp = true;
			robotTurning = 180;
			am->stopAnimation("Walk");
		}

		if(m_Keyboard->isKeyDown(OIS::KC_LEFT))//rotate left
		{
			Ogre::Quaternion quat(Ogre::Degree(2), Ogre::Vector3::UNIT_Y);
			mainCamera->rotate(quat);
		}
		if(m_Keyboard->isKeyDown(OIS::KC_RIGHT))//rotate right
		{
			Ogre::Quaternion quat(Ogre::Degree(-2), Ogre::Vector3::UNIT_Y);
			mainCamera->rotate(quat);
		}
		if(fireTimer > FIRE_WAIT_TIME)
		{
			if(m_Keyboard->isKeyDown(OIS::KC_RETURN))//Fire projectile
			{
				//Find the orientation of the camera, set it up as a direction
				Ogre::Quaternion q = mainCamera->getOrientation(); //Get the quaternion from the camera

				//Spawn a projectile object facing that direction (+1 unit from where you are facing) away from the right arm
				Ogre::Vector3 v3One(0, 0, -1);
				v3One = mainCamera->_getDerivedOrientation() * cameraPitch->_getDerivedOrientation() * v3One + mainCamera->_getDerivedPosition();
				btDefaultMotionState *projectileMotionState = new btDefaultMotionState(btTransform(btQuaternion(q.w, q.x, q.y, q.z), btVector3(v3One.x, v3One.y, v3One.z)));
				btScalar boxMass = 10;
				blockShape->calculateLocalInertia(boxMass, fallInertia);
				btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(boxMass, projectileMotionState, blockShape, fallInertia);
				btRigidBody *newBox = new btRigidBody(boxRigidBodyCI);
				newBox->setRestitution(.9f);
				
				Ogre::Vector3 projectileTemp(q * cameraPitch->getOrientation() * Ogre::Vector3::NEGATIVE_UNIT_Z * 200);
				btVector3 projectileVelocity(projectileTemp.x, projectileTemp.y, projectileTemp.z);
				newBox->setLinearVelocity(projectileVelocity);

				dynamicsWorld->addRigidBody(newBox);
				PhysicsObjects.push_back(newBox);

				Ogre::SceneNode *root_sn = graphInterface.GetRootSceneNode();
				Ogre::SceneManager *manager = graphInterface.GetManager();
				char name[20];
				sprintf(name, "projectile%d", projectileNum);
				++projectileNum;
				Ogre::Entity *projectileO = manager->createEntity(name, "cube.mesh");
				projectile_sn = root_sn->createChildSceneNode(name);
				projectile_sn->scale(.0025, .0025, .0025);
				projectile_sn->attachObject(projectileO);
				projectileO->setCastShadows(true);
				projectileO->setMaterialName("M_NoLighting");

				OgreObjects.push_back(projectile_sn);

				fireTimer -= FIRE_WAIT_TIME;
			}
		}
		else
		{
			fireTimer += 16;
		}

		btTransform trans;
		player->getMotionState()->getWorldTransform(trans);
		btVector3 playerOrig = trans.getOrigin();

		if(m_Keyboard->isKeyDown(OIS::KC_SPACE))//Jump/Rocket
		{
			player->applyCentralForce(btVector3(0,JETPACK_SPEED,0));
		}
		if(m_Keyboard->isKeyDown(OIS::KC_LCONTROL))
			player->applyCentralForce(btVector3(0,-JETPACK_SPEED,0));

		//Update sound//

		//Player listener location
		Vector3 globalPos = playerNode->_getDerivedPosition() / SOUND_FADE;
		btVector3 playerVel = player->getLinearVelocity() / SOUND_FADE;
		double pos[3] = {globalPos.x, globalPos.y, globalPos.z};
		double vel[3] = {playerVel.x(), playerVel.y(), playerVel.z()};
		Vector3 forwardVec = cameraPitch->_getDerivedOrientation() * Vector3::NEGATIVE_UNIT_Z;
		double forward[3] = {forwardVec.x, forwardVec.y, forwardVec.z};
		double up[3] = {0,1,0};
		sound.setListenerData(pos, vel, forward, up);

		//Escort1 Position
		Vector3 escort1Global = acc2->_getDerivedPosition() / SOUND_FADE;
		double esc1Pos[3] = {escort1Global.x, escort1Global.y, escort1Global.z};
		double esc1Vel[3] = { escort1MovingUp ? 2 : -2, 0, 0};
		sound.setSourceData("Escort1", esc1Pos, esc1Vel);

		//Escort2 Position
		Vector3 escort2Global = acc3->_getDerivedPosition() / SOUND_FADE;
		double esc2Pos[3] = {escort2Global.x, escort2Global.y, escort2Global.z};
		double esc2Vel[3] = {0, 0, escort1MovingUp ? 2 : -2};
		sound.setSourceData("Escort2", esc2Pos, esc2Vel);

		//Update Kinect data
		FAASTJoint headJoint = kinect.getJoint(0);
		FAASTJoint rightHand = kinect.getJoint(8);

		Ogre::Vector3 headVector(headJoint.pos[0], headJoint.pos[1], headJoint.pos[2]);
		Ogre::Vector3 rightHandVector(rightHand.pos[0], rightHand.pos[1], rightHand.pos[2]);
		Ogre::Vector3 relativePosition = headVector - rightHandVector;
		relativePosition.x *= 5;
		relativePosition.z *= -5;//The kinect uses a different Z
		relativePosition.y *= -5;//The kinect uses a different Y
		//rightArmOgre->setPosition(relativePosition);

		//btTransform rArmTrans;
		//rightArm->getMotionState()->getWorldTransform(rArmTrans);
		//cout << rArmTrans.getOrigin().x() << endl;
		
		//Ogre::Vector3 rightArmGlobal = rightArmOgre->_getDerivedPosition();
		//btVector3 rightArmGlobalBT = btVector3(rightArmGlobal.x, rightArmGlobal.y, rightArmGlobal.z);

		//Update the kinematic arms
		//KinematicMotionState *state = (KinematicMotionState*)rightArm->getMotionState();
		//state->setKinematicPos(btTransform(btQuaternion(0,0,0,1), rightArmGlobalBT));

		//UpdateRobotJoints(robot, kinect);

		//Update Ogre based on physics
		std::list<btRigidBody*>::iterator Pit;
		Pit = PhysicsObjects.begin();
		std::list<SceneNode*>::iterator Oit;
		Oit = OgreObjects.begin();
		for(int i = 0; i < PhysicsObjects.size(); i++, Pit++, Oit++)
		{
			btTransform tr;
			(*Pit)->getMotionState()->getWorldTransform(tr);

			(*Oit)->setPosition(tr.getOrigin().getX(), tr.getOrigin().getY(), tr.getOrigin().getZ());
			(*Oit)->setOrientation(tr.getRotation().getW(), tr.getRotation().getX(), tr.getRotation().getY(), tr.getRotation().getZ());
		}
		graphInterface.MoveCameraForward(movement, 1.f);

		//Move player based on physics
		playerNode->setPosition(playerOrig.x(), playerOrig.y(), playerOrig.z());

		//if(elapsed < 16)
			Sleep(16);// - elapsed);
	}

}