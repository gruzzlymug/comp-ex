/*********************************************************\
GunshotPerformance.h
\*********************************************************/

#ifndef EUPHORIA_GUNSHOT_PERFORMANCE_H
#define EUPHORIA_GUNSHOT_PERFORMANCE_H

#include "EuphoriaPerformance.h"
#include "EuphoriaParams.h"
#include "StateController.h"

namespace euphoria
{
    // FORWARD DECLARATIONS
    class GunshotPerformance;

    // TYPEDEFS
    typedef Ronin::StateMachine<GunshotPerformance> GunshotStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class  GunshotPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static GunshotStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        GunshotParams mParams;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        GunshotPerformance(const GunshotPerformance&);
        GunshotPerformance& operator=(const GunshotPerformance&);

        // METHODS
        GunshotStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FALLING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit GunshotPerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~GunshotPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void SetParams(const GunshotParams& rGunshotParams);

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_GUNSHOT_PERFORMANCE_H
#endif
