/*********************************************************\
EuphoriaPerformance.h
\*********************************************************/

#ifndef EUPHORIA_PERFORMANCE_H
#define EUPHORIA_PERFORMANCE_H

#include "LECAlign.h"
#include "crc32.h"

namespace Ronin
{
    // FORWARD DECLARATIONS
    class EuphoriaComp;
}

namespace euphoria
{
    // FORWARD DECLARATIONS
    class EuphoriaEvent;

    /*!
     *
     */
    DECLARE_ALIGNED class EuphoriaPerformance
    {
        LECALIGNEDCLASS(16);

    protected:
        // MEMBERS
        Ronin::EuphoriaComp* mpOwner;

        // For recovery to hard-keyed animation
        float mfStationarySpeedThreshold;
        float mfStationaryWaitTime;

    public:
        // CREATORS
        EuphoriaPerformance(Ronin::EuphoriaComp* pOwner);
        virtual ~EuphoriaPerformance();

        // MANIPULATORS
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Think(float elapsedTime) = 0;
		virtual void OnEvent(const EuphoriaEvent& event) = 0;
        
        // ACCESSORS
    } END_DECLARE_ALIGNED;
}

// EUPHORIA_PERFORMANCE_H
#endif
