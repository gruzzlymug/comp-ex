#ifndef EUPHORIA_COMPONENT_H
#define EUPHORIA_COMPONENT_H

#include "RoninConfig.h"
#include "ComponentBase.h"
#include "BehaviorSklMgr.h"
#include "EuphoriaCompPlugInInterface.h"
#include "EuphoriaConstants.h"
#include "EuphoriaEvent.h"
#include "EuphoriaStruct.h"
#include "RoninSortedComponentList.h"

namespace euphoria
{
    // FORWARD DECLARATIONS
    class EuphoriaParamsBase;
    class BalanceParams;
    class BlendParams;
    class EPAParams;
    class ExplosionParams;
    class PunchParams;
    class ShoveParams;
    class ThrowParams;
    class HitReactParams;
    class FallingParams;
    class GunshotParams;
    // Performances
    class EuphoriaPerformance;
    class BlendPerformance;
    class EPAPerformance;
    class ExplosionPerformance;
    class PunchPerformance;
    class ShovePerformance;
    class ThrowPerformance;
    class FallingPerformance;
    class GunshotPerformance;
    class BalancePerformance;
    class HitReactPerformance;
}

namespace GameHooks
{
    class FortuneGamePlugInInterface;
}

namespace GameHooks
{
    class FortuneGamePlugInInterface;
}

namespace Ronin
{
    // FORWARD DECLARATIONS
    class AnimationCompPlugInInterface;
    class PhysicsCompPlugInInterface;
    class ScriptCompPlugInInterface;
    class FortuneGameCompPlugInInterface;
    class FortuneGameRenPlugInInterface;

    /*!
     * @todo IMPORTANT!!! Add checks prior to starting performances to MAKE SURE that AGENTS ARE AVAILABLE
     * @todo Go through and optimize Vec3/Vec4 stuff w/ V4s
     * @todo Should there be a ptr to the EdgeSystemMgr stored? or is it OK to just get it via the EngineInterface each time?
     */
    DECLARE_ALIGNED class EuphoriaComp // Uncomment when we want a common component base class : public CompBase
    {
        LECALIGNEDCLASS(16);

        // FRIENDS
        friend class SortedCompPtrList<EuphoriaComp>;
        friend class EuphoriaCompPlugIn;
        friend class RerouteSystem;

    private:
        // MEMBERS
        RenID mRenID;
        EuphoriaCompPlugIn mPlugInInterface;
        RenID mAttackerRenID;

        // Component interfaces
        AnimationCompPlugInInterface* mpAnimationComp;
        FortuneGameCompPlugInInterface* mpGameComp;
        PhysicsCompPlugInInterface* mpPhysicsComp;
        ScriptCompPlugInInterface* mpScriptComp;
        FortuneGameRenPlugInInterface* mpRenInterface;
        GameHooks::FortuneGamePlugInInterface* mpEngineInterface;

        // Stationary timers
        float mfStationaryWaitTime;
        float mfStationaryElapsedTime;
        float mfStationaryMinSpeed;

        // Performance Members
        euphoria::EuphoriaPerformance* mpCurrentPerformance;
        euphoria::ThrowPerformance* mpPerfThrow;
        euphoria::BlendPerformance* mpPerfBlend;
        euphoria::PunchPerformance* mpPerfPunch;
        euphoria::EPAPerformance* mpPerfEPA;
        euphoria::ExplosionPerformance* mpPerfExplosion;
        euphoria::ShovePerformance* mpPerfShove;
        euphoria::FallingPerformance* mpPerfFalling;
        euphoria::GunshotPerformance* mpPerfGunshot;
        euphoria::BalancePerformance* mpPerfBalance;
        euphoria::HitReactPerformance* mpPerfHitReact;

        // Constraint related
        euphoria::GrabbedEdgeInfo mGrabbedEdge;                                 //!< Keeps track of what this NPC is grabbing
        bool mbIsGrabbingWith[euphoria::eNumArms];
        RoninPhysicsConstraint* mpHandConstraint[euphoria::eNumArms];
        const float mkConstraintBreakThreshold;
        //! Keeps track of time between grab attempts
        float mGrabDelayTimer;
        const float mkGrabDelayThreshold;

        // Recovery related
        bool mbIsRecoveryEnabled;
        bool mbIsTransitioning;

        // Miscellaneous - Should the character ignore edges which constrain navigation?
        bool mbTruncateMovementEnable;

        // METHODS
        void CreatePerformances();
        void DestroyPerformances();
        void CheckBodyForMovement(float elapsedTime);

        // OVERLOADED METHODS FOR INDIVIDUAL PERFORMANCES
        void StartSpecificPerformance(const euphoria::BlendParams& rParams);
        void StartSpecificPerformance(const euphoria::EPAParams& rParams);
        void StartSpecificPerformance(const euphoria::ExplosionParams& rParams);
        void StartSpecificPerformance(const euphoria::PunchParams& rParams);
        void StartSpecificPerformance(const euphoria::ShoveParams& rParams);
        void StartSpecificPerformance(const euphoria::ThrowParams& rParams);
        void StartSpecificPerformance(const euphoria::HitReactParams& rParams);
        void StartSpecificPerformance(const euphoria::FallingParams& rParams);
        void StartSpecificPerformance(const euphoria::GunshotParams& rParams);
        void StartSpecificPerformance(const euphoria::BalanceParams& rParams);

    public:
        // CREATORS
        EuphoriaComp(RenID renID);
        ~EuphoriaComp();

        // MANIPULATORS
        inline EuphoriaCompPlugInInterface* GetInterface();
        void InitCrossComp();
        void InitPostCrossComp();
        void Think(float elapsedTime);
        void Destroy();
        // EXPOSED MANIPULATORS
        inline void SetTruncateMovementEnable(bool bEnable);
        inline void DisableRecovery();
        inline void EnableRecovery();
        inline void SetStationaryTimer(float speedThreshold, float waitTime);
        inline void ResetStationaryTimer();
        void StartPerformance(const euphoria::EuphoriaParamsBase& rParams);
        void StopPerformance();
        void DisconnectPerformance();
        void HandleAnimationEvent(crc32_t eventNameCRC);
        void HandleBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData);

        // Constraint-related
        void CheckForBrokenConstraints();
        void ConstrainLimb(euphoria::ReachingArm reachingArm, RoninPhysicsEntity* pEntityToGrab, const RavenMath::Vec3& vWorldReachForPos);
        void ReleaseConstraint(euphoria::ReachingArm reachingArm);
        void HandleEuphoriaConstraintMessages(euphoria::EuphoriaEvent::Type eventType);

        //! @todo this can be moved into BlendPerformance once Lua is laid to rest...
        void OrientRen(euphoria::GetupPosition position);

        // ACCESSORS
        inline RenID GetRenID() const;

        // Interface accessors
        inline AnimationCompPlugInInterface* GetAnimationInterface() const;
        inline FortuneGameCompPlugInInterface* GetGameInterface() const;
        inline PhysicsCompPlugInInterface* GetPhysicsInterface() const;
        inline FortuneGameRenPlugInInterface* GetRenInterface() const;
        inline GameHooks::FortuneGamePlugInInterface* GetEngineInterface() const;

        // Constraint/Grabbing-related
        inline bool IsReadyToGrab() const;
        inline bool IsHandConstrained(euphoria::ReachingArm reachingArm) const;

        void DebugDraw() const;

        // EXPOSED ACCESSORS
        inline bool IsEuphoriaActive() const;
        inline bool GetTruncateMovementEnable() const;
        inline bool IsBodyStationary() const;
        euphoria::GetupPosition DetermineGetupPosition() const;

    } END_DECLARE_ALIGNED;

    // INLINES
    RenID EuphoriaComp::GetRenID() const
    {
        return mRenID;
    }

    AnimationCompPlugInInterface* EuphoriaComp::GetAnimationInterface() const
    {
        return mpAnimationComp;
    }

    FortuneGameCompPlugInInterface* EuphoriaComp::GetGameInterface() const
    {
        return mpGameComp;
    }

    PhysicsCompPlugInInterface* EuphoriaComp::GetPhysicsInterface() const
    {
        return mpPhysicsComp;
    }

    FortuneGameRenPlugInInterface* EuphoriaComp::GetRenInterface() const
    {
        return mpRenInterface;
    }

    /*!
     * Returns a plug-in that allows the objects in the DLL to access engine
     * functionality not available in the interfaces. This is usually required
     * when the functions in question would suffer from linking issues if they
     * were in the regular interfaces.
     *
     * This is primarily here for use by the performances.
     *
     * @note The name is GetEngineInterface to 1) avoid confusion with GetGameInterface,
     *       and 2) because that's what it really is
     */
    GameHooks::FortuneGamePlugInInterface* EuphoriaComp::GetEngineInterface() const
    {
        return mpEngineInterface;
    }

    EuphoriaCompPlugInInterface* EuphoriaComp::GetInterface()
    {
        return reinterpret_cast<EuphoriaCompPlugInInterface*>(&mPlugInInterface);
    }

    // EXPOSED METHOD - INLINE
    /*!
     * Allows the character to travel over edges when true. This is useful for
     * performances such as the Throw.
     *
     * @note Someday this can/should be done in the MoveGraph. There are other moves that do this. UNIFY.
     */
    void EuphoriaComp::SetTruncateMovementEnable(bool bEnable)
    {
        mbTruncateMovementEnable = bEnable;
    }

    void EuphoriaComp::DisableRecovery()
    {
        mbIsRecoveryEnabled = false;
    }

    void EuphoriaComp::EnableRecovery()
    {
        mbIsRecoveryEnabled = true;
    }

    // EXPOSED METHOD - INLINE
    void EuphoriaComp::SetStationaryTimer(float speedThreshold, float waitTime)
    {
        mfStationaryWaitTime = waitTime;
        mfStationaryMinSpeed = speedThreshold;
    }

    void EuphoriaComp::ResetStationaryTimer()
    {
        mfStationaryElapsedTime = 0.0f;
    }

    // EXPOSED METHOD - INLINE
    bool EuphoriaComp::IsEuphoriaActive() const
    {
        return (mpCurrentPerformance != NULL);
    }

    // EXPOSED METHOD - INLINE
    bool EuphoriaComp::GetTruncateMovementEnable() const
    {
        return mbTruncateMovementEnable;
    }

    // EXPOSED METHOD - INLINE
    /*!
     * Returns true if the body has been still longer than the wait time, false otherwise
     */
    bool EuphoriaComp::IsBodyStationary() const
    {
        return (mfStationaryElapsedTime > mfStationaryWaitTime);
    }

    /*!
     * Makes sure enough time has passed since the last grab attempt. This will
     * prevent grab-release dithering.
     *
     * @note Might want to have one timer for each grabbing limb
     */
    bool EuphoriaComp::IsReadyToGrab() const
    {
        return (mGrabDelayTimer > mkGrabDelayThreshold);
    }

    /*!
     * Returns true if the reaching arm is constrained, false otherwise.
     */
    bool EuphoriaComp::IsHandConstrained(euphoria::ReachingArm reachingArm) const
    {
        LECASSERTMSG((reachingArm >= 0 && reachingArm < euphoria::eNumArms), "reachingArm OUT OF RANGE!");

        return mbIsGrabbingWith[reachingArm];
    }
}

#define PLUGIN_IMPL_EuphoriaComp
#include "EuphoriaCompPlugInInterface.h"
#undef  PLUGIN_IMPL_EuphoriaComp

// EUPHORIA_COMPONENT_H
#endif
