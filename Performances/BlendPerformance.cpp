/*********************************************************\
BlendPerformance.cpp
\*********************************************************/

#include "BlendPerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "AnimParamsInterface.h"
#include "BoneCRC.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaEvent.h"
#include "EuphoriaComp.h"
#include "FortuneGamePlugInInterface.h"
#include "PhysicsComponentPlugInInterface.h"
#include "RoninPhysics/RoninPhysics_Base.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace RavenMath;
    using namespace Ronin;
    using Ronin::StateDataDefault;
    using Ronin::StateID;
    using lec::LecTraceChannel;

    // Define statics
    // State controller
    BlendStateController BlendPerformance::msStateController;

    // Chore crcs
    const crc32_t BlendPerformance::msParameterizedChoreTable[] = {     // Make sure all entries for this table are present and are in enum order
        CRC32_NULL,
        CRCINIT("CHORE_PARAMETERIZED_GETUP_FROM_FRONT"),
        CRCINIT("CHORE_PARAMETERIZED_GETUP_FROM_BACK"),
        CRCINIT("CHORE_PARAMETERIZED_GETUP_FROM_LEFT"),
        CRCINIT("CHORE_PARAMETERIZED_GETUP_FROM_RIGHT"),
        CRCINIT("CHORE_PARAMETERIZED_UPRIGHT_RECOVERY"),
        CRCINIT("CHORE_DEATH")
    };
    const crc32_t BlendPerformance::msParameterizedBlockTable[] = {     // Make sure all entries for this table are present and are in enum order
        CRC32_NULL,
        CRCINIT("CBLK_Param_Getup_Front"),
        CRCINIT("CBLK_Param_Getup_Back"),
        CRCINIT("CBLK_Param_Getup_Left"),
        CRCINIT("CBLK_Param_Getup_Right"),
        CRCINIT("CBLK_Param_Recover_Upright"),
        CRCINIT("CBLK_StaticDeadPose")
    };
    const crc32_t BlendPerformance::msNonParameterizedChoreTable[] = {  // Make sure all entries for this table are present and are in enum order
        CRC32_NULL,
        CRCINIT("CHORE_GETUP_FROM_FRONT"),
        CRCINIT("CHORE_GETUP_FROM_BACK"),
        CRCINIT("CHORE_GETUP_FROM_LEFT"),
        CRCINIT("CHORE_GETUP_FROM_RIGHT"),
        CRCINIT("CHORE_UPRIGHT_RECOVERY"),
        CRCINIT("CHORE_DEATH")
    };
    const crc32_t BlendPerformance::msNonParameterizedBlockTable[] = {  // Make sure all entries for this table are present and are in enum order
        CRC32_NULL,
        CRCINIT("CBLK_Euph_Getup_Front"),
        CRCINIT("CBLK_Euph_Getup_Back"),
        CRCINIT("CBLK_Euph_Getup_Left"),
        CRCINIT("CBLK_Euph_Getup_Right"),
        CRCINIT("CBLK_Euph_Recover_Upright"),
        CRCINIT("CBLK_StaticDeadPose")
    };

    // Define all of the states for this Performance
    DEFINE_STATE(BlendPerformance, IDLE);
    DEFINE_STATE(BlendPerformance, TRANSITIONING);
    DEFINE_STATE(BlendPerformance, INTERRUPT);
    DEFINE_STATE(BlendPerformance, BONKED);
    DEFINE_STATE(BlendPerformance, STUCK);
    DEFINE_STATE(BlendPerformance, BLENDTO);
    DEFINE_STATE(BlendPerformance, CLEANUP);

    /*!
     *
     */
    BlendPerformance::BlendPerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner), mpStateMachine(NULL), mBlendPhase(ePhaseNone), mBlendStartTime(0.0f), mBlendDuration(0.0f), mBlendPhaseTwoStart(0.0f)
    {
        // Create a state machine instance for this class instance
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     *
     */
    BlendPerformance::~BlendPerformance()
    {
    }

    /*!
     *
     */
    void BlendPerformance::Start()
    {
        switch (mParams.mInitialPosition)
        {
            case eGetupNone:
                mpStateMachine->ManualTransition(STATEID(STUCK));
                break;
            default:
                mpStateMachine->ManualTransition(STATEID(BLENDTO));
                break;
        }
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void BlendPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    /*!
     *
     */
    void BlendPerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    /*!
     *
     */
    void BlendPerformance::OnEvent(const EuphoriaEvent& event)
    {
        mpStateMachine->OnEvent(event);
    }

    /*!
     * Sets the configuration for this performance. This typically includes 'constants'
     * that affect the way the performance changes state, such as stationary timer
     * settings.
     *
     * @note Look at pros/cons of making this and SetParams template methods.
     */
    void BlendPerformance::Initialize(const DefEuphoriaPerformanceBlendSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

    /*!
     *
     */
    void BlendPerformance::SetParams(const BlendParams& params)
    {
        mParams = params;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    BlendStateController& BlendPerformance::GetStateController()
    {
        static bool sInitialized = false;

        // Run once
        if (!sInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, IDLE, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, TRANSITIONING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, INTERRUPT, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, BONKED, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, STUCK, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, BLENDTO, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(BlendPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sInitialized = true;
        }

        return msStateController;
    }

    /*
     * return ready to try to recover
     */
    bool BlendPerformance::CanRecoverThink()
    {
        return mpOwner->IsBodyStationary();
    }

    /*
     *
     */
    StateID BlendPerformance::Rethink(bool unstick)
    {
        if (mpOwner->GetAnimationInterface()->IsBehaviorActiveAndDriving() && CanRecoverThink())
        {
            GetupPosition ePosition = mpOwner->DetermineGetupPosition();

            if (ePosition != eGetupNone)
            {
                mpOwner->GetAnimationInterface()->StopAllBehaviors();
                mParams.mInitialPosition = ePosition;

                return STATEID(BLENDTO);
            }
            else if (unstick)
            {
                // unknown recovery position
                mpOwner->GetAnimationInterface()->StopAllBehaviors();

                return STATEID(STUCK);
            }
        }

        return STATEID_INVALID;
    }

    /*
     * Converted from the C version
     */
    bool BlendPerformance::UpdateBlendToAnimation()
    {
        // NOTE: This check for mBlendPhase > 0 might not be needed here in Lua-Land...check once this thing is running!
        if (mBlendPhase != ePhaseNone)
        {
            // tick
            if (mBlendStartTime == 0.0f)
            {
                mBlendStartTime = mpStateMachine->GetTimeInCurrentState();
            }
            float elapsedBlendTime = mpStateMachine->GetTimeInCurrentState() - mBlendStartTime;

            switch (mBlendPhase)
            {
                case ePhaseOne:
                    if (elapsedBlendTime >= mBlendPhaseTwoStart)
                    {
                        ParameterOverrideDataInterface* pPhaseTwoData = mpOwner->GetAnimationInterface()->GetPODI();
                        pPhaseTwoData->SetInt(0, (int) mPhaseTwo.effectorRampDuration);
                        pPhaseTwoData->SetInt(1, (int) mPhaseTwo.coreBlendDuration);
                        pPhaseTwoData->SetInt(2, (int) mPhaseTwo.bodyBlendStart);
                        pPhaseTwoData->SetInt(3, (int) mPhaseTwo.bodyBlendDuration);
                        pPhaseTwoData->SetChar(0, mPhaseTwo.group);

                        // Start phase two of the blend
                        mpOwner->GetAnimationInterface()->StopBehavior(EuphoriaBehaviorCRC::eSuperBlend);
                        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eBlendToAnim, pPhaseTwoData->GetPOD());
                        mBlendPhase = ePhaseTwo;
                    }
                    break;
                case ePhaseTwo:
                    if (elapsedBlendTime >= mBlendDuration)
                    {
                        // Reset local blend vars (extracted from EndBehaviorControl)
                        mBlendPhase = ePhaseNone;

                        return false;
                    }
                    break;
            }
        }

        return true;
    }

    /*
     * Determine if there is an obstacle overhead, for interrupting the getup blend
     */
    bool BlendPerformance::IsHeadBlocked()
    {
        // is the get up chore active?

        if (true) //animation.IsReturningFromBehaviorControl(renID))
        {
            RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(BoneCRC::eSpine3);
            Vec3 rayFrom;
            pRPE->GetPosition(rayFrom);
            Vec3 vecTo;
            float speed;
            mpOwner->GetPhysicsInterface()->GetVelocity(vecTo, speed);

            Vec3Set(vecTo, 0.0f, 1.0f, 0.0f);
            float maxHeight = 0.12f;

            if (speed != 0.0f)
            {
                Vec3 rayTo;
                Vec3AddScaled(rayTo, rayFrom, vecTo, maxHeight);

                if (!mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, RoninPhysics::kAll, NULL, mpOwner->GetRenID()))
                {
                    return true;
                }
            }
        }

        return false;
    }

    /*
     * Sets the override parameters for the blend based on the orientation of the NPC
     */
    float BlendPerformance::InitiateBlend(float fGetupBlendTime, GetupPosition ePosition, float turnCorrect, const RavenMath::Vec3& offVec)
    {
        float fSecondBlendTime = fGetupBlendTime * 0.75f;
        float fFirstBlendTime = fGetupBlendTime - fSecondBlendTime;

        // ntoe: duration choice of 30 represents total blend time in a phase 
        //
        // These default timings mean that the blend is finished half way through the
        // animation cycle, by which point the get up position, with weight over the support
        // points, should have ideally been reached. Then the animation can provide the good
        // looking get up. Any blending occurring here can only make things look worse.
        //
        // Below are reasonable phase defaults that take extremities to place quickly but without
        // jerkiness, then lets the animation do what it often needs to do for a good get up.
        // 
        // Sometimes with a mashed up character...
        // a jerk is a consequence of unexpected distance  :O)
        //
        // Any further work should be focused on getting mr. ragdoll to his 'lift position'.
        //
        // Phase 1
        mPhaseOne.bodyStiffness			= 1.0f;
        mPhaseOne.effectorRampDuration	= 15.0f * fFirstBlendTime;
        mPhaseOne.coreBlendDuration		= 30.0f * fFirstBlendTime;
        strcpy(mPhaseOne.group, "handsFeetHead");
        // Phase 2
        mPhaseTwo.effectorRampDuration	= 1.0f * fSecondBlendTime;
        mPhaseTwo.coreBlendDuration		= 5.0f * fSecondBlendTime;
        mPhaseTwo.bodyBlendStart		= 0.0f * fSecondBlendTime;
        mPhaseTwo.bodyBlendDuration		= 5.0f * fSecondBlendTime;
        strcpy(mPhaseTwo.group, "lower");

        switch (ePosition)
        {
            case eGetupBack:
                // overidden as the easyest to improve..... maybe
                fSecondBlendTime = fGetupBlendTime * 0.70f;
                fFirstBlendTime = fGetupBlendTime - fSecondBlendTime;
                // Phase 1
                mPhaseOne.bodyStiffness			= 1.0f;
                mPhaseOne.effectorRampDuration	= 0.0f * fFirstBlendTime;
                mPhaseOne.coreBlendDuration		= 30.0f * fFirstBlendTime;
                strcpy(mPhaseOne.group, "handsFeetHead");
                // Phase 2
                mPhaseTwo.effectorRampDuration	= 1.0f * fSecondBlendTime;
                mPhaseTwo.coreBlendDuration		= 5.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendStart		= 0.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendDuration		= 5.0f * fSecondBlendTime;
                strcpy(mPhaseTwo.group, "all");
                break;

            case eGetupLeft:
            case eGetupRight:
                // the stationary timer settings could need tweeking for this case
                fSecondBlendTime = fGetupBlendTime * 0.80f;
                fFirstBlendTime = fGetupBlendTime - fSecondBlendTime;
                // Phase 1
                mPhaseOne.bodyStiffness			= 1.0f;
                mPhaseOne.effectorRampDuration	= 0.0f * fFirstBlendTime;
                mPhaseOne.coreBlendDuration		= 30.0f * fFirstBlendTime;
                strcpy(mPhaseOne.group, "lower");
                // Phase 2
                mPhaseTwo.effectorRampDuration	= 1.0f * fSecondBlendTime;
                mPhaseTwo.coreBlendDuration		= 5.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendStart		= 0.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendDuration		= 5.0f * fSecondBlendTime;
                strcpy(mPhaseTwo.group, "all");
                break;

            case eGetupUpright:
                // I've not touched this as it may require special treatment [MH]
                //AMAC  Upright idle anim is too long.  Once this anim is fixed delete all lines with 'AMAC'
                fGetupBlendTime = fGetupBlendTime * 0.25f;	//AMAC
                mBlendDuration = fGetupBlendTime;			//AMAC

                fSecondBlendTime = fGetupBlendTime * 0.26f;
                fFirstBlendTime = fGetupBlendTime - fSecondBlendTime;
                // Phase 1
                mPhaseOne.bodyStiffness			= 1.0f;
                mPhaseOne.effectorRampDuration	= 10.0f * fFirstBlendTime;
                mPhaseOne.coreBlendDuration		= 20.0f * fFirstBlendTime;
                strcpy(mPhaseOne.group, "handsFeetHeadPelvis");	// All Core group causing sinking bug
                // Phase 2
                mPhaseTwo.effectorRampDuration	= 5.0f;
                mPhaseTwo.coreBlendDuration		= 10.0f;
                mPhaseTwo.bodyBlendStart		= 0.0f;
                mPhaseTwo.bodyBlendDuration		= 20.0f;
                strcpy(mPhaseTwo.group, "all");
                break;

            case eGetupFront:
                // some leg bending perhaps due to anims being offset too much, note that our pd
                // controllers in super blend ignore y-rotation...
                fSecondBlendTime = fGetupBlendTime * 0.80f * turnCorrect;
                fFirstBlendTime = fGetupBlendTime - fSecondBlendTime;
                // Phase 1
                mPhaseOne.bodyStiffness			= 1.0f;
                mPhaseOne.effectorRampDuration	= 0.0f * fFirstBlendTime;
                mPhaseOne.coreBlendDuration		= 30.0f * fFirstBlendTime;
                strcpy(mPhaseOne.group, "handsFeetHeadPelvis");
                // Phase 2
                mPhaseTwo.effectorRampDuration	= 1.0f * fSecondBlendTime;
                mPhaseTwo.coreBlendDuration		= 5.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendStart		= 0.0f * fSecondBlendTime;
                mPhaseTwo.bodyBlendDuration		= 5.0f * fSecondBlendTime;
                strcpy(mPhaseTwo.group, "all");
                break;
        }

        mPhaseOne.offVec = offVec;
        mPhaseOne.effectorRampDuration = 20.0f * fFirstBlendTime;

        ParameterOverrideDataInterface* pPhaseOneData = mpOwner->GetAnimationInterface()->GetPODI();
        pPhaseOneData->SetFloat(0, mPhaseOne.bodyStiffness);
        pPhaseOneData->SetFloat(1, mPhaseOne.offVec.x);
        pPhaseOneData->SetFloat(2, mPhaseOne.offVec.y);
        pPhaseOneData->SetFloat(3, mPhaseOne.offVec.z);
        pPhaseOneData->SetInt(0, (int) mPhaseOne.effectorRampDuration);
        pPhaseOneData->SetInt(1, (int) mPhaseOne.coreBlendDuration);
        pPhaseOneData->SetChar(0, mPhaseOne.group);

        mpOwner->GetAnimationInterface()->StopAllBehaviors();
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eSuperBlend, pPhaseOneData->GetPOD());

        return fSecondBlendTime;
    }

    /*
     *
     */
    void BlendPerformance::GetExitDirection(float proxRange, RavenMath::Vec3* pOutVec)
    {
        const uint32 nCollisionFlags = RoninPhysics::kAll;

        RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(BoneCRC::eSpine0);
        Vec3 rayFrom;
        pRPE->GetPosition(rayFrom);

        Vec3 vecTo;
        Vec3 rayTo;
    
        Vec3Set(vecTo, 1.0f, 0.0f, 0.0f);
        Vec3AddScaled(rayTo, rayFrom, vecTo, proxRange);
        bool result1 = !mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID());

        Vec3Set(vecTo, -1.0f, 0.0f, 0.0f);
        Vec3AddScaled(rayTo, rayFrom, vecTo, proxRange);
        bool result2 = !mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID());

        Vec3Set(vecTo, 0.0f, 0.0f, 1.0f);
        Vec3AddScaled(rayTo, rayFrom, vecTo, proxRange);
        bool result3 = !mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID());

        Vec3Set(vecTo, 0.0f, 0.0f, -1.0f);
        Vec3AddScaled(rayTo, rayFrom, vecTo, proxRange);
        bool result4 = !mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID());

        Vec3Set(*pOutVec, 0.0f, 0.0f, 0.0f);

        if (result1)
        {
            pOutVec->x -= 1;
        }
        if (result2)
        {
            pOutVec->x += 1;
        }
        if (result3)
        {
            pOutVec->z -= 1;
        }
        if (result4)
        {
            pOutVec->z += 1;
        }

        float mag = Vec3Mag(*pOutVec);
        if (mag > 0.0f)
        {
            Vec3Mul(*pOutVec, *pOutVec, 1.0f / mag);
        }
    }

    /*
     *
     */
    void BlendPerformance::CorrectForWallLeaning(GetupPosition ePosition, GetupPosition* pOutPosition, Vec3* pOutVec)
    {
        const float hitThreshold = 0.75f;
        const uint32 nCollisionFlags = RoninPhysics::kAll;

        float moveOut = 0.0f;
        Vec3SetZero(*pOutVec);

        switch (ePosition)
        {
            case eGetupLeft:
            case eGetupRight:
                {
                    Mat44 tm;
                    RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(BoneCRC::eSpine0);
                    pRPE->GetTransform(tm);
                    Vec3 vecTo = ROW_Z_BASIS(tm);
                    vecTo.y = 0.0f;
                    Vec3Normalize(vecTo, vecTo);

                    Vec3 rayFrom, rayTo;
                    pRPE->GetPosition(rayFrom);
                    Vec3Mul(rayTo, vecTo, hitThreshold);
                    Vec3Add(rayTo, rayFrom, rayTo);
                    if (!mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID()))
                    {
                        ePosition = eGetupBack;
                        moveOut = 2.0f;
                    }

                    Vec3Mul(rayTo, vecTo, -hitThreshold);
                    Vec3Add(rayTo, rayFrom, rayTo);
                    if (!mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID()))
                    {
                        ePosition = eGetupFront;
                        moveOut = 2.0f;
                    }
                }
                break;

            case eGetupBack:
                {
                    Mat44 tm;
                    RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(BoneCRC::eHips0);
                    pRPE->GetTransform(tm);
                    Vec3 vecTo = ROW_Y_BASIS(tm);
                    vecTo.y = 0.0f;
                    Vec3Normalize(vecTo, vecTo);

                    Vec3 rayFrom, rayTo;
                    pRPE->GetPosition(rayFrom);
                    Vec3Mul(rayTo, vecTo, -hitThreshold);
                    Vec3Add(rayTo, rayFrom, rayTo);
                    if (!mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID()))
                    {
                        ePosition = eGetupLeft;

                        // best turn?
                        vecTo = ROW_X_BASIS(tm);
                        vecTo.y = 0.0f;
                        Vec3Normalize(vecTo, vecTo);

                        Vec3Mul(rayTo, vecTo, hitThreshold);
                        Vec3Add(rayTo, rayFrom, rayTo);
                        if (!mpOwner->GetEngineInterface()->CheckLOS(rayFrom, rayTo, 0.0f, nCollisionFlags, NULL, mpOwner->GetRenID()))
                        {
                            ePosition = eGetupRight;
                        }

                        moveOut = 3.0f;
                    }
                }
                break;
        }

        if (moveOut > 0.0f)
        {
            GetExitDirection(1.0f, pOutVec);
            Vec3Mul(*pOutVec, *pOutVec, 0.016f * moveOut);
            pOutVec->y = 0.0f;
        }

        *pOutPosition = ePosition;
    }

    /*!
     * Parameterized version
     */
    void BlendPerformance::FindParameterizedChore(GetupPosition position, crc32_t* pChoreCRC, crc32_t* pBlockCRC)
    {
        *pChoreCRC = msParameterizedChoreTable[position];
        *pBlockCRC = msParameterizedBlockTable[position];
    }

    /*!
     * Non-parameterized version
     */
    void BlendPerformance::FindNonParameterizedChore(GetupPosition position, crc32_t* pChoreCRC, crc32_t* pBlockCRC)
    {
        *pChoreCRC = msNonParameterizedChoreTable[position];
        *pBlockCRC = msNonParameterizedBlockTable[position];
    }

    /*!
     *
     */
    float BlendPerformance::CalcBlockParams(RenID targetRenID, GetupPosition position)
    {
        // get the direction to the current target

        Vec3 targetPos = mpOwner->GetRenInterface()->RenGetPosition(targetRenID);
        Vec3 renPos = mpOwner->GetRenInterface()->RenGetPosition(mpOwner->GetRenID());
        Vec3 dirToTarget;
        Vec3Sub(dirToTarget, targetPos, renPos);
        dirToTarget.y = 0.0f;
        float distToTargetSq = Vec3MagSqr(dirToTarget);
        Vec3 renForward;
        mpOwner->GetRenInterface()->LocationGetForward(mpOwner->GetRenID(), renForward);
        Vec3 targetForward = dirToTarget;

        const float epsilon = 1.0e-4f;
        const float epsilonSq = epsilon * epsilon;
        if (distToTargetSq > epsilonSq)
        {
            Vec3Normalize(targetForward, targetForward);
        }
        else
        {
            targetForward = renForward;
        }

        return RavenMath::ArcTangent2(-renForward.x * targetForward.z + renForward.z * targetForward.x, renForward.x * targetForward.x + renForward.z * targetForward.z);
    }

    /*!
     *
     */
    bool BlendPerformance::CheckRenOrient(GetupPosition ePosition, bool up)
    {
        Mat44 rootTransform;
        RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(BoneCRC::eSpine0);
        pRPE->GetTransform(rootTransform);
        const Mat44& renTransform = mpOwner->GetRenInterface()->RenGetMatrix(mpOwner->GetRenID());
        Vec3 forwardVec = ROW_Y_BASIS(renTransform);
        Vec3Normalize(forwardVec, forwardVec);
        Vec3 renForward;
        mpOwner->GetRenInterface()->LocationGetForward(mpOwner->GetRenID(), renForward);
        renForward.y = 0.0f;
        if (up && (Vec3Dot(forwardVec, renForward) < 0.0f))
        {
            return false;
        }
        else if (!up && (Vec3Dot(forwardVec, renForward) > 0.0f))
        {
            return false;
        }
        return true;
    }

    /*!
     * Determines what orientation the Euphoric character is in
     * We have a valid getup position, so start recovering
     */
    GetupPosition BlendPerformance::DetermineGetupChore(GetupPosition ePosition, GetupPosition cPosition)
    {
        // If there is an instigator, orient on getup, otherwise, do a standard getup
        RenID renInstigator = mParams.mAttacker;
        crc32_t choreCRC = CRC32_NULL;
        crc32_t blockCRC = CRC32_NULL;
        float heading = 0.0f;

        if (ePosition != cPosition)
        {
            switch (cPosition)
            {
                case eGetupBack:
                    if (!CheckRenOrient(ePosition, false))
                    {
                        cPosition = ePosition;
                    }
                    break;
                case eGetupFront:
                    if (!CheckRenOrient(ePosition, true))
                    {
                        cPosition = ePosition;
                    }
                    break;
            }
        }

        switch (cPosition)
        {
            case eGetupFront:
                if (!CheckRenOrient(ePosition, true))
                {
                    cPosition = eGetupRight;
                }
                break;
            case eGetupBack:
                if (!CheckRenOrient(ePosition, false))
                {
                    cPosition = eGetupLeft;
                }
                break;
        }

        switch (cPosition)
        {
            case eGetupLeft:
            case eGetupRight:
                mpOwner->GetInterface()->OrientRen(cPosition);
                break;
        }

        if (renInstigator != RENID_NULL)
        {
            FindParameterizedChore(cPosition, &choreCRC, &blockCRC);
            // get the angle between the thug's heading & the instigator (probably indy)
            heading = CalcBlockParams(renInstigator, cPosition);
        }
        else
        {
            FindNonParameterizedChore(cPosition, &choreCRC, &blockCRC);
        }

        // Make sure a chore has been specified
        LECASSERTMSG(choreCRC, "WARNING: Chore not specified!");

        AnimAngleParamsInterface* pAngleParams = mpOwner->GetAnimationInterface()->GetAnimAngleParamsInterface();
        pAngleParams->SetAngle(heading);
        mpOwner->GetAnimationInterface()->StartBlock(choreCRC, blockCRC, pAngleParams->GetAnimParams(), NULL, true);

        return cPosition;
    }

    /*!
     * Determines what orientation the Euphoric character is in
     * We have a valid getup position, so start recovering
     */
    void BlendPerformance::DetermineGetupChoreSimple(GetupPosition position)
    {
        // If there is an instigator, orient on getup, otherwise, do a standard getup
        RenID renInstigator = mParams.mAttacker;
        crc32_t choreCRC = CRC32_NULL;
        crc32_t blockCRC = CRC32_NULL;
        float heading = 0.0f;

        if (renInstigator != RENID_NULL)
        {
            FindParameterizedChore(position, &choreCRC, &blockCRC);
            // get the angle between the thug's heading & the instigator (probably indy)
            heading = CalcBlockParams(renInstigator, position);
        }
        else
        {
            FindNonParameterizedChore(position, &choreCRC, &blockCRC);
        }

        // Make sure a chore has been specified
        LECASSERTMSG(choreCRC, "WARNING: Chore not specified!");

        AnimationCompPlugInInterface* pAnimComp = mpOwner->GetAnimationInterface();
        AnimAngleParamsInterface* pAngleParams = pAnimComp->GetAnimAngleParamsInterface();
        pAngleParams->SetAngle(heading);
        pAnimComp->StartBlock(choreCRC, blockCRC, pAngleParams->GetAnimParams(), NULL, true);
    }

    //
    // IDLE
    //
    void BlendPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID BlendPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "BlendPerformance::IDLE\n");
        return STATEID_INVALID;
    }

    void BlendPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID BlendPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // TRANSITIONING
    //
    void BlendPerformance::STATEFN_ENTER(TRANSITIONING)(const StateDataDefault& krStateData)
    {
        mpOwner->ResetStationaryTimer();

        Vec3 outVec;
        GetupPosition nPosition;
        CorrectForWallLeaning(mParams.mInitialPosition, &nPosition, &outVec);
        mParams.mInitialPosition = DetermineGetupChore(mParams.mInitialPosition, nPosition);

        // InitiateBlend calls ReturnFromBehaviorControl which sets some blend vars
        // We set the local blend vars right here:
        mBlendPhase = ePhaseOne;
        mBlendDuration = mpOwner->GetAnimationInterface()->GetPrimaryBlockRemainingTime();
        // ADD A CORRECT_FOR_WALL_LEANING MODIFICATION HERE...

        // AMAC: I'm modifying mBlendDuration in InitiateBledn to adjust for the long idle anim
        mBlendPhaseTwoStart = InitiateBlend(mBlendDuration,  mParams.mInitialPosition, 1.0f, outVec);
        mBlendPhaseTwoStart = mBlendDuration - mBlendPhaseTwoStart;
        mBlendStartTime = 0.0f;
    }

    StateID BlendPerformance::STATEFN_UPDATE(TRANSITIONING)(const StateDataDefault& krStateData)
    {
        //  Animation Recovery Logic
        if (mpOwner->GetAnimationInterface()->IsBehaviorActiveAndDriving())
        {
            if (!UpdateBlendToAnimation())
            {
                return STATEID(CLEANUP);
            }

            // replaced with injury system, not
            // Look for blend interupt
            if (IsHeadBlocked())
            {
                return STATEID(INTERRUPT);
            }
        }
        else
        {
            // Done Recovery
            return STATEID(CLEANUP);
        }

        return STATEID_INVALID;
    }

    void BlendPerformance::STATEFN_EXIT(TRANSITIONING)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(TRANSITIONING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        /*
        if (msg.name == "Behavior:Collision") then
            if (msg.partcrc == stricrc32("Head")) then
                print("HEAD")
                --sm:Change(sm.STATE_INTERUPT)
            end
        end
        */

        return STATEID_INVALID;
    }

    //
    // INTERRUPT
    //
    void BlendPerformance::STATEFN_ENTER(INTERRUPT)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();

        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();

        Vec3 out;
        GetExitDirection(1.0f, &out);
        Vec3Mul(out, out, 10.0f);
        out.y = -1.0f;

        pData->SetVector(0, out.x, out.y, out.z);
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eHeadHit, pData->GetPOD());

        mpOwner->ResetStationaryTimer();
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);
    }

    StateID BlendPerformance::STATEFN_UPDATE(INTERRUPT)(const StateDataDefault& krStateData)
    {
        return Rethink(false);
    }

    void BlendPerformance::STATEFN_EXIT(INTERRUPT)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(INTERRUPT)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        switch (event.GetType())
        {
            case EuphoriaEvent::eBehaviorStaggerTrip:
                return STATEID(BONKED);
        }

        return STATEID_INVALID;
    }

    //
    // BONKED
    //
    void BlendPerformance::STATEFN_ENTER(BONKED)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eCatchFall);
        mpOwner->ResetStationaryTimer();
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);
    }

    StateID BlendPerformance::STATEFN_UPDATE(BONKED)(const StateDataDefault& krStateData)
    {
        return Rethink(false);
    }

    void BlendPerformance::STATEFN_EXIT(BONKED)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(BONKED)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // STUCK
    //
    void BlendPerformance::STATEFN_ENTER(STUCK)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Blend::STUCK\n");

        mpOwner->GetAnimationInterface()->StopAllBehaviors();
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eUnstick);
    }

    StateID BlendPerformance::STATEFN_UPDATE(STUCK)(const StateDataDefault& krStateData)
    {
        return Rethink(false);
    }

    void BlendPerformance::STATEFN_EXIT(STUCK)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(STUCK)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // BLENDTO
    //
    void BlendPerformance::STATEFN_ENTER(BLENDTO)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Blend::BLENDTO\n");

        DetermineGetupChoreSimple(mParams.mInitialPosition);

        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();
        float statTime = 0.1f;
        float minAvgVel = 2.0f;

        // Defaults
        pData->SetFloat(0, 1.0f);	// BodyStiffness
        pData->SetFloat(1, 0.0f);	// BlendWeightStart
        pData->SetFloat(2, 1.0f);	// BlendWeightEnd

        pData->SetInt(0, 4);		// BlendRampDuration 
        pData->SetInt(1, 10);		// BlendFrames 
        pData->SetInt(2, 1);		// RootPart 
        pData->SetInt(3, 1);		// AlignToPart 

        pData->SetBool(0, true);	// TransformUpdate
        pData->SetBool(1, false);	// DynamicKeyFraming
        pData->SetBool(2, false);	// Debug

        switch (mParams.mInitialPosition)
        {
            case eGetupBack:
                pData->SetFloat(1, 0.0f); 	// BlendWeightStart
                pData->SetFloat(2, 1.0f);	// BlendWeightEnd
                pData->SetInt(0, 3);		// BlendRampDuration
                pData->SetInt(1, 4);		// BlendFrames
                statTime = 0.12f;
                minAvgVel = 5.0f;
                pData->SetInt(4, 6);		// blendErrorLimit
                break;
            case eGetupLeft:
            case eGetupRight:
                pData->SetFloat(1, 0.0f); 	// BlendWeightStart
                pData->SetFloat(2, 1.0f);	// BlendWeightEnd
                pData->SetInt(0, 3);		// BlendRampDuration
                pData->SetInt(1, 4);		// BlendFrames
                statTime = 0.12f;
                minAvgVel = 0.45f;
                pData->SetInt(4, 6);		// blendErrorLimit
                break;
            case eGetupUpright:
                pData->SetFloat(1, 0.0f); 	// BlendWeightStart
                pData->SetFloat(2, 1.0f);	// BlendWeightEnd
                pData->SetInt(0, 2);		// BlendRampDuration
                pData->SetInt(1, 2);		// BlendFrames
                statTime = 0.1f;
                minAvgVel = 5.0f;
                pData->SetInt(4, 6);		// blendErrorLimit
                break;
            case eGetupFront:
                pData->SetFloat(1, 0.0f); 	// BlendWeightStart
                pData->SetFloat(2, 1.0f);	// BlendWeightEnd
                pData->SetInt(0, 3);		// BlendRampDuration
                pData->SetInt(1, 4);		// BlendFrames
                statTime = 0.3f;
                minAvgVel = 1.0f;
                pData->SetInt(4, 6);		// blendErrorLimit
                break;
        }

        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eBlendToFrame, pData->GetPOD());
        mpOwner->ResetStationaryTimer();
        mpOwner->SetStationaryTimer(minAvgVel, statTime);
    }

    StateID BlendPerformance::STATEFN_UPDATE(BLENDTO)(const StateDataDefault& krStateData)
    {
        if (mpOwner->GetAnimationInterface()->IsBehaviorActiveAndDriving())
        {
            GetupPosition position = mpOwner->DetermineGetupPosition();
            if (position != eGetupNone)
            {
                if (mParams.mInitialPosition != position)
                {
                    mParams.mInitialPosition = position;
                    return STATEID(BLENDTO);
                }
            }
            else
            {
                return STATEID(STUCK);
            }
        }
        else
        {
            return STATEID(CLEANUP);
        }

        return STATEID_INVALID;
    }

    void BlendPerformance::STATEFN_EXIT(BLENDTO)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(BLENDTO)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        switch (event.GetType())
        {
        case EuphoriaEvent::eBehaviorBlendFrameDone:
            mpOwner->GetAnimationInterface()->StopAllBehaviors();
            mParams.mInitialPosition = mpOwner->DetermineGetupPosition();
            if (mParams.mInitialPosition != eGetupNone)
            {
                return STATEID(TRANSITIONING);
            }
            return STATEID(STUCK);
        }

        return STATEID_INVALID;
    }

    //
    // CLEANUP
    //
    void BlendPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "..stopping BLEND\n");

        mpOwner->GetAnimationInterface()->StopAllBehaviors();
        mpOwner->GetAnimationInterface()->EndBehaviorControl();
    }

    StateID BlendPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void BlendPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
        mpOwner->DisconnectPerformance();
    }

    StateID BlendPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }
}
