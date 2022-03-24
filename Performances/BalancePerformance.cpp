/*********************************************************\
BalancePerformance.cpp
\*********************************************************/

#include "BalancePerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaBehaviorCRC.h"
#include "PhysicsComponentPlugInInterface.h"
#include "EuphoriaComp.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace RavenMath;
    using namespace Ronin;
    using namespace lec;

    // Define statics
    // State controller
    BalanceStateController BalancePerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(BalancePerformance, IDLE);
    DEFINE_STATE(BalancePerformance, BALANCING);
    DEFINE_STATE(BalancePerformance, FALLING);
    DEFINE_STATE(BalancePerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sBalanceControllerInitialized = false;

    /*!
     * Constructor
     */
    BalancePerformance::BalancePerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner)
    {
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     * Destructor
     */
    BalancePerformance::~BalancePerformance()
    {

    }

    /*!
     * Determines whether the Falling Performance should be started
     *
     * @note This function was called ShouldGoToAirborne
     */
    bool BalancePerformance::IsFalling(RenID renID, bool bIsSupported, bool bIsMovingFast) const
    {
/*
        AnimationCompPlugInInterface* pAnimComp = GetAnimationComp(id);

        if (!isSupported && pAnimComp->IsConstrainedToMe())
        {
            // Someone might be grabbing onto me, so me landing this jump is very unlikely.
            return true;
        }

        if (isSupported && bIsMovingFast)
        {
            PhysicsComp *pPhysics = GetPhysicsComp(id);
            Vec3 vNormal;
            if (pPhysics->IsSupported(vNormal))
            {
                // Check if the character's body is parallel to the surface it is laying on.
                RoninPhysicsEntity* pRPESpine0 = pAnimComp->GetBodyPartByBone(BoneCRC::eSpine0);
                LECASSERT(pRPESpine0);
                Vec3 vSpine0Pos;
                pRPESpine0->GetPosition(vSpine0Pos);

                RoninPhysicsEntity* pRPESpine3 = pAnimComp->GetBodyPartByBone(BoneCRC::eSpine3);
                LECASSERT(pRPESpine3);
                Vec3 vSpine3Pos;
                pRPESpine3->GetPosition(vSpine3Pos);

                Vec3 vSpineVec;
                Vec3Sub(vSpineVec, vSpine0Pos, vSpine3Pos);
                Vec3Normalize(vSpineVec, vSpineVec);

                if (RavenMath::Abs(Vec3Dot(vNormal, vSpineVec)) < 0.05f)
                {
                    // Ok our back is parallel to the ground

                    // Lets make sure that we are not just folded over from landing a high jump.
                    // Check if the character's body is parallel to the surface it is laying on.
                    RoninPhysicsEntity *pRPEPelvis = pAnimComp->GetBodyPartByBone(BoneCRC::eHips0);
                    if (!pRPEPelvis)
                    {
                        return false;
                    }

                    Vec3 vPelvisPos;
                    pRPEPelvis->GetPosition(vPelvisPos);

                    RoninPhysicsEntity *pRPEThigh = pAnimComp->GetBodyPartByBone(BoneCRC::eRightUpperLeg0);
                    if (!pRPEThigh)
                    {
                        return false;
                    }

                    Vec3 vThighPos;
                    pRPEThigh->GetPosition(vThighPos);

                    Vec3 vPelvisToThighVec;

                    Vec3Sub(vPelvisToThighVec, vPelvisPos, vThighPos);
                    Vec3Normalize(vPelvisToThighVec, vPelvisToThighVec);

                    if (Vec3Dot(vNormal, vPelvisToThighVec) > -0.80f)
                    {
                        return true;
                    }
                }
            }
        }
        else
        {
            if (!IsSupportClose(id))
            {
                return true;
            }                
        }
*/
        return false;
    }

    void BalancePerformance::Start()
    {
        PhysicsCompPlugInInterface* pPhysicsComp = mpOwner->GetPhysicsInterface();
        if (pPhysicsComp->IsSupported())
        {
            mpStateMachine->ManualTransition(STATEID(BALANCING));
        }
        else
        {
            mpStateMachine->ManualTransition(STATEID(FALLING));
        }
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void BalancePerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    /*!
     * Delegates Think tasks to the individual states of this performance
     */
    void BalancePerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    void BalancePerformance::OnEvent(const EuphoriaEvent& event)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Balance NOT handling events\n");
    }

    /*!
     * Saves the parameters used by this performance
     */
    void BalancePerformance::SetParams(const BalanceParams& rBalanceParams)
    {
        mParams = rBalanceParams;
    }

    /*!
    * Returns the local static copy of the state controller. Initializes it if
    * necessary.
    */
    BalanceStateController& BalancePerformance::GetStateController()
    {
        // Run once
        if (!sBalanceControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BalancePerformance, IDLE, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BalancePerformance, BALANCING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BalancePerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sBalanceControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void BalancePerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID BalancePerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void BalancePerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID BalancePerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // BALANCING
    //
    void BalancePerformance::STATEFN_ENTER(BALANCING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "BalancePerformance::BALANCING ENTERED\n");

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        bool bStaggerStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eStaggerBalance);
        LECASSERT(bStaggerStarted);
        LECUNUSED(bStaggerStarted);
    }

    StateID BalancePerformance::STATEFN_UPDATE(BALANCING)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void BalancePerformance::STATEFN_EXIT(BALANCING)(const StateDataDefault& krStateData)
    {
        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        pAnimComp->StopAllBehaviors();
    }

    StateID BalancePerformance::STATEFN_ONEVENT(BALANCING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // FALLING
    //
    void BalancePerformance::STATEFN_ENTER(FALLING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "BalancePerformance::FALLING ENTERED\n");

        // Legs behavior
        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        bool bStaggerStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eStaggerBalance);
        LECASSERT(bStaggerStarted);
        LECUNUSED(bStaggerStarted);

        // Arms behavior
        bool bLandingFallWindmillStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eLandingFall);
        LECASSERT(bLandingFallWindmillStarted);
        LECUNUSED(bLandingFallWindmillStarted);
    }

    StateID BalancePerformance::STATEFN_UPDATE(FALLING)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void BalancePerformance::STATEFN_EXIT(FALLING)(const StateDataDefault& krStateData)
    {
        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        pAnimComp->StopAllBehaviors();
    }

    StateID BalancePerformance::STATEFN_ONEVENT(FALLING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // CLEANUP
    //
    void BalancePerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "BalancePerformance::CLEANUP\n");
    }

    StateID BalancePerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void BalancePerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

    StateID BalancePerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }
}
