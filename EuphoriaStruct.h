#ifndef _EuphoriaStructs_H_
#define _EuphoriaStructs_H_
#pragma once

#include "EdgeSystemTypes.h"
#include "RavenMath.h"
#include "Rens/RenTypes.h"

// FORWARD DECLARATIONS
class RoninPhysicsEntity_Havok;
typedef RoninPhysicsEntity_Havok	    RoninPhysicsEntity;

namespace euphoria
{
    // ENUMERATIONS
    enum GetupPosition
    {
        eGetupNone = 0,
        eGetupFront,
        eGetupBack,
        eGetupLeft,
        eGetupRight,
        eGetupUpright,
        eGetupDead,
        eNumGetups
    };

    /*!
     * These values are used to indicate limbs which can grab edges in the game
     * world.
     */
    enum ReachingArm
    {
        eLeftArm,
        eRightArm,
        eNumArms        //! "number of appendages that can grab"
    };

    /*!
     * Used to pass information about grabbed edges between performances and the
     * actual code that does the edge searching
     */
    class GrabbedEdgeInfo
    {
    private:
        // NOT IMPLEMENTED
        GrabbedEdgeInfo(const GrabbedEdgeInfo& rhs);

    public:
        RavenMath::Vec3 mvStart;                                                //!< Position of edge start when hanging (in world space, or local when attached to a physics object)
        RavenMath::Vec3 mvEnd;                                                  //!< Position of edge end when hanging
        Ronin::EdgeHandle mHandle;
        RoninPhysicsEntity* mpOwnerRPE;                                         //!< RPE to which an embedded edge may be attached

        inline GrabbedEdgeInfo();
        inline GrabbedEdgeInfo& operator=(const GrabbedEdgeInfo& rhs);
    };

    GrabbedEdgeInfo::GrabbedEdgeInfo()
        : mHandle(Ronin::EDGE_HANDLE_INVALID), mpOwnerRPE(NULL)
    {
        Vec3SetZero(mvStart);
        Vec3SetZero(mvEnd);
    }

    GrabbedEdgeInfo& GrabbedEdgeInfo::operator=(const GrabbedEdgeInfo& rhs)
    {
        if (this != &rhs)
        {
            Vec3Set(mvStart, rhs.mvStart);
            Vec3Set(mvEnd, rhs.mvEnd);
            mHandle = rhs.mHandle;
            mpOwnerRPE = rhs.mpOwnerRPE;
        }

        return *this;
    }
}

// _EuphoriaStructs_H_
#endif
