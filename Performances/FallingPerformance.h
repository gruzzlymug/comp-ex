#ifndef EUPHORIA_FALLING_PERFORMANCE_H
#define EUPHORIA_FALLING_PERFORMANCE_H

#include "EuphoriaPerformance.h"
#include "EdgeSystemTypes.h"
#include "EuphoriaEvent.h"
#include "EuphoriaParams.h"
#include "RenTypes.h"
#include "StateController.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceFallingSettings;
// Physics Entity
class RoninPhysicsEntity_Havok;
typedef RoninPhysicsEntity_Havok	    RoninPhysicsEntity;
// Physics Constraint
class RoninPhysicsConstraint_Havok;
typedef RoninPhysicsConstraint_Havok    RoninPhysicsConstraint;

namespace Ronin
{
    // FORWARD DECLARATIONS
    class AnimationCompPlugInInterface;
}

namespace euphoria
{
    // FORWARD DECLARATIONS
    class FallingPerformance;
    class GrabbedEdgeInfo;

    // TYPEDEFS
    typedef Ronin::StateMachine<FallingPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> FallingStateController;

    /*!
     * @note ONLY grabbing of non-actor-embedded edges is supported!
     * @note Grabbing DMM and other NPCs is not supported.
     */
    DECLARE_ALIGNED class FallingPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);
    
    private:
        // STATIC MEMBERS
        static FallingStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        FallingParams mParams;

        // Reference to the owning component's GrabbedEdgeInfo for convenience
        GrabbedEdgeInfo& mGrabbedEdge;

        // Constants extracted during port from SW
        const float mkMinFallingImpactTime;
        const float mkMinReactImpactTime;
        const float mkMaxReactHeight;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        FallingPerformance(const FallingPerformance&);
        FallingPerformance& operator=(const FallingPerformance&);

        // METHODS
        FallingStateController& GetStateController();

        // Character state checks
        bool IsMovingFast() const;
        bool IsLayingOnBack() const;
        bool IsSliding(const RavenMath::Vec3& rNormal);
        bool IsFallingFeetFirst() const;
        bool IsAbleToLandOnFeet(const RavenMath::Vec3& rNormal, float fHeight) const;

        // Helper for state checks
        bool RayCastWrapper(const RavenMath::Vec3& rayFrom, const RavenMath::Vec3& rayTo, Ronin::RenID renID, RavenMath::Vec4& outHitNormal, float& outHitDist) const;

        // To be moved to a utility library
        float GetMomentumMultiplier() const;
        float GetAverageSpeedOfBody() const;
        bool GetHeightFromGround(float& outHitDist, RavenMath::Vec3& outHitNormal) const;
        bool GetTimeAndDistanceToImpact(float& outTime, float& outHitDist) const;
        void GetLookDirectionForFalling(RavenMath::Vec3& vLookAtPos) const;
        bool FindClosestLookAtTarget(Ronin::RenID renID, RavenMath::Vec3& vLookAtPos) const;

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FALLING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CATCH_HEAD, const EuphoriaEvent);                   // LANDING_ON_HEAD
        DECLARE_STATE_EVENT(CATCH_FEET, const EuphoriaEvent);                   // LANDING_ON_FEET
        DECLARE_STATE_EVENT(REACT, const EuphoriaEvent);                        // ?
        DECLARE_STATE_EVENT(HANGING, const EuphoriaEvent);                      // This is part of grab N go, in theory
        DECLARE_STATE_EVENT(SLIDING, const EuphoriaEvent);                      // This is part of grab N go, in theory
        DECLARE_STATE_EVENT(GRABBING, const EuphoriaEvent);                     // when sliding & you find an edge, go into this
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit FallingPerformance(Ronin::EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo);
        virtual ~FallingPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceFallingSettings& params);
        void SetParams(const FallingParams& params);
    } END_DECLARE_ALIGNED;
}

// EUPHORIA_GUNSHOT_PERFORMANCE_H
#endif
