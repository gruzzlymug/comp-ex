#ifndef EUPHORIA_HITREACT_PERFORMANCE
#define EUPHORIA_HITREACT_PERFORMANCE

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"
#include "LECAlign.h"
#include "crc32.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceHitReactSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class HitReactPerformance;

    // TYPEDEFS
	typedef Ronin::StateMachine<HitReactPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> HitReactStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class HitReactPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static HitReactStateController msStateController;
        static const float msStaggerMomentumTolerance;
        static const float msRecoverDelay;
        static const float msCrunchDelay;

		// MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
		HitReactParams mParams;
        Ronin::RenID mIncomingRenID;
        bool mRecoverTransfer;
        float mRecoverStartTime;
        float mCrunchStartTime;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        HitReactPerformance(const HitReactPerformance&);
        HitReactPerformance& operator=(const HitReactPerformance&);

        // METHODS
        HitReactStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(REACT, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CRUNCH, const EuphoriaEvent);
        DECLARE_STATE_EVENT(IMPACT, const EuphoriaEvent);
        DECLARE_STATE_EVENT(STAGGER, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit HitReactPerformance(Ronin::EuphoriaComp* pOwner);
        ~HitReactPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
		virtual void Think(float elapsedTime);
		virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceHitReactSettings& params);
		void SetParams(const HitReactParams& params);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_HITREACT_PERFORMANCE
#endif
