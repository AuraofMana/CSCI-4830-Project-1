#include <btBulletDynamicsCommon.h>

class KinematicMotionState : public btMotionState {
public:
    KinematicMotionState(const btTransform &initialpos) {
        mPos1 = initialpos;
    }

    virtual ~ KinematicMotionState() {
    }

    /*void setNode(Ogre::SceneNode *node) {
        //mVisibleobj = node;
    }*/

    virtual void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = mPos1;
    }

    void setKinematicPos(btTransform &currentPos) {
        mPos1 = currentPos;
    }

    virtual void setWorldTransform(const btTransform &worldTrans) {
    }

protected:
    btTransform mPos1;
};