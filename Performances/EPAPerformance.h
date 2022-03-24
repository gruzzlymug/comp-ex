#ifndef EUPHORIA_EPA_PERFORMANCE
#define EUPHORIA_EPA_PERFORMANCE

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"
#include "LECAlign.h"
#include "crc32.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceEPASettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class GrabbedEdgeInfo;
    class EPAPerformance;

    // TYPEDEFS
	typedef Ronin::StateMachine<EPAPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> EPAStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class EPAPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    private:
        // STATIC MEMBERS
        static EPAStateController msStateController;

		// MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
		EPAParams mParams;

        // Reference to the owning component's GrabbedEdgeInfo for convenience
        GrabbedEdgeInfo& mGrabbedEdge;

        float mCheckStartTime;
		float mCrunchStartTime;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        EPAPerformance(const EPAPerformance&);
        EPAPerformance& operator=(const EPAPerformance&);

        // METHODS
        EPAStateController& GetStateController();

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
        DECLARE_STATE_EVENT(RUNNING, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit EPAPerformance(Ronin::EuphoriaComp* pOwner, GrabbedEdgeInfo& grabbedEdgeInfo);
        ~EPAPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
		virtual void Think(float elapsedTime);
		virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceEPASettings& params);
		void SetParams(const EPAParams& params);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_EPA_PERFORMANCE
#endif
