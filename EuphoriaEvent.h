/*********************************************************\
EuphoriaEvent.h
\*********************************************************/

#ifndef EUPHORIA_EVENT_H
#define EUPHORIA_EVENT_H

#include "BehaviorSklMgr.h"
#include "LECAlign.h"

namespace Ronin
{
    // FORWARD DECLARATIONS
    class ParameterOverrideDataInterface;
}

namespace RoninPhysics
{
    // FORWARD DECLARATIONS
    struct CollisionData;
}

namespace euphoria
{
    /*!
     * The EuphoriaEvent base class. All events handled by Euphoria Performances
     * must be derived from this class.
     */
    DECLARE_ALIGNED	class EuphoriaEvent
    {
        LECALIGNEDCLASS(16);
        
    public:
        // ENUMERATIONS
        enum Type
        {
            eInvalid,
            // Animation events
            eAnimationEnd,
            // Behavior events
            eBehaviorLeftArmReached,
            eBehaviorRightArmReached,
            eBehaviorRelaxed,
            eBehaviorDefend,
            eBehaviorFlail,
            eBehaviorStaggerBalanced,
            eBehaviorStaggerBalancedFeetOnGround,
            eBehaviorStaggerTrip,
            eBehaviorStaggerMaxSteps,
            eBehaviorStaggerStepping,
            eBehaviorStaggerStartedFalling,
            eBehaviorStaggerStoppedFalling,
            eBehaviorCollision,
            eBehaviorTumble,
            eBehaviorConstrainLeftHand,
            eBehaviorConstrainRightHand,
            eBehaviorUnConstrainLeftHand,
            eBehaviorUnConstrainRightHand,
            eBehaviorHangFall,
            eBehaviorMotionTransferIn,
            eBehaviorMotionTransferOut,
			eBehaviorBlendFrameDone,
        };

    private:
        // MEMBERS
        const Type mType;

    protected:
        // CREATORS
        // Hidden - For derived class use ONLY
        explicit EuphoriaEvent(Type type) : mType(type) {}

    public:
        // CREATORS
        virtual ~EuphoriaEvent() { /* does nothing */ }

        // ACCESSORS
        const Type GetType() const { return mType; }

    } END_DECLARE_ALIGNED;

    /*!
    * This EuphoriaEvent subclass is used to wrap events coming from the Animation
    * System. These events come from animation Chore Blocks as McGuffin events.
     */
    class EuphoriaAnimationEvent : public EuphoriaEvent
    {
    public:
        // CREATORS
        explicit EuphoriaAnimationEvent(Type type) : EuphoriaEvent(type) {}
        ~EuphoriaAnimationEvent() {}
    };

    /*!
     * This EuphoriaEvent subclass is used to wrap events coming from the Behavior
     * Skeleton Manager. These events originate from within the Euphoria behaviors
     * themselves.
     */
    class EuphoriaBehaviorEvent : public EuphoriaEvent
    {
    protected:
        // MEMBERS
        const Ronin::ParameterOverrideDataInterface* mpData;
        const RoninPhysics::CollisionData* mpCollisionData;

    public:
        // CREATORS
        explicit EuphoriaBehaviorEvent(Type type, const Ronin::ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData) : EuphoriaEvent(type), mpData(pData), mpCollisionData(pCollisionData) {}
        ~EuphoriaBehaviorEvent() {}
    };

    // FREE FUNCTION DECLARATIONS
    EuphoriaBehaviorEvent CreateEuphoriaBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, Ronin::ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData);
    EuphoriaAnimationEvent CreateEuphoriaAnimationEvent(crc32_t eventNameCRC);
}

// EUPHORIA_EVENT_H
#endif
