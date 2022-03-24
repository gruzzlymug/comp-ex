#ifndef EUPHORIA_THROW_PERFORMANCE
#define EUPHORIA_THROW_PERFORMANCE

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"
#include "LECAlign.h"
#include "crc32.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceThrowSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class GrabbedEdgeInfo;
    class ThrowPerformance;

    // TYPEDEFS
	typedef Ronin::StateMachine<ThrowPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> ThrowStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class ThrowPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static ThrowStateController msStateController;
		static const float msCheckCollisionDelay;
		static const float msCheckTransferDelay;
		static const float msCrunchDelay;

		// MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
		ThrowParams mParams;

        // Reference to the owning component's GrabbedEdgeInfo for convenience
        GrabbedEdgeInfo& mGrabbedEdge;

        float mCheckStartTime;
		float mCrunchStartTime;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        ThrowPerformance(const ThrowPerformance&);
        ThrowPerformance& operator=(const ThrowPerformance&);

        // METHODS
        ThrowStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(FLY, const EuphoriaEvent);
        DECLARE_STATE_EVENT(BREAK, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CRUNCH, const EuphoriaEvent);
        DECLARE_STATE_EVENT(IMPACT, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit ThrowPerformance(Ronin::EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo);
        ~ThrowPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
		virtual void Think(float elapsedTime);
		virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceThrowSettings& params);
		void SetParams(const ThrowParams& rParams);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_THROW_PERFORMANCE
#endif
