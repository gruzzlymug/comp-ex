#ifndef EUPHORIA_BLEND_PERFORMANCE_H
#define EUPHORIA_BLEND_PERFORMANCE_H

#include "EuphoriaParams.h"
#include "EuphoriaPerformance.h"
#include "StateController.h"
#include "LECAlign.h"

// FORWARD DECLARATIONS
struct DefEuphoriaPerformanceBlendSettings;

namespace euphoria
{
    // FORWARD DECLARATIONS
    class BlendPerformance;

    // TYPEDEFS
	typedef Ronin::StateMachine<BlendPerformance, const Ronin::StateDataDefault, const EuphoriaEvent> BlendStateController;

    /*!
     *
     */
    DECLARE_ALIGNED class BlendPerformance : public EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

        enum Phase
        {
            ePhaseNone,
            ePhaseOne,
            ePhaseTwo,
        };

        struct PhaseOneParams
        {
            float bodyStiffness;
            float effectorRampDuration;   // this is a float but for some reason the NM behavior takes an int
            float coreBlendDuration;      // this is a float but for some reason the NM behavior takes an int
            char group[64];
            RavenMath::Vec3 offVec;
        };

        struct PhaseTwoParams
        {
            float effectorRampDuration;   // this is a float but for some reason the NM behavior takes an int
            float coreBlendDuration;      // this is a float but for some reason the NM behavior takes an int
            float bodyBlendStart;         // this is a float but for some reason the NM behavior takes an int
            float bodyBlendDuration;      // this is a float but for some reason the NM behavior takes an int
            char group[64];
        };

    private:
        // STATIC MEMBERS
		static BlendStateController msStateController;
        static const crc32_t msParameterizedChoreTable[eNumGetups];
        static const crc32_t msParameterizedBlockTable[eNumGetups];
        static const crc32_t msNonParameterizedChoreTable[eNumGetups];
        static const crc32_t msNonParameterizedBlockTable[eNumGetups];

        // MEMBERS
        Ronin::StateMachineInstance* mpStateMachine;
		BlendParams mParams;

        // Blending members
		Phase mBlendPhase;
		float mBlendStartTime;
		float mBlendDuration;		// This is called mfBlendToAnimationTime in AnimationComponent
		float mBlendPhaseTwoStart;	// This is called mfBlendToAnimationPhaseTwoStart in AnimationComponent

		// Parameters
		PhaseOneParams mPhaseOne;
		PhaseTwoParams mPhaseTwo;

        // UNIMPLEMENTED METHODS
        // Prevent copying and assignment
        BlendPerformance(const BlendPerformance&);
        BlendPerformance& operator=(const BlendPerformance&);

        // METHODS
        BlendStateController& GetStateController();

		// helpers
		bool CanRecoverThink();
		Ronin::StateID Rethink(bool unstick);
		bool UpdateBlendToAnimation();
		bool IsHeadBlocked();
		float InitiateBlend(float fGetupBlendTime, GetupPosition ePosition, float turnCorrect, const RavenMath::Vec3& offVec);
		void GetExitDirection(float proxRange, RavenMath::Vec3* pOutVec);
		void CorrectForWallLeaning(GetupPosition ePosition, GetupPosition* pOutPosition, RavenMath::Vec3* pOutVec);
		float CalcBlockParams(Ronin::RenID targetRenID, GetupPosition position);
		bool CheckRenOrient(GetupPosition ePosition, bool up);
		GetupPosition DetermineGetupChore(GetupPosition ePosition, GetupPosition cPosition);
		void DetermineGetupChoreSimple(GetupPosition position);
		void FindParameterizedChore(GetupPosition position, crc32_t* pChoreCRC, crc32_t* pBlockCRC);
		void FindNonParameterizedChore(GetupPosition position, crc32_t* pChoreCRC, crc32_t* pBlockCRC);

        // STATES
        DECLARE_STATE_EVENT(IDLE, const EuphoriaEvent);
		DECLARE_STATE_EVENT(TRANSITIONING, const EuphoriaEvent);
		DECLARE_STATE_EVENT(INTERRUPT, const EuphoriaEvent);
		DECLARE_STATE_EVENT(BONKED, const EuphoriaEvent);
		DECLARE_STATE_EVENT(STUCK, const EuphoriaEvent);
		DECLARE_STATE_EVENT(BLENDTO, const EuphoriaEvent);
        DECLARE_STATE_EVENT(CLEANUP, const EuphoriaEvent);

    public:
        // CREATORS
        explicit BlendPerformance(Ronin::EuphoriaComp* pOwner);
        ~BlendPerformance();

        // MANIPULATORS
        virtual void Start();
        virtual void Stop();
        virtual void Think(float elapsedTime);
		virtual void OnEvent(const EuphoriaEvent& event);

        void Initialize(const DefEuphoriaPerformanceBlendSettings& params);
		void SetParams(const BlendParams& params);

        // ACCESSORS

    } END_DECLARE_ALIGNED;
}

// EUPHORIA_BLEND_PERFORMANCE_H
#endif
