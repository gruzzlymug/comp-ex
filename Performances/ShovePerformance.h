/*********************************************************\
ShovePerformance.h
\*********************************************************/

#ifndef EUPHORIA_SHOVE_PERFORMANCE_H
#define EUPHORIA_SHOVE_PERFORMANCE_H

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceShoveSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class ShovePerformance;

    // TYPEDEFS
    typedef Ronin::StateMachine<ShovePerformance> ShoveStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class ShovePerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static const crc32_t msDummyBehavior;
        static const crc32_t msStaggerBehavior;
        static const crc32_t msCatchFallBehavior;
        static ShoveStateController msStateController;

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
        ShoveParams mParams;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        ShovePerformance(const ShovePerformance&);
        ShovePerformance& operator=(const ShovePerformance&);

        // METHODS
        ShoveStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(STAGGERING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FALLING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit ShovePerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~ShovePerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
        virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceShoveSettings& params);
        void SetParams(const ShoveParams& rShoveParams);

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_SHOVE_PERFORMANCE_H
#endif
