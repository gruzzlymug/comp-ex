/*********************************************************\
BalancePerformance.h
\*********************************************************/

#ifndef EUPHORIA_BALANCE_PERFORMANCE_H
#define EUPHORIA_BALANCE_PERFORMANCE_H

#include "EuphoriaPerformance.h"
#include "EuphoriaParams.h"
#include "RenTypes.h"
#include "StateController.h"

namespace euphoria
{
    // FORWARD DECLARATIONS
    class BalancePerformance;

    // TYPEDEFS
    typedef Ronin::StateMachine<BalancePerformance, const Ronin::StateDataDefault, const EuphoriaEvent> BalanceStateController;

    /*!
     * This performance will be based on this: kIn_LandingBalance from SCUM
     */
    DECLARE_ALIGNED class BalancePerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static BalanceStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        BalanceParams mParams;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        BalancePerformance(const BalancePerformance&);
        BalancePerformance& operator=(const BalancePerformance&);

        // METHODS
        BalanceStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(BALANCING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FALLING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit BalancePerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~BalancePerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void SetParams(const BalanceParams& rBalanceParams);

        // ACCESSORS
        bool IsFalling(Ronin::RenID renID, bool bIsSupported, bool bIsMovingFast) const;

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_BALANCE_PERFORMANCE_H
#endif
