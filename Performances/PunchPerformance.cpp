/*********************************************************\
PunchPerformance.cpp
\*********************************************************/

#include "PunchPerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaEvent.h"
#include "EuphoriaComp.h"
#include "FortuneGamePlugInInterface.h"
#include "ParameterOverrideDataInterface.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace RavenMath;
    using namespace Ronin;
    using lec::LecTraceChannel;

    // Define statics
    // State controller
    PunchStateController PunchPerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(PunchPerformance, IDLE);
    DEFINE_STATE(PunchPerformance, ON_FEET);
    DEFINE_STATE(PunchPerformance, FALLING);
    DEFINE_STATE(PunchPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sPunchControllerInitialized = false;

    /*!
     * Constructor
     */
    PunchPerformance::PunchPerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner)
    {
        // Create a state machine instance for this class instance
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     * Destructor
     */
    PunchPerformance::~PunchPerformance()
    {
    }

    /*!
     * Puts the state machine into the starting state for this performance
     */
    void PunchPerformance::Start()
    {
        mpStateMachine->ManualTransition(STATEID(ON_FEET));
    }

    void PunchPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    /*!
     *
     */
    void PunchPerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    /*!
     * Handles events from the euphoria system for *all* states
     */
    void PunchPerformance::OnEvent(const EuphoriaEvent& event)
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
    void PunchPerformance::Initialize(const DefEuphoriaPerformancePunchSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

    /*!
     * Saves the parameters used by this performance
     *
     * @note Might want to check/enforce that these have been set
     */
    void PunchPerformance::SetParams(const PunchParams& params)
    {
        mParams = params;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    PunchStateController& PunchPerformance::GetStateController()
    {
        // Run once
        if (!sPunchControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS_EVENT(PunchPerformance, IDLE, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(PunchPerformance, ON_FEET, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(PunchPerformance, FALLING, const EuphoriaEvent));
            msStateController.AddState(ADDSTATE_ARGS_EVENT(PunchPerformance, CLEANUP, const EuphoriaEvent));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sPunchControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void PunchPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID PunchPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

	void PunchPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
	{
	}

	StateID PunchPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

    //
    // ON_FEET
    //
    void PunchPerformance::STATEFN_ENTER(ON_FEET)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "PunchPerformance::ON_FEET Entered\n");

        // Set conditions for recovery
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);

        // Prepare overrides for the punch behavior
        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();

        pData->SetVector(0,                                     // Impulse vector
            mParams.mForceNormal.x * mParams.mForceMagnitude,
            mParams.mForceNormal.y * mParams.mForceMagnitude,
            mParams.mForceNormal.z * mParams.mForceMagnitude);
        pData->SetFloat(0, 0.1f);                               // Spread
        pData->SetFloat(1, 0.0f);                               // Duration
        pData->SetBool(0, false);                               // Apply total each frame
        pData->SetPart(0, mParams.mImpactBone);                 // Part to apply impulse to
        // For torque (from original Lua)
        pData->SetVector(1, 0.0f, 1.0f, 0.0f);
        pData->SetFloat(2, 0);

        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::ePunch, pData->GetPOD());

        // Punch Behavior for Root, to prevent the character from folding
        pData->Reset();

        RoninPhysicsEntity* pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(mParams.mImpactBone);
        LECASSERT(pRPE);
        
        Vec3 posImpactBone;
        pRPE->GetPosition(posImpactBone);

        pRPE = mpOwner->GetAnimationInterface()->GetBodyPartByBone(CRCINIT("Spine[3]"));
        Vec3 posChest;
        pRPE->GetPosition(posChest);

        Vec3 toImpactFromChest;
        Vec3Sub_UA(toImpactFromChest, posImpactBone, posChest);
        Vec3Normalize_UA(toImpactFromChest, toImpactFromChest);

        float scaleFactor = 4.0f;
        pData->SetVector(0,                                     // Impulse vector
            toImpactFromChest.x * scaleFactor,
            toImpactFromChest.y * scaleFactor,
            toImpactFromChest.z * scaleFactor);
        pData->SetFloat(0, 0.3f);                               // Spread
        pData->SetFloat(1, 0.3f);                               // Duration
        pData->SetBool(0, false);                               // Apply total each frame
        pData->SetPart(0, CRCINIT("Root"));                     // Part to apply impulse to

        // Overrides for Add Torque
        // These settings are hard-coded for the "right hook"
        pData->SetVector(1, 0.0f, 1.0f, 0.0f);
        pData->SetFloat(2, 0.0f);

        // Start punch
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::ePunch, pData->GetPOD());

        // Start stagger
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eStaggerPunch, NULL);
    }

    StateID PunchPerformance::STATEFN_UPDATE(ON_FEET)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "PunchPerformance::ON_FEET Updating\n");
        return STATEID_INVALID;
    }

    void PunchPerformance::STATEFN_EXIT(ON_FEET)(const StateDataDefault& krStateData)
    {
    }

	StateID PunchPerformance::STATEFN_ONEVENT(ON_FEET)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		// Transitions into the same state (FALLING) for both messages
		switch (event.GetType())
		{
		case EuphoriaEvent::eBehaviorStaggerTrip:
		case EuphoriaEvent::eBehaviorStaggerMaxSteps:
			return STATEID(FALLING);
		}

		return STATEID_INVALID;
	}

    //
    // FALLING
    //
    void PunchPerformance::STATEFN_ENTER(FALLING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "PunchPerformance::FALLING Entered\n");

        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eFall, NULL);
    }

    StateID PunchPerformance::STATEFN_UPDATE(FALLING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "PunchPerformance::FALLING Updating\n");
        return STATEID_INVALID;
    }

    void PunchPerformance::STATEFN_EXIT(FALLING)(const StateDataDefault& krStateData)
    {
    }

	StateID PunchPerformance::STATEFN_ONEVENT(FALLING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}

    //
    // CLEANUP
    //
    void PunchPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "PunchPerformance::CLEANUP\n");
    }

    StateID PunchPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void PunchPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

	StateID PunchPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
	{
		return STATEID_INVALID;
	}
}
