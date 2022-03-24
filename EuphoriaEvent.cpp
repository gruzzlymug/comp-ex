/*********************************************************\
EuphoriaEvent.cpp
\*********************************************************/

#include "EuphoriaEvent.h"
#include "LECLog.h"

namespace euphoria
{
    // USING DIRECTIVES
    using Ronin::ParameterOverrideDataInterface;
    using RoninPhysics::CollisionData;
    using namespace lec;

    // LOCAL CONST INITIALIZATION
    namespace
    {
        const crc32_t kEndCRC = CRCINIT("END");
    }

    // FREE FUNCTIONS
    /*!
     * This is a utility function that doesn't need to be a part of any particular
     * class.
     *
     * Creates an animation event out of a CRC for use by the performances.
     */
    EuphoriaAnimationEvent CreateEuphoriaAnimationEvent(crc32_t eventNameCRC)
    {
        if (eventNameCRC == kEndCRC)
        {
            return EuphoriaAnimationEvent(EuphoriaEvent::eAnimationEnd);
        }

        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "WARNING: Unknown Animation Event: %d\n", static_cast<int>(eventNameCRC));
        return EuphoriaAnimationEvent(EuphoriaEvent::eInvalid);
    }

    /*!
     * This is a utility function that doesn't need to be a part of any particular
     * class.
     *
     * Converts the various parts of a Behavior Event into a type handled by the
     * performances.
     */
    EuphoriaBehaviorEvent CreateEuphoriaBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData)
    {
        switch (eventType)
        {
        case BehaviorFeedback::BET_LeftArmReached:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorLeftArmReached, pData, pCollisionData);

        case BehaviorFeedback::BET_RightArmReached:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorRightArmReached, pData, pCollisionData);

        case BehaviorFeedback::BET_Relaxed:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorRelaxed, pData, pCollisionData);

        case BehaviorFeedback::BET_Defend:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorDefend, pData, pCollisionData);

        case BehaviorFeedback::BET_Flail:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorFlail, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerBalanced:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerBalanced, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerBalancedFeetOnGround:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerBalancedFeetOnGround, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerTrip:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerTrip, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerMaxSteps:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerMaxSteps, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerStepping:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerStepping, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerStartedFalling:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerStartedFalling, pData, pCollisionData);

        case BehaviorFeedback::BET_StaggerStoppedFalling:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorStaggerStoppedFalling, pData, pCollisionData);

        case BehaviorFeedback::BET_Collision:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorCollision, pData, pCollisionData);

        case BehaviorFeedback::BET_Tumble:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorTumble, pData, pCollisionData);

        case BehaviorFeedback::BET_ConstrainLeftHand:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorConstrainLeftHand, pData, pCollisionData);

        case BehaviorFeedback::BET_ConstrainRightHand:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorConstrainRightHand, pData, pCollisionData);

        case BehaviorFeedback::BET_UnConstrainLeftHand:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorUnConstrainLeftHand, pData, pCollisionData);

        case BehaviorFeedback::BET_UnConstrainRightHand:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorUnConstrainRightHand, pData, pCollisionData);

        case BehaviorFeedback::BET_HangFall:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorHangFall, pData, pCollisionData);

        case BehaviorFeedback::BET_MotionTransferIn:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorMotionTransferIn, pData, pCollisionData);

        case BehaviorFeedback::BET_MotionTransferOut:
            return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorMotionTransferOut, pData, pCollisionData);

		case BehaviorFeedback::BET_BlendFrameDone:
			return EuphoriaBehaviorEvent(EuphoriaEvent::eBehaviorBlendFrameDone, pData, pCollisionData);

        default:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "WARNING: Unknown Behavior Event: %d\n", static_cast<int>(eventType));
            return EuphoriaBehaviorEvent(EuphoriaEvent::eInvalid, NULL, NULL);
        }
    }
}
