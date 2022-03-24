#include "EuphoriaPerformance.h"

namespace euphoria
{
    // USING DECLARATIONS
    using namespace Ronin;

    /*!
     * Initializes the component that owns this performance. Sets the default
     * recovery parameters.
     *
     * @note Default recovery parameters are set such that the NPC will NOT recover.
     */
    EuphoriaPerformance::EuphoriaPerformance(EuphoriaComp* pOwner)
        : mpOwner(pOwner), mfStationarySpeedThreshold(0.0f), mfStationaryWaitTime(0.0f)
    {
    }

    /*!
     *
     */
    EuphoriaPerformance::~EuphoriaPerformance()
    {
    }
}
