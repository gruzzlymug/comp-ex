/*********************************************************\
GunshotPerformance.cpp
\*********************************************************/

#include "GunshotPerformance.h"
#include "EuphoriaComp.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace lec;
    using namespace Ronin;

    // Define statics
    // Behaviors
    // StateController
    GunshotStateController GunshotPerformance::msStateController;

    // Define all of the states for this Performance
    DEFINE_STATE(GunshotPerformance, IDLE);
    DEFINE_STATE(GunshotPerformance, FALLING);
    DEFINE_STATE(GunshotPerformance, CLEANUP);

    // This is not in GetStateController on purpose
    static bool sGunshotControllerInitialized = false;

    /*!
     * Constructor
     */
    GunshotPerformance::GunshotPerformance(EuphoriaComp* pOwner)
        : EuphoriaPerformance(pOwner)
    {
        mpStateMachine = GetStateController().CreateInstance(this);
    }

    /*!
     * Destructor
     */
    GunshotPerformance::~GunshotPerformance()
    {

    }

    void GunshotPerformance::Start()
    {
        mpStateMachine->ManualTransition(STATEID(FALLING));
    }

    /*!
    * Forces this performance to stop by forcing it to enter cleanup
    */
    void GunshotPerformance::Stop()
    {
        mpStateMachine->ManualTransition(STATEID(CLEANUP));
    }

    void GunshotPerformance::Think(float elapsedTime)
    {
        mpStateMachine->Advance(elapsedTime);
    }

    void GunshotPerformance::OnEvent(const EuphoriaEvent& event)
    {
        mpStateMachine->OnEvent(event);
    }

    /*!
     *
     */
    void GunshotPerformance::SetParams(const GunshotParams& rGunshotParams)
    {
        mParams.mAttacker = rGunshotParams.mAttacker;
    }

    /*!
     * Returns the local static copy of the state controller. Initializes it if
     * necessary.
     */
    GunshotStateController& GunshotPerformance::GetStateController()
    {
        // Run once
        if (!sGunshotControllerInitialized)
        {
            // Attach state functions to the state
            msStateController.AddState(ADDSTATE_ARGS(GunshotPerformance, IDLE));
            msStateController.AddState(ADDSTATE_ARGS(GunshotPerformance, FALLING));
            msStateController.AddState(ADDSTATE_ARGS(GunshotPerformance, CLEANUP));
            // Set initial state
            msStateController.SetInitialState(STATEID(IDLE));
            // Only initialize once!
            sGunshotControllerInitialized = true;
        }

        return msStateController;
    }

    //
    // IDLE
    //
    void GunshotPerformance::STATEFN_ENTER(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID GunshotPerformance::STATEFN_UPDATE(IDLE)(const StateDataDefault& krStateData)
    {
        return STATEID(FALLING);
    }

    void GunshotPerformance::STATEFN_EXIT(IDLE)(const StateDataDefault& krStateData)
    {
    }

    StateID GunshotPerformance::STATEFN_ONEVENT(IDLE)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // FALLING
    //
    void GunshotPerformance::STATEFN_ENTER(FALLING)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Gunshot::FALLING\n");
    }

    StateID GunshotPerformance::STATEFN_UPDATE(FALLING)(const StateDataDefault& krStateData)
    {
        FallingParams fp;
        fp.mAttacker = RENID_NULL;
        mpOwner->StartPerformance(fp);

        return STATEID_INVALID;
    }

    void GunshotPerformance::STATEFN_EXIT(FALLING)(const StateDataDefault& krStateData)
    {
    }

    StateID GunshotPerformance::STATEFN_ONEVENT(FALLING)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }

    //
    // CLEANUP
    //
    void GunshotPerformance::STATEFN_ENTER(CLEANUP)(const StateDataDefault& krStateData)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Gunshot::CLEANUP\n");
    }

    StateID GunshotPerformance::STATEFN_UPDATE(CLEANUP)(const StateDataDefault& krStateData)
    {
        return STATEID_INVALID;
    }

    void GunshotPerformance::STATEFN_EXIT(CLEANUP)(const StateDataDefault& krStateData)
    {
    }

    StateID GunshotPerformance::STATEFN_ONEVENT(CLEANUP)(const StateDataDefault& krStateData, const EuphoriaEvent& event)
    {
        return STATEID_INVALID;
    }
}
