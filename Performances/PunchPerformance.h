#ifndef EUPHORIA_PUNCH_PERFORMANCE_H
#define EUPHORIA_PUNCH_PERFORMANCE_H

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformancePunchSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class PunchPerformance;

    // TYPEDEFS
    typedef Ronin::StateMachine<PunchPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> PunchStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class PunchPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static PunchStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        PunchParams mParams;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        PunchPerformance(const PunchPerformance&);
        PunchPerformance& operator=(const PunchPerformance&);

        // METHODS
        PunchStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(ON_FEET, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FALLING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit PunchPerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~PunchPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformancePunchSettings& params);
        void SetParams(const PunchParams& params);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_PUNCH_PERFORMANCE
#endif
