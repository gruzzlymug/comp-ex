/*********************************************************\
ShovePerformance.cpp
\*********************************************************/

#include "ShovePerformance.h"
#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaEvent.h"
#include "EuphoriaComp.h"
#include "FortuneGamePlugInInterface.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace RavenMath;
    using namespace Ronin;
    using namespace lec;

    // Define statics
    // State controller
    ShoveStateController ShovePerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(ShovePerformance, IDLE);
    DEFINE_STATE(ShovePerformance, STAGGERING);
    DEFINE_STATE(ShovePerformance, FALLING);
    DEFINE_STATE(ShovePerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sShoveControllerInitialized = false;

    /*!
     * Constructor
     */
    ShovePerformance::ShovePerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner)
    {
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     * Destructor
     */
    ShovePerformance::~ShovePerformance()
    {

    }

    void ShovePerformance::Start()
    {
        mpStateMachine->ManualTransition(STATEID(STAGGERING));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void ShovePerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    /*!
     * @note Maybe this should just be in the base? It is the some in all perfs, yes?
     */
    void ShovePerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    /*!
     * Handles events from the euphoria system for *all* states
     */
    void ShovePerformance::OnEvent(const EuphoriaEvent& event)
    {
        mpStateMachine->OnEvent(event);
    }

    /*!
     * Sets the load-time configuration for this performance. This typically includes 'constants'
     * that affect the way the performance changes state, such as stationary timer
     * settings.
     *
     * @note Look pros/cons of making this and SetParams template methods.
     */
    void ShovePerformance::Initialize(const DefEuphoriaPerformanceShoveSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

	/*!
	 * Sets the run-time input parameters for this performance. This includes variables that
     * make each performance unique such as the force to apply to a particular
     * bone, the bone itself, etc.
	 */
    void ShovePerformance::SetParams(const ShoveParams& rShoveParams)
    {
        mParams = rShoveParams;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    ShoveStateController& ShovePerformance::GetStateController()
    {
        // Run once
        if (!sShoveControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS(ShovePerformance, IDLE));
            msStateController.AddState(ADDSTATE_ARGS(ShovePerformance, STAGGERING));
            msStateController.AddState(ADDSTATE_ARGS(ShovePerformance, FALLING));
            msStateController.AddState(ADDSTATE_ARGS(ShovePerformance, CLEANUP));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sShoveControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void ShovePerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID ShovePerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void ShovePerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID ShovePerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // STAGGERING
    //
    void ShovePerformance::STATEFN_ENTER(STAGGERING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Shove::STAGGERING\n");

        // Set conditions for recovery
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);

        // Determine 'shove vector'
        Vec3 posAttacker = mpOwner->GetRenInterface()->RenGetPosition(mParams.mAttacker);
        Vec3 posShovee = mpOwner->GetRenInterface()->RenGetPosition(mpOwner->GetRenID());
        Vec3 shoveDir;
        Vec3Sub_UA(shoveDir, posShovee, posAttacker);
        // @todo ASSERT? or check for 0 length?
        Vec3Normalize_UA(shoveDir, shoveDir);

        // Set overrides for stagger behavior
        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();
//        pData->SetVector(0, shoveDir.x, shoveDir.y, shoveDir.z);
        pData->SetVector(0, 0, 0, 0);

        // Start stagger behavior
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eStagger, pData->GetPOD());
    }

    StateID ShovePerformance::STATEFN_UPDATE(STAGGERING)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void ShovePerformance::STATEFN_EXIT(STAGGERING)(const StateDataDefault& krStateData)
    {
        // @todo Make sure StopAllBehaviors is only called when necessary. It seems to be in a lot of places (in the Lua versions)
        // @todo It can probably be replaced with StopBehavior() in most cases
        mpOwner->GetAnimationInterface()->StopBehavior(EuphoriaBehaviorCRC::eStagger);
    }

    StateID ShovePerformance::STATEFN_ONEVENT(STAGGERING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        switch (event.GetType())
        {
        case EuphoriaEvent::eBehaviorStaggerTrip:
            return STATEID(FALLING);
        }

        return STATEID_INVALID;
    }

    //
    // FALLING
    //
    void ShovePerformance::STATEFN_ENTER(FALLING)(const StateDataDefault& krStateData)
    {
        // @note This state doesn't seem to get used much (if at all)
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Shove::FALLING\n");

        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eCatchFall, NULL);
    }

    StateID ShovePerformance::STATEFN_UPDATE(FALLING)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void ShovePerformance::STATEFN_EXIT(FALLING)(const StateDataDefault& krStateData)
    {
        mpOwner->GetAnimationInterface()->StopBehavior(EuphoriaBehaviorCRC::eCatchFall);
    }

    StateID ShovePerformance::STATEFN_ONEVENT(FALLING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // CLEANUP
    //
    void ShovePerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Shove::CLEANUP\n");

        // @todo Make sure StopAllBehaviors is only called when necessary. It seems to be in a lot of places (in the Lua versions)
        mpOwner->GetAnimationInterface()->StopAllBehaviors();
    }

    StateID ShovePerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void ShovePerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

    StateID ShovePerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }
}
