/*********************************************************\
ExplosionPerformance.cpp
\*********************************************************/

#include "ExplosionPerformance.h"

#include "AnimationComponentPlugInInterface.h"
#include "EuphoriaBehaviorCRC.h"
#include "EuphoriaComp.h"
#include "EuphoriaUtilities.h"
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
    ExplosionStateController ExplosionPerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(ExplosionPerformance, IDLE);
    DEFINE_STATE(ExplosionPerformance, EXPLODING);
    DEFINE_STATE(ExplosionPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sExplosionControllerInitialized = false;

    /*!
     * Constructor
     */
    ExplosionPerformance::ExplosionPerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner)
    {
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    ExplosionPerformance::~ExplosionPerformance()
    {

    }

    void ExplosionPerformance::Start()
    {
        mpStateMachine->ManualTransition(STATEID(EXPLODING));
    }

    /*!
     * Forces the performance to enter the CLEANUP state
     */
    void ExplosionPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    /*!
     * Delegates Think tasks to the individual states of this performance
     */
    void ExplosionPerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    void ExplosionPerformance::OnEvent(const EuphoriaEvent& event)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Explosion NOT handling events\n");
    }

    /*!
     * Sets the configuration for this performance. This typically includes 'constants'
     * that affect the way the performance changes state, such as stationary timer
     * settings.
     *
     * @note Look pros/cons of making this and SetParams template methods.
     */
    void ExplosionPerformance::Initialize(const DefEuphoriaPerformanceExplosionSettings& params)
    {
        mfStationarySpeedThreshold = params.mStationarySpeedThreshold;
        mfStationaryWaitTime = params.mStationaryWaitTime;
    }

    /*!
     * Saves the parameters used by this performance
     *
     * @todo add assignment op to param structures and clean up all perf's SetParams
     */
    void ExplosionPerformance::SetParams(const ExplosionParams& params)
    {
        mParams = params;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    ExplosionStateController& ExplosionPerformance::GetStateController()
    {
        // Run once
        if (!sExplosionControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS(ExplosionPerformance, IDLE));
            msStateController.AddState(ADDSTATE_ARGS(ExplosionPerformance, EXPLODING));
            msStateController.AddState(ADDSTATE_ARGS(ExplosionPerformance, CLEANUP));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sExplosionControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void ExplosionPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID ExplosionPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void ExplosionPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    //
    // EXPLODING
    //
    void ExplosionPerformance::STATEFN_ENTER(EXPLODING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "ExplosionPerformance::EXPLODING ENTERED\n");

        // Set conditions for recovery
        mpOwner->SetStationaryTimer(mfStationarySpeedThreshold, mfStationaryWaitTime);

        // Prepare overrides for the force behavior
        Vec3 vImpulse;
        Vec3Set(vImpulse, mParams.mNormal);

        // @note Hard-Coded tweaks!
        const float npcMassInKG = 75.0f;
        const float fudgeFactor = 5.0f;
        const float maxMagnitude = 100.0f;

        float magImpulse = RavenMath::Sqrt(mParams.mSourceMass / npcMassInKG) * mParams.mVelocity * fudgeFactor;
        Vec3Mul(vImpulse, vImpulse, magImpulse);

        // Keep the force magnitude within limits
        float magnitude = Vec3Mag(vImpulse);
        if (magnitude > maxMagnitude)
        {
            Vec3Mul(vImpulse, vImpulse, maxMagnitude/magnitude);
        }

        // Add some lift to the force
        // Set overrides for force behavior
        ParameterOverrideDataInterface* pData = mpOwner->GetAnimationInterface()->GetPODI();

        pData->SetVector(0, vImpulse.x, vImpulse.y, vImpulse.z);
        pData->SetFloat(0, 1.0f);                                    // Spread
        pData->SetFloat(1, 0.1f);                                    // Duration
        pData->SetPart(0, CRCINIT("Spine2"));
        pData->SetBool(0, false);                                    // Apply total force each frame

        // Start force behavior
        mpOwner->GetAnimationInterface()->StartBehavior(EuphoriaBehaviorCRC::eForce, pData->GetPOD());
    }

    StateID ExplosionPerformance::STATEFN_UPDATE(EXPLODING)(const StateDataDefault& krStateData)
    {
        if (ShouldFallingPerformanceTakeOver(mpOwner))
        {
            FallingParams fp;
            fp.mAttacker = mParams.mAttacker;
            mpOwner->StartPerformance(fp);
        }

        return STATEID_INVALID;
    }

    void ExplosionPerformance::STATEFN_EXIT(EXPLODING)(const StateDataDefault& krStateData)
    {
    }

    //
    // CLEANUP
    //
    void ExplosionPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "ExplosionPerformance::CLEANUP\n");
    }

    StateID ExplosionPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        // SM will always end in CLEANUP and start in IDLE. Does this have to be explicit?
        return STATEID(IDLE); 
    }

    void ExplosionPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }
}
