#ifndef NBOwnTLDef_h
#define NBOwnTLDef_h

#include <vector>
#include <set>
#include "NBTrafficLightDefinition.h"

class NBNode;

class NBOwnTLDef : public NBTrafficLightDefinition {
public:
    /// Constructor
    NBOwnTLDef(const std::string &id,
        const std::set<NBNode*> &junctions);

    /// Constructor
    NBOwnTLDef(const std::string &id, NBNode *junction);

    /// Constructor
    NBOwnTLDef(const std::string &id, std::string type,
        NBNode *junction);

    /// Constructor
    NBOwnTLDef(const std::string &id);

    /// Destructor
    ~NBOwnTLDef();

    /** possible types of removeing a link from regardation during the
        building of the traffic light logic */
    enum LinkRemovalType {
        /// all links will be regarded
        LRT_NO_REMOVAL,
        /** all left-movers which are together with other direction on the same
            lane will be removed */
        LRT_REMOVE_WHEN_NOT_OWN,
        /// remove all left-movers
        LRT_REMOVE_ALL_LEFT
    };

    void setParticipantsInformation();

public:
    void remapRemoved(NBEdge *removed,
        const EdgeVector &incoming, const EdgeVector &outgoing);

protected:
    /// Computes the traffic light logic
    NBTrafficLightLogicVector *myCompute(size_t breakingTime,
        std::string type, bool buildAll);

    /// Collects the nodes participating in this traffic light
    void collectNodes();

    void collectLinks();

    void replaceRemoved(NBEdge *removed, int removedLane,
        NBEdge *by, int byLane);

    void setTLControllingInformation() const;


private:
    /** compute the traffic light logics for the current node and the
        given settings */
    NBTrafficLightLogicVector *computeTrafficLightLogics(
        const std::string &key, std::string type,
        bool joinLaneLinks, bool removeTurnArounds, LinkRemovalType removal,
        bool appendSmallestOnly, bool skipLarger,
        size_t breakingTime) const;

    /** compute the pases for the current node and the given settings */
/*    NBTrafficLightPhases * computePhases(bool joinLaneLinks,
        bool removeTurnArounds, LinkRemovalType removal,
        bool appendSmallestOnly, bool skipLarger) const;*/

    /** build the matrix of links that may be used simultaneously */
    NBLinkPossibilityMatrix *getPossibilityMatrix(bool joinLanes,
        bool removeTurnArounds, LinkRemovalType removalType) const;

};

#endif
