/*********************************************************\
ThrowPerformance.cpp
\*********************************************************/

#include "ThrowPerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaEvent.h"
#include "EuphoriaComp.h"
#include "EuphoriaUtilities.h"
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
    ThrowStateController ThrowPerformance::msStateController;
	const float ThrowPerformance::msCheckCollisionDelay = 0.2f;
	const float ThrowPerformance::msCheckTransferDelay = 0.1f;
	const float ThrowPerformance::msCrunchDelay = 0.3f;

    // Define all of the states for this Performance
	DEFINE_STATE(ThrowPerformance, IDLE);
	DEFINE_STATE(ThrowPerformance, FLY);
	DEFINE_STATE(ThrowPerformance, BREAK);
	DEFINE_STATE(ThrowPerformance, CRUNCH);
	DEFINE_STATE(ThrowPerformance, IMPACT);
	DEFINE_STATE(ThrowPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sThrowControllerInitialized = false;

    /*!
     *
     */
    ThrowPerformance::ThrowPerformance(EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo)
        : EuphoriaPerformance(pOwner),
          mGrabbedEdge(grabbedEdgeInfo),
          mCheckStartTime(0.0f),
          mCrunchStartTime(0.0f)
    {
        // Create a state machine instance for this class instance
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     *
     */
    ThrowPerformance::~ThrowPerformance()
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw destroyed\n");
    }

    /*!
     *
     */
    void ThrowPerformance::Start()
    {
		mpStateMachine->ManualTransition(STATEID(FLY));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void ThrowPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

	/*!
	 *
	 */
	void ThrowPerformance::Think(float elapsedTime)
	{
		mpStateMachine->Advance(elapsedTime);
	}

	/*!
	 *
	 */
	void ThrowPerformance::OnEvent(const EuphoriaEvent& event)
	{
		mpStateMachine->OnEvent(event);
	}

    /*!
     * Sets the load-time configuration for this performance. This typically includes 'constants'
     * that affect the way the performance changes state, such as stationary timer
     * settings.
     *
     * @note Look at pros/cons of making this and SetParams template methods.
     */
    void ThrowPerformance::Initialize(const DefEuphoriaPerformanceThrowSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

	/*!
	 * Sets the run-time input parameters for this performance. This includes variables that
     * make each performance unique such as the force to apply to a particular
     * bone, etc.
	 */
	void ThrowPerformance::SetParams(const ThrowParams& rParams)
	{
		mParams = rParams;
	}

	/*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    ThrowStateController& ThrowPerformance::GetStateController()
    {
        // Run once
        if (!sThrowControllerInitialized)
        {
            // Attach state functions to the state
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, IDLE, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, FLY, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, BREAK, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, CRUNCH, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, IMPACT, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(ThrowPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sThrowControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void ThrowPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID ThrowPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

	void ThrowPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
	{
	}

	StateID ThrowPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

    //
    // FLY
    //
    void ThrowPerformance::STATEFN_ENTER(FLY)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw::FLY ENTERED\n");

        // Set recovery params for the Throw, but not the FLY state
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);
        mpOwner->DisableRecovery();

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        pAnimation->StartBehavior(EuphoriaBehaviorCRC::eFly, NULL, false);
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eTransfer, NULL, false);
		mpOwner->SetTruncateMovementEnable(false);
		mCheckStartTime = mpStateMachine->GetTimeInCurrentState();
    }

    StateID ThrowPerformance::STATEFN_UPDATE(FLY)(const StateDataDefault& krStateData)
    {
        if (ShouldFallingPerformanceTakeOver(mpOwner))
        {
            FallingParams fp;
            fp.mAttacker = mParams.mAttacker;
            mpOwner->StartPerformance(fp);
        }
        else if (mpOwner->IsReadyToGrab())
        {
            // Try to grab an edge. It's unlikely that we need to check the delay timer
            // when in the throw performance because as soon as an edge is grabbed, the
            // NPC will switch to the fall (which includes hang), so there should not
            // be any dithering possible here. Things may change so it's better to be safe.
            bool bGrabbedSomething = TryToGrabNearbyEdges(mpOwner, mGrabbedEdge);
            if (bGrabbedSomething)
            {
                FallingParams fp;
                fp.mAttacker = mParams.mAttacker;
                mpOwner->StartPerformance(fp);
            }
        }

		return STATEID_INVALID; 
    }

    void ThrowPerformance::STATEFN_EXIT(FLY)(const StateDataDefault& krStateData)
    {
        // Recovery is allowed in other states
        mpOwner->EnableRecovery();

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eFly);
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eTransfer);
		mpOwner->SetTruncateMovementEnable(true);
    }

	StateID ThrowPerformance::STATEFN_ONEVENT(FLY)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		switch (event.GetType())
		{
			// check for end of animation message
			case EuphoriaEvent::eAnimationEnd:
				LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "AnimationEnd\n");
				return STATEID(BREAK);

			// check for collisions
			case EuphoriaEvent::eBehaviorCollision:
				if ((mpStateMachine->GetTimeInCurrentState() - mCheckStartTime) > msCheckCollisionDelay)
				{
					LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Collision\n");
					return STATEID(CRUNCH);
				}
				break;

			// check motion transfer to see if character has been over deformed
			case EuphoriaEvent::eBehaviorMotionTransferOut:
				if ((mpStateMachine->GetTimeInCurrentState() - mCheckStartTime) > msCheckTransferDelay)
				{
					LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "MotionTransfer\n");
                    return STATEID(CRUNCH);
				}
				break;
		}

		return STATEID_INVALID;
	}

    //
    // BREAK
    //
    void ThrowPerformance::STATEFN_ENTER(BREAK)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw::BREAK ENTERED\n");

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eBreak);
    }

    StateID ThrowPerformance::STATEFN_UPDATE(BREAK)(const StateDataDefault& krStateData)
    {
        // Check velocity against gravity up...if the angle is greater than 45 degrees
        // then we will switch into the Falling performance
        if (ShouldFallingPerformanceTakeOver(mpOwner))
        {
            FallingParams fp;
            fp.mAttacker = mParams.mAttacker;
            mpOwner->StartPerformance(fp);
        }

        return STATEID_INVALID; 
    }

    void ThrowPerformance::STATEFN_EXIT(BREAK)(const StateDataDefault& krStateData)
    {
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eBreak);
    }

	StateID ThrowPerformance::STATEFN_ONEVENT(BREAK)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		switch (event.GetType())
		{
			// check for collision to stop flying
			case EuphoriaEvent::eBehaviorCollision:
				LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Collision\n");
				return STATEID(CRUNCH);
		}

		return STATEID_INVALID;
	}

	//
	// CRUNCH
	//
	void ThrowPerformance::STATEFN_ENTER(CRUNCH)(const StateDataDefault& krStateData)
	{
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw::CRUNCH ENTERED\n");

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eCrunch);
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eTorque);
		mCrunchStartTime = mpStateMachine->GetTimeInCurrentState();
	}

	StateID ThrowPerformance::STATEFN_UPDATE(CRUNCH)(const StateDataDefault& krStateData)
	{
        // Check velocity against gravity up...if the angle is greater than 45 degrees
        // then we will switch into the Falling performance
        if (ShouldFallingPerformanceTakeOver(mpOwner))
        {
            FallingParams fp;
            fp.mAttacker = mParams.mAttacker;
            mpOwner->StartPerformance(fp);
        }
        else if (mpOwner->IsReadyToGrab())
        {
            bool bGrabbedSomething = TryToGrabNearbyEdges(mpOwner, mGrabbedEdge);
            if (bGrabbedSomething)
            {
                FallingParams fp;
                fp.mAttacker = mParams.mAttacker;
                mpOwner->StartPerformance(fp);
            }
        }

        // start impact after delay
		if ((mpStateMachine->GetTimeInCurrentState() - mCrunchStartTime) > msCrunchDelay)
		{
			return STATEID(IMPACT);
		}

		return STATEID_INVALID; 
	}

	void ThrowPerformance::STATEFN_EXIT(CRUNCH)(const StateDataDefault& krStateData)
	{
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eCrunch);
	}

	StateID ThrowPerformance::STATEFN_ONEVENT(CRUNCH)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

	//
	// IMPACT
	//
	void ThrowPerformance::STATEFN_ENTER(IMPACT)(const StateDataDefault& krStateData)
	{
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw::IMPACT ENTERED\n");

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StartBehavior(EuphoriaBehaviorCRC::eImpact);
	}

	StateID ThrowPerformance::STATEFN_UPDATE(IMPACT)(const StateDataDefault& krStateData)
	{
		return STATEID_INVALID; 
	}

	void ThrowPerformance::STATEFN_EXIT(IMPACT)(const StateDataDefault& krStateData)
	{
        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopBehavior(EuphoriaBehaviorCRC::eImpact);
	}

	StateID ThrowPerformance::STATEFN_ONEVENT(IMPACT)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

	//
    // CLEANUP
    //
    void ThrowPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw::CLEANUP ENTERED\n");

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
		pAnimation->StopAllBehaviors();
    }

    StateID ThrowPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void ThrowPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

	StateID ThrowPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}
}
