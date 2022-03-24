/*********************************************************\
EPAPerformance.cpp
\*********************************************************/

#include "EPAPerformance.h"
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
    EPAStateController EPAPerformance::msStateController;

    // Define all of the states for this Performance
	DEFINE_STATE(EPAPerformance, IDLE);
	DEFINE_STATE(EPAPerformance, RUNNING);
	DEFINE_STATE(EPAPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sEPAControllerInitialized = false;

    /*!
     *
     */
    EPAPerformance::EPAPerformance(EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo)
        : EuphoriaPerformance(pOwner),
          mGrabbedEdge(grabbedEdgeInfo)
    {
        // Create a state machine instance for this class instance
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     *
     */
    EPAPerformance::~EPAPerformance()
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Throw destroyed\n");
    }

    /*!
     *
     */
    void EPAPerformance::Start()
    {
		mpStateMachine->ManualTransition(STATEID(RUNNING));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void EPAPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

	/*!
	 *
	 */
	void EPAPerformance::Think(float elapsedTime)
	{
		mpStateMachine->Advance(elapsedTime);
	}

	/*!
	 *
	 */
	void EPAPerformance::OnEvent(const EuphoriaEvent& event)
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
    void EPAPerformance::Initialize(const DefEuphoriaPerformanceEPASettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

	/*!
	 *
	 */
	void EPAPerformance::SetParams(const EPAParams& params)
	{
		mParams = params;
	}

	/*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    EPAStateController& EPAPerformance::GetStateController()
    {
        // Run once
        if (!sEPAControllerInitialized)
        {
            // Attach state functions to the state
			msStateController.AddState(ADDSTATE_ARGS_EVENT(EPAPerformance, IDLE, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(EPAPerformance, RUNNING, const EuphoriaEvent));
			msStateController.AddState(ADDSTATE_ARGS_EVENT(EPAPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sEPAControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void EPAPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID EPAPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

	void EPAPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
	{
	}

	StateID EPAPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

    //
    // FLY
    //
    void EPAPerformance::STATEFN_ENTER(RUNNING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EPA::RUNNING ENTERED\n");

        // Set conditions for recovery
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        pAnimation->StartEPA();
    }

    StateID EPAPerformance::STATEFN_UPDATE(RUNNING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EPA::RUNNING...\n");

        //if (ShouldFallingPerformanceTakeOver(mpOwner))
        //{
        //    FallingParams fp;
        //    fp.mAttacker = mParams.mAttacker;
        //    mpOwner->StartPerformance(fp);
        //}
        //else if (mpOwner->IsReadyToGrab())
        //{
        //    // Try to grab an edge. It's unlikely that we need to check the delay timer
        //    // when in the throw performance because as soon as an edge is grabbed, the
        //    // NPC will switch to the fall (which includes hang), so there should not
        //    // be any dithering possible here. Things may change so it's better to be safe.
        //    bool bGrabbedSomething = TryToGrabNearbyEdges(mpOwner, mGrabbedEdge);
        //    if (bGrabbedSomething)
        //    {
        //        FallingParams fp;
        //        fp.mAttacker = mParams.mAttacker;
        //        mpOwner->StartPerformance(fp);
        //    }
        //}


        // alex is the king

		return STATEID_INVALID; 
    }

    void EPAPerformance::STATEFN_EXIT(RUNNING)(const StateDataDefault& krStateData)
    {
        //AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        //pAnimation->StopBehavior(EuphoriaBehaviorCRC::eFly);
        //pAnimation->StopBehavior(EuphoriaBehaviorCRC::eTransfer);
        //mpOwner->SetTruncateMovementEnable(true);
    }

	StateID EPAPerformance::STATEFN_ONEVENT(RUNNING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		switch (event.GetType())
		{
			// check for end of animation message
			case EuphoriaEvent::eAnimationEnd:
				//LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "AnimationEnd\n");
				//return STATEID(BREAK);

			// check for collisions
			case EuphoriaEvent::eBehaviorCollision:
				//if ((mpStateMachine->GetTimeInCurrentState() - mCheckStartTime) > msCheckCollisionDelay)
				//{
				//	LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Collision\n");
				//	return STATEID(CRUNCH);
				//}
				break;

			// check motion transfer to see if character has been over deformed
			case EuphoriaEvent::eBehaviorMotionTransferOut:
                //if ((mpStateMachine->GetTimeInCurrentState() - mCheckStartTime) > msCheckTransferDelay)
                //{
                //    LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "MotionTransfer\n");
                //    return STATEID(CRUNCH);
                //}
				break;
		}

		return STATEID_INVALID;
	}

	//
    // CLEANUP
    //
    void EPAPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EPA::CLEANUP ENTERED\n");

        AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        pAnimation->StopEPA();

        //AnimationCompPlugInInterface* pAnimation = mpOwner->GetAnimationInterface();
        //pAnimation->StopAllBehaviors();
    }

    StateID EPAPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    /*!
     * @note May have to DisconnectPerformance here
     */
    void EPAPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

	StateID EPAPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}
}
