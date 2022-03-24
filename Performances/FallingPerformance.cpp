#include "FallingPerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "BoneCRC.h"
#include "CollisionCastStructs.h"
#include "CollisionManagerPlugInInterface.h"
#include "EdgeSystemManagerPlugInInterface.h"       // REMOVE??
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaComp.h"
#include "EuphoriaStruct.h"
#include "EuphoriaUtilities.h"
#include "FortuneGameCompPlugInInterface.h"
#include "FortuneGamePlugInInterface.h"
#include "PhysicsComponentPlugInInterface.h"
#include "RavenMath.h"
#include "RavenMathDefines.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

namespace euphoria
{
    // USING DIRECTIVES
    using namespace lec;
    using namespace GameHooks;
    using namespace RavenMath;
    using namespace Ronin;
    using namespace RoninPhysics;
    
    // DEFINE STATICS
    // State controller
    FallingStateController FallingPerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(FallingPerformance, IDLE);
    DEFINE_STATE(FallingPerformance, FALLING);
    DEFINE_STATE(FallingPerformance, CATCH_HEAD);
    DEFINE_STATE(FallingPerformance, CATCH_FEET);
    DEFINE_STATE(FallingPerformance, REACT);
    DEFINE_STATE(FallingPerformance, HANGING);
    DEFINE_STATE(FallingPerformance, SLIDING);
    DEFINE_STATE(FallingPerformance, GRABBING);
    DEFINE_STATE(FallingPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sFallingControllerInitialized = false;

    /*!
     * Constructor
     *
     * @note Grabbing is now ON by default instead of being read from an attribute/member (mCanGrabWith[x] = SOME_ATTRIB)
     *
     * @todo More stuff can be moved into initializer list
     */
    FallingPerformance::FallingPerformance(EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo)
        : EuphoriaPerformance(pOwner),
          mGrabbedEdge(grabbedEdgeInfo),
          mkMinFallingImpactTime(1.0f),
          mkMinReactImpactTime(0.2f),
          mkMaxReactHeight(0.7f)
    {
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     * Destructor
     */
    FallingPerformance::~FallingPerformance()
    {
    }

    /*!
     * @todo Take a look to see if this needs to be done like this
     */
    void FallingPerformance::Start()
    {
        // Check to see if some other performance grabbed an edge
        if (mpOwner->IsHandConstrained(eLeftArm) || mpOwner->IsHandConstrained(eRightArm))
        {
            mpStateMachine->ManualTransition(STATEID(HANGING));
            return;
        }

        // In all other cases, start falling
        mpStateMachine->ManualTransition(STATEID(FALLING));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void FallingPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    void FallingPerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    /*!
     * Delegates event handling to the currently running state
     */
    void FallingPerformance::OnEvent(const EuphoriaEvent& event)
    {
        mpStateMachine->OnEvent(event);
    }

    /*!
     * Sets the configuration for this performance. This typically includes 'constants'
     * that affect the way the performance changes state, such as stationary timer
     * settings.
     *
     * @note Look pros/cons of making this and SetParams template methods.
     */
    void FallingPerformance::Initialize(const DefEuphoriaPerformanceFallingSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

    /*!
     * Saves the parameters used by this performance
     *
     * @note Might want to check/enforce that these have been set
     */
    void FallingPerformance::SetParams(const FallingParams& params)
    {
        mParams = params;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    FallingStateController& FallingPerformance::GetStateController()
    {
        // Run once
        if (!sFallingControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, IDLE, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, FALLING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, CATCH_HEAD, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, CATCH_FEET, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, REACT, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, HANGING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, SLIDING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, GRABBING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(FallingPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sFallingControllerInitialized = true;
        }

        return msStateController;
    }

    /*!
     * Wraps the raycast for this performance. All spots in the ported code happened
     * to use the same settings so it lives in this method.
     *
     * @note If some of the functions in this performance move to a utility library,
     *       this code can be embedded in the caller(s)
     */
    bool FallingPerformance::RayCastWrapper(const Vec3& rayFrom, const Vec3& rayTo, RenID renID, Vec4& outHitNormal, float& outHitDist) const
    {
        CollisionManagerPlugInInterface* pCollisionSystem = mpOwner->GetEngineInterface()->GetCollisionManagerInterface();

        RoninPhysics::CastInput ci;
        uint32 flags = RoninPhysics::kAllPhysical & (~RoninPhysics::kDMMTetExact);
        SETUP_CASTINPUT(ci, rayFrom, rayTo, NULL, flags, flags, false);
        pCollisionSystem->AddRenToIgnore(renID, ci);

        CollisionSystem::CollisionResult cr;
        bool bHit = pCollisionSystem->RayCastClosest(ci, cr);
        Vec4Set(outHitNormal, cr.mOutHitNormal);
        outHitDist = cr.mOutHitDist;

        return bHit;
    }

    /*!
     * Returns the number of seconds until impact at the current velocity
     */
    bool FallingPerformance::GetTimeAndDistanceToImpact(float& outTime, float& outHitDist) const
    {
        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        RoninPhysicsEntity* pRPE = pAnimComp->GetBodyPartByBone(BoneCRC::eSpine0);

        Vec3 rayFrom;
        pRPE->GetPosition(rayFrom);

        Vec3 rayTo;
        pRPE->GetLinearVelocity(rayTo);
        float speed = Vec3Mag(rayTo);

        if (speed > 0.2)
        {
            Vec3Normalize(rayTo, rayTo);
            Vec3Mul(rayTo, rayTo, 20.0f);

            Vec3 rayBetween;
            Vec3Add(rayBetween, rayTo, rayFrom);

            // @note ray cast replacement below
            Vec4 outHitNormal;
            bool bHit = RayCastWrapper(rayFrom, rayTo, mpOwner->GetRenID(), outHitNormal, outHitDist);
            if (bHit)
            {
                outTime = outHitDist / speed;
                return true;
            }
        }

        // if all else fails...
        outTime = FLT_MAX;
        outHitDist = FLT_MAX;
        return false;
    }

    //
    // IDLE
    //
    void FallingPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID FallingPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID FallingPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // FALLING
    //
    /*!
     * @todo this used to play a voice event (eAirborneFalling), 2nd to last line. See SCUM code
     * @note There should be no reason to check for IsBehaviorActiveAndDriving
     */
    void FallingPerformance::STATEFN_ENTER(FALLING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::FALLING\n");

        // Set conditions for recovery
        //! @note These values are manipulated in HANGING (to avoid recovery while hanging)
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);

        // Reset any grabbed edge RPE
        mGrabbedEdge.mpOwnerRPE = NULL;

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        pAnimComp->StartOverlay(OverlayParameters::eLeftHand, CRCINIT("LEFT_HAND_OPEN"), CRC32_NULL);
        pAnimComp->StartOverlay(OverlayParameters::eRightHand, CRCINIT("RIGHT_HAND_OPEN"), CRC32_NULL);

        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();

        float fMomentum = GetMomentumMultiplier();
        pData->SetMomentumMultiplier(fMomentum);

        //! @note Can't call this here because NO BEHAVIORS have STARTED (yet)
        //Vec3 vLookAtPos;
        //GetLookDirectionForFalling(vLookAtPos);
        //pData->SetVector(0, vLookAtPos.x, vLookAtPos.y, vLookAtPos.z);
        //pAnimComp->StartBehavior(msFlailBehavior, pData->GetPOD());
        bool bFlailStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eFlail);
        LECASSERT(bFlailStarted);
        LECUNUSED(bFlailStarted);
    }

    /*!
     * Handles state change checks while the character is falling. Potential transitions
     * are: SLIDING, HANGING, head-first (CATCH_HEAD) and feet-first (CATCH_FEET) falling
     *
     * @note 2-handed grabbing is not supported here anymore.
     */
    StateID FallingPerformance::STATEFN_UPDATE(FALLING)(const StateDataDefault& krStateData)
    {
        // @note THIS moved from OnEnter, MIGHT NOT need to be here, or maybe it should be somewhere ELSE?
        // Check to see if we are sliding down a slope
        PhysicsCompPlugInInterface* pPhysicsComp = mpOwner->GetPhysicsInterface();
        Vec3 vNormal;   //!< @note this is REUSED below a couple times
        bool bIsSupported = pPhysicsComp->IsSupported(vNormal);
        if (IsMovingFast() && bIsSupported)
        {
            if (IsSliding(vNormal))
            {
                return STATEID(SLIDING);
            }                    
        }

        if (mpOwner->IsReadyToGrab())
        {
            bool bGrabbedSomething = TryToGrabNearbyEdges(mpOwner, mGrabbedEdge);
            if (bGrabbedSomething)
            {
                return STATEID(HANGING);
            }
        }

        // Check Falling Head first or feet first
        float time = 0.0f;
        float distance = 0.0f;
        bool bTimeResult = GetTimeAndDistanceToImpact(time, distance);

        float height = 0.0f;
        bool bHeightResult = GetHeightFromGround(height, vNormal);
        if ((bTimeResult && time < mkMinFallingImpactTime) || (bHeightResult && height < mkMaxReactHeight))
        {
            if (IsFallingFeetFirst())
            {
                return STATEID(CATCH_FEET);
            }
            else
            {
                return STATEID(CATCH_HEAD);
            }
        }

        // Check to see if we are in a position to pull off landing on feet
        if (IsAbleToLandOnFeet(vNormal, height))
        {
            // Start trying to balance
            //BalanceParams bp;
            //bp.mAttacker = mParams.mAttacker;
            //mpOwner->StartPerformance(bp);
        }
        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(FALLING)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID FallingPerformance::STATEFN_ONEVENT(FALLING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // CATCH_HEAD
    //
    void FallingPerformance::STATEFN_ENTER(CATCH_HEAD)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::CATCH_HEAD\n");

        bool bHeadFirstFallStarted = mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eHeadFirstFall);
        LECASSERT(bHeadFirstFallStarted);
        LECUNUSED(bHeadFirstFallStarted);
    }

    /*!
     * @note NO GRABBING is taking place in this update, same with feet first
     */
    StateID FallingPerformance::STATEFN_UPDATE(CATCH_HEAD)(const StateDataDefault& krStateData)
    {
        float time = 0.0f;
        float distance = 0.0f;
        bool bTimeResult = GetTimeAndDistanceToImpact(time, distance);

        float height = 0.0f;
        Vec3 unusedNormal;
        bool bHeightResult = GetHeightFromGround(height, unusedNormal);

        if ((bTimeResult && time < mkMinReactImpactTime) || (bHeightResult && height < mkMaxReactHeight))
        {
            return STATEID(REACT);
        }
        else if (bTimeResult && time > mkMinFallingImpactTime)
        {
            return STATEID(FALLING);
        }
        else if (IsFallingFeetFirst())
        {
            return STATEID(CATCH_FEET);
        }
        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(CATCH_HEAD)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID FallingPerformance::STATEFN_ONEVENT(CATCH_HEAD)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // CATCH_FEET
    //
    void FallingPerformance::STATEFN_ENTER(CATCH_FEET)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::CATCH_FEET\n");

        //! @note this was commented out on SW because the char was too stiff or something
        //bool bFeetFirstFallStarted = mpOwner->GetAnimationInterface()->StartBehavior(msFeetFirstFallBehavior);
        bool bFeetFirstFallStarted = mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eFlail);
        LECASSERT(bFeetFirstFallStarted);
        LECUNUSED(bFeetFirstFallStarted);
    }

    StateID FallingPerformance::STATEFN_UPDATE(CATCH_FEET)(const StateDataDefault& krStateData)
    {
        float time = 0.0f;
        float distance = 0.0f;
        bool bTimeResult = GetTimeAndDistanceToImpact(time, distance);

        float height = 0.0f;
        Vec3 vNormal;
        bool bHeightResult = GetHeightFromGround(height, vNormal);

        if ((bTimeResult && time < mkMinReactImpactTime) || (bHeightResult && height < mkMaxReactHeight))
        {
            return STATEID(REACT);
        }
        else if (bTimeResult && time > mkMinFallingImpactTime)
        {
            return STATEID(FALLING);
        }
        else if (!IsFallingFeetFirst())
        {
            return STATEID(CATCH_HEAD);
        }
        else if (IsAbleToLandOnFeet(vNormal, height))
        {
            // Start trying to balance
            //BalanceParams bp;
            //bp.mAttacker = mParams.mAttacker;
            //mpOwner->StartPerformance(bp);
        }

        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(CATCH_FEET)(const StateDataDefault& krStateData)
    {
        // [3/12/2008 kguran]
        //! @todo Verify that this is OK. (removed from REACT enter)
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID FallingPerformance::STATEFN_ONEVENT(CATCH_FEET)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // REACT
    //
    /*!
     * @todo Should NOT be handling death stuff here!
     */
    void FallingPerformance::STATEFN_ENTER(REACT)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::REACT\n");

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();

        ParameterOverrideDataInterface* pData = pAnimComp->GetPODI();
        FortuneGameCompPlugInInterface* pGameComp = mpOwner->GetGameInterface();
        if (pGameComp->IsDead())
        {
            // Disable arms reaching
            pData->SetBool(0, true);
        }
        else
        {
            // Do not disable arms reaching
            pData->SetBool(0, false);
        }

        bool bReactionStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eImpactReaction, pData->GetPOD());
        LECASSERT(bReactionStarted);
        LECUNUSED(bReactionStarted);
    }

    /*!
     *
     */
    StateID FallingPerformance::STATEFN_UPDATE(REACT)(const StateDataDefault& krStateData)
    {
        RenID renID = mpOwner->GetRenID();

        Vec3 DZ_LOOKTARGET;
        FindClosestLookAtTarget(renID, DZ_LOOKTARGET);

        float time = 0.0f;
        float distance = 0.0f;
        GetTimeAndDistanceToImpact(time, distance);

        float height = 0.0f;
        Vec3 unusedNormal;
        GetHeightFromGround(height, unusedNormal);

        if ((height > mkMaxReactHeight) && (time > mkMinReactImpactTime))
        {
            return STATEID(FALLING);
        }

        Vec3 vNormal;
        PhysicsCompPlugInInterface* pPhysicsComp = mpOwner->GetPhysicsInterface();
        bool bIsSupported = pPhysicsComp->IsSupported(vNormal);
        if (IsMovingFast() && bIsSupported)
        {
            // Pass in a 0 height because we are touching the ground.
            if (IsAbleToLandOnFeet(vNormal, 0.0f))
            {
                //BalanceParams bp;
                //bp.mAttacker = mParams.mAttacker;
                //mpOwner->StartPerformance(bp);

                return STATEID_INVALID;
            }

            if (IsSliding(vNormal))
            {
                return STATEID(SLIDING);
            }
        }

        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(REACT)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID FallingPerformance::STATEFN_ONEVENT(REACT)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // HANGING
    //
    /*!
     * @note Stationary timers modified on ENTER and EXIT
     * @note Does not support Free Falling objects a la SW yet
     */
    void FallingPerformance::STATEFN_ENTER(HANGING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::HANGING\n");

        // Turn off the ability to recover because the NPC won't move much while hanging
        mpOwner->DisableRecovery();

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        pAnimComp->StartOverlay(OverlayParameters::eLeftHand, CRCINIT("LEFT_HAND_HANG"), CRC32_NULL);
        pAnimComp->StartOverlay(OverlayParameters::eRightHand, CRCINIT("RIGHT_HAND_HANG"), CRC32_NULL);

        ParameterOverrideDataInterface* pPODI = pAnimComp->GetPODI();

        // If the object is dynamic, give the behavior a reference ID and change the edge pos to local space
        // Note: there is ONLY support for a single RPE being grabbed. SW has one for each hand.
        if (mGrabbedEdge.mpOwnerRPE != NULL)
        {
            // Give an entity to euphoria so we know to update the edge based on the entity
            pPODI->SetPhysicsEntity(0, mGrabbedEdge.mpOwnerRPE);
            /*
            if (ReactionHelpers::IsFreeFallingDynamicObject(id, mGrabbedEdge.mpOwnerRPE))
            {
                pPODI->SetFloat(0, 1.0f);       // Reduce damping when grabbed on dynamic object
                pPODI->SetBool(2, false);       // Do not use external forces
            }
            */
        }

        // Set edge start and end
        pPODI->SetVector(0, mGrabbedEdge.mvStart.x, mGrabbedEdge.mvStart.y, mGrabbedEdge.mvStart.z);
        pPODI->SetVector(1, mGrabbedEdge.mvEnd.x, mGrabbedEdge.mvEnd.y, mGrabbedEdge.mvEnd.z);

        // Tell it which hand is already grabbing
        pPODI->SetBool(0, mpOwner->IsHandConstrained(eLeftArm));
        pPODI->SetBool(1, mpOwner->IsHandConstrained(eRightArm));

        bool bHangStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eHang, pPODI->GetPOD());
        LECASSERT(bHangStarted);
        LECUNUSED(bHangStarted);
    }

    StateID FallingPerformance::STATEFN_UPDATE(HANGING)(const StateDataDefault& krStateData)
    {
        // Fall if no longer holding on to anything
        if (!mpOwner->IsHandConstrained(eLeftArm) && !mpOwner->IsHandConstrained(eRightArm))
        {
            return STATEID(FALLING);
        }

        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(HANGING)(const StateDataDefault& krStateData)
    {
        mpOwner->EnableRecovery();
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    /*!
     * Handles euphoria messages relating to constraints ONLY!
     */
    StateID FallingPerformance::STATEFN_ONEVENT(HANGING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        EuphoriaEvent::Type eventType = event.GetType();
        mpOwner->HandleEuphoriaConstraintMessages(eventType);

        return STATEID_INVALID;
    }

    //
    // SLIDING
    //
    void FallingPerformance::STATEFN_ENTER(SLIDING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::SLIDING\n");

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        pAnimComp->StartOverlay(OverlayParameters::eLeftHand, CRCINIT("LEFT_HAND_OPEN"), CRC32_NULL);
        pAnimComp->StartOverlay(OverlayParameters::eRightHand, CRCINIT("RIGHT_HAND_OPEN"), CRC32_NULL);

        bool bSlideStarted = pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eSlide);
        LECASSERT(bSlideStarted);
        LECUNUSED(bSlideStarted);
    }

    /*!
     * @note This grab test should be unified w/ the others (look for calls to FindClosestGrabbableObject)
     * @note having to do 2 searches on edges and save and copy the vars seems a bit excessive
     */
    StateID FallingPerformance::STATEFN_UPDATE(SLIDING)(const StateDataDefault& krStateData)
    {
        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();

        // Left hand
        Vec3 posLeftHand;
        pAnimComp->GetBodyPartByBone(BoneCRC::eLeftHand0)->GetPosition(posLeftHand);
        GrabbedEdgeInfo leftEdge;

        Vec3 vReachForPosLeft;
        float lDist;
        bool bFoundEdgeLeft = SearchForGrabbableEdge(posLeftHand, leftEdge, vReachForPosLeft, lDist);

        // Right hand
        Vec3 posRightHand;
        pAnimComp->GetBodyPartByBone(BoneCRC::eRightHand0)->GetPosition(posRightHand);
        GrabbedEdgeInfo rightEdge;

        Vec3 vReachForPosRight;
        float rDist;
        bool bFoundEdgeRight = SearchForGrabbableEdge(posRightHand, rightEdge, vReachForPosRight, rDist);

        // Find closest edge
        if (bFoundEdgeRight && rDist < lDist)
        {
            mGrabbedEdge = rightEdge;
            return STATEID(GRABBING);
        }
        else if (bFoundEdgeLeft)
        {
            mGrabbedEdge = leftEdge;
            return STATEID(GRABBING);
        }

        // Check for FALLING conditions
        PhysicsCompPlugInInterface* pPhysicsComp = mpOwner->GetPhysicsInterface();
        bool bIsSupported = pPhysicsComp->IsSupported();
        if (!bIsSupported)
        {
            return STATEID(FALLING);
        }


        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(SLIDING)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID FallingPerformance::STATEFN_ONEVENT(SLIDING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // GRABBING
    //
    /*!
     * This state is used for cases when the character is sliding and finds an edge
     * The euphoria behavior will send a "grabbed" message...in these cases there
     * is not a need to have the game decide to create the constraint as it has 
     * to in the (FALLING) state.
     */
    void FallingPerformance::STATEFN_ENTER(GRABBING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::GRABBING\n");

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        ParameterOverrideDataInterface* pPODI = pAnimComp->GetPODI();
        // If the object is dynamic, give the behavior a reference ID and change the edge pos to local space
        // Note: there is ONLY support for a single RPE being grabbed. SW has one for each hand.
        if (mGrabbedEdge.mpOwnerRPE != NULL)
        {
            // Give an entity to euphoria so we know to update the edge based on the entity
            pPODI->SetPhysicsEntity(0, mGrabbedEdge.mpOwnerRPE);
        }
        pPODI->SetVector(0, mGrabbedEdge.mvStart.x, mGrabbedEdge.mvStart.y, mGrabbedEdge.mvStart.z);
        pPODI->SetVector(1, mGrabbedEdge.mvEnd.x, mGrabbedEdge.mvEnd.y, mGrabbedEdge.mvEnd.z);
        pAnimComp->StartBehavior(EuphoriaBehaviorCRC::eGrab, pPODI->GetPOD());
    }

    StateID FallingPerformance::STATEFN_UPDATE(GRABBING)(const StateDataDefault& krStateData)
    {
        // Fall if not supported
        PhysicsCompPlugInInterface* pPhysicsComp = mpOwner->GetPhysicsInterface();
        bool bIsSupported = pPhysicsComp->IsSupported();
        if (!bIsSupported)
        {
            return STATEID(FALLING);
        }

        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(GRABBING)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    /*!
     * Handles euphoria messages relating to constraints ONLY!
     */
    StateID FallingPerformance::STATEFN_ONEVENT(GRABBING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        EuphoriaEvent::Type eventType = event.GetType();
        mpOwner->HandleEuphoriaConstraintMessages(eventType);

        bool bGrabbedLeft = mpOwner->IsHandConstrained(eLeftArm);
        bool bGrabbedRight = mpOwner->IsHandConstrained(eRightArm);
        if (bGrabbedLeft || bGrabbedRight)
        {
            return STATEID(HANGING);
        }

        return STATEID_INVALID;
    }

    //
    // CLEANUP
    //
    void FallingPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling::CLEANUP\n");
    }

    StateID FallingPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void FallingPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

    StateID FallingPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }
}
