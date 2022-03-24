/*********************************************************\
HitReactPerformance.cpp
\*********************************************************/

#include "HitReactPerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaEvent.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaComp.h"
#include "EuphoriaManager.h"
#include "PhysicsComponentPlugInInterface.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace Ronin;
    using Ronin::StateDataDefault;
    using Ronin::StateID;
    using lec::LecTraceChannel;

    // Define statics
    // State controller
    HitReactStateController HitReactPerformance::msStateController;
    const float HitReactPerformance::msStaggerMomentumTolerance = -1.0f;
    const float HitReactPerformance::msRecoverDelay = 0.5f;
    const float HitReactPerformance::msCrunchDelay = 0.3f;

    // Define all of the states for this Performance
	DEFINE_STATE(HitReactPerformance, IDLE);
	DEFINE_STATE(HitReactPerformance, REACT);
	DEFINE_STATE(HitReactPerformance, CRUNCH);
    DEFINE_STATE(HitReactPerformance, IMPACT);
    DEFINE_STATE(HitReactPerformance, CLEANUP);

    /*!
     *
     */
    HitReactPerformance::HitReactPerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner), mRecoverTransfer(false), mRecoverStartTime(0.0f), mCrunchStartTime(0.0f)
    {
        // Create a state machine instance for this class instance
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     *
     */
    HitReactPerformance::~HitReactPerformance()
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "HitReact destroyed\n");
    }

    /*!
     *
     */
    void HitReactPerformance::Start()
    {
		mpStateMachine->ManualTransition(STATEID(REACT));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void HitReactPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

	/*!
	 *
	 */
	void HitReactPerformance::Think(float elapsedTime)
	{
		mpStateMachine->Advance(elapsedTime);
	}

	/*!
	 *
	 */
	void HitReactPerformance::OnEvent(const EuphoriaEvent& event)
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
    void HitReactPerformance::Initialize(const DefEuphoriaPerformanceHitReactSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

    /*!
     * Saves the parameters used by this performance
     *
     * @note Might want to check/enforce that these have been set
     */
	void HitReactPerformance::SetParams(const HitReactParams& params)
	{
		mParams = params;
	}

	/*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    HitReactStateController& HitReactPerformance::GetStateController()
    {
        static bool sInitialized = false;

        // Run once
        if (!sInitialized)
        {
            // Attach state functions to the state
			msStateController.AddState(ADDSTATE_ARGS_EVENT(HitReactPerformance, IDLE, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(HitReactPerformance, REACT, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(HitReactPerformance, CRUNCH, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(HitReactPerformance, IMPACT, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(HitReactPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void HitReactPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID HitReactPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

	void HitReactPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
	{
	}

	StateID HitReactPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

    //
    // REACT
    //
    void HitReactPerformance::STATEFN_ENTER(REACT)(const StateDataDefault& krStateData)
    {
		LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "React");
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        pAnimation->StartBehavior(EuphoriaBehaviorCRC::eReact, NULL, false, false);
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eTransfer_HR, NULL, false, false);
		mpOwner->SetTruncateMovementEnable(false);
		mpOwner->DisableRecovery();
        mRecoverTransfer = false;
    }

    StateID HitReactPerformance::STATEFN_UPDATE(REACT)(const StateDataDefault& krStateData)
    {
		// start recover after delay
        if (!mRecoverTransfer)
        {
            mRecoverStartTime = mpStateMachine->GetTimeInCurrentState();
        }
        if ((mpStateMachine->GetTimeInCurrentState() - mRecoverStartTime) > msRecoverDelay)
        {
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Recover");
            return STATEID(CLEANUP);
        }
        mRecoverTransfer = false;

		return STATEID_INVALID; 
    }

    void HitReactPerformance::STATEFN_EXIT(REACT)(const StateDataDefault& krStateData)
    {
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eReact);
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eTransfer_HR);
		mpOwner->SetTruncateMovementEnable(true);
    }

	StateID HitReactPerformance::STATEFN_ONEVENT(REACT)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		switch (event.GetType())
		{
            // check motion transfer to see if character has recovered
            case EuphoriaEvent::eBehaviorMotionTransferIn:
                mRecoverTransfer = true;
                break;

            // check motion transfer to see if character has been over deformed
			case EuphoriaEvent::eBehaviorMotionTransferOut:
                // find incoming ren momentum
                RavenMath::Vec3 dir;
                float mag, mass;
                GameHooks::FortuneGamePlugInInterface* pEngine = mpOwner->GetEngineInterface();
                PhysicsCompPlugInInterface* pPhysics = pEngine->GetPhysicsCompInterface(mParams.mAttacker);
                pPhysics->GetVelocity(dir, mag);
                pPhysics->GetMass(mass);
                float momentum = mag * mass;

                // respond to incoming hit
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "MotionTransfer");
				if (momentum > msStaggerMomentumTolerance)
                {
                    return STATEID(CRUNCH);
                }
				else
                {
                    /*
					local msg = {}
					msg.name = "EuphoriaStagger"
					mBotPerfMgr:SendMsg(msg)
                    */
				}
				break;
		}

		return STATEID_INVALID;
	}

	//
	// CRUNCH
	//
	void HitReactPerformance::STATEFN_ENTER(CRUNCH)(const StateDataDefault& krStateData)
	{
		LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Crunch");
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eCrunch_HR);
		mCrunchStartTime = mpStateMachine->GetTimeInCurrentState();
	}

	StateID HitReactPerformance::STATEFN_UPDATE(CRUNCH)(const StateDataDefault& krStateData)
	{
		// start impact after delay
		if ((mpStateMachine->GetTimeInCurrentState() - mCrunchStartTime) > msCrunchDelay)
		{
			return STATEID(IMPACT);
		}

		return STATEID_INVALID; 
	}

	void HitReactPerformance::STATEFN_EXIT(CRUNCH)(const StateDataDefault& krStateData)
	{
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eCrunch_HR);
	}

	StateID HitReactPerformance::STATEFN_ONEVENT(CRUNCH)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

	//
	// IMPACT
	//
	void HitReactPerformance::STATEFN_ENTER(IMPACT)(const StateDataDefault& krStateData)
	{
		LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Impact");
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eImpact_HR);
        mpOwner->EnableRecovery();
		mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);
	}

	StateID HitReactPerformance::STATEFN_UPDATE(IMPACT)(const StateDataDefault& krStateData)
	{
		return STATEID_INVALID; 
	}

	void HitReactPerformance::STATEFN_EXIT(IMPACT)(const StateDataDefault& krStateData)
	{
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eImpact_HR);
	}

	StateID HitReactPerformance::STATEFN_ONEVENT(IMPACT)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

	//
    // CLEANUP
    //
    void HitReactPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
		LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "..stopping HITREACT");
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopAllBehaviors();
        pAnimation->EndBehaviorControl();
    }

    StateID HitReactPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void HitReactPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

	StateID HitReactPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}
}
