/*********************************************************\
ExplosionPerformance.h
\*********************************************************/

#ifndef EUPHORIA_EXPLOSION_PERFORMANCE_H
#define EUPHORIA_EXPLOSION_PERFORMANCE_H

#include "EuphoriaPerformance.h"
#include "EuphoriaParams.h"
#include "StateController.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceExplosionSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class ExplosionPerformance;

    // TYPEDEFS
    typedef Ronin::StateMachine<ExplosionPerformance> ExplosionStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class ExplosionPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static ExplosionStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        ExplosionParams mParams;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        ExplosionPerformance(const ExplosionPerformance&);
        ExplosionPerformance& operator=(const ExplosionPerformance&);

        // METHODS
        ExplosionStateController& GetStateController();

        // STATES
        DECLARE_STATE(IDLE);
        DECLARE_STATE(EXPLODING);
        DECLARE_STATE(CLEANUP);

    public:
        // CREATORS
        explicit ExplosionPerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~ExplosionPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceExplosionSettings& params);
        void SetParams(const ExplosionParams& params);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_EXPLOSION_PERFORMANCE_H
#endif
