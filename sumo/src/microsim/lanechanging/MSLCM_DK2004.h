#ifndef MSLCM_DK2004_h
#define MSLCM_DK2004_h

#include <microsim/MSAbstractLaneChangeModel.h>
#include <microsim/MSVehicleQuitReminded.h>
#include <vector>

enum MyLCAEnum {
    LCA_AMBLOCKINGLEADER = 256, // 0
    LCA_AMBLOCKINGFOLLOWER = 512,// 1
    LCA_MRIGHT = 1024, // 2
    LCA_MLEFT = 2048,// 3
    LCA_UNBLOCK = 4096,// 4
    LCA_AMBLOCKINGFOLLOWER_DONTBRAKE = 8192,// 5
    LCA_AMBLOCKINGSECONDFOLLOWER = 16384, // 6
    LCA_KEEP1 = 65536,// 8
    LCA_KEEP2 = 131072,// 9
    LCA_AMBACKBLOCKER = 262144,// 10
    LCA_AMBACKBLOCKER_STANDING = 524288,// 11

};

class MSLCM_DK2004 : public MSAbstractLaneChangeModel {
public:
    MSLCM_DK2004(MSVehicle &v);

    virtual ~MSLCM_DK2004();

    /** @brief Called to examine whether the vehicle wants to change to right
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToRight(
        MSAbstractLaneChangeModel::MSLCMessager &msgPass, int blocked,
        const std::pair<MSVehicle*, double> &leader,
        const std::pair<MSVehicle*, double> &neighLead,
        const std::pair<MSVehicle*, double> &neighFollow,
        const MSLane &neighLane,
        int bestLaneOffset, double bestDist, double neighDist,
        double currentDist,
        MSVehicle **lastBlocked);

    /** @brief Called to examine whether the vehicle wants to change to left
        This method gets the information about the surrounding vehicles
        and whether another lane may be more preferable */
    virtual int wantsChangeToLeft(
        MSAbstractLaneChangeModel::MSLCMessager &msgPass, int blocked,
        const std::pair<MSVehicle*, double> &leader,
        const std::pair<MSVehicle*, double> &neighLead,
        const std::pair<MSVehicle*, double> &neighFollow,
        const MSLane &neighLane,
//        bool congested, bool predInteraction,
        int bestLaneOffset, double bestDist, double neighDist,
        double currentDist,
        MSVehicle **lastBlocked);

    virtual void *inform(void *info, MSVehicle *sender);

    virtual double patchSpeed(double min, double wanted, double max,
        double vsafe);

    virtual void changed();

    double getProb() const;
    virtual void prepareStep();


protected:
    /*
    void unblockLeader();
    void unblockFollower();
    */
    void informBlocker(MSAbstractLaneChangeModel::MSLCMessager &msgPass,
        int &blocked, int dir,
        const std::pair<MSVehicle*, double> &neighLead,
        const std::pair<MSVehicle*, double> &neighFollow);

//    void addToBlocking(MSVehicle *veh);

    inline bool amBlockingLeader() { return (myState&LCA_AMBLOCKINGLEADER)!=0; }
    inline bool amBlockingFollower() { return (myState&LCA_AMBLOCKINGFOLLOWER)!=0; }
    inline bool amBlockingFollowerNB() { return (myState&LCA_AMBLOCKINGFOLLOWER_DONTBRAKE)!=0; }
    inline bool amBlockingFollowerPlusNB() { return (myState&(LCA_AMBLOCKINGFOLLOWER|LCA_AMBLOCKINGFOLLOWER_DONTBRAKE))!=0; }
    inline bool currentDistDisallows(double dist, int laneOffset, double lookForwardDist) {
        return dist/abs(laneOffset)<lookForwardDist;
    }
    inline bool currentDistAllows(double dist, int laneOffset, double lookForwardDist) {
        return dist/abs(laneOffset)>lookForwardDist;
    }

    typedef std::pair<double, int> Info;

    void setBlockingFollower(MSVehicle *v);


protected:
    float myChangeProbability;
    double myVSafe;
};


#endif
