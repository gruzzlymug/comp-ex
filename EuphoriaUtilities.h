#ifndef EUPHORIA_EUPHORIA_UTILITIES_H
#define EUPHORIA_EUPHORIA_UTILITIES_H

// FORWARD DECLARATIONS
class RoninPhysicsEntity_Havok;
typedef RoninPhysicsEntity_Havok	    RoninPhysicsEntity;

namespace RavenMath
{
    // FORWARD DECLARATIONS
    struct Vec4;

    typedef Vec4 Vec3;
}

namespace Ronin
{
    // FORWARD DECLARATIONS
    class EuphoriaComp;
}

namespace euphoria
{
    // FORWARD DECLARATIONS
    class GrabbedEdgeInfo;

    float GetSpeedAndVelocity(const Ronin::EuphoriaComp* const pEuphoriaComp, RavenMath::Vec3& outVelocity);
    bool ShouldFallingPerformanceTakeOver(const Ronin::EuphoriaComp* const pEuphoriaComp);
    bool TryToGrabNearbyEdges(Ronin::EuphoriaComp* pOwner, GrabbedEdgeInfo& outGrabbedEdge);
    bool SearchForGrabbableEdge(const RavenMath::Vec3& posGrabbingHand, GrabbedEdgeInfo& outGrabbedEdge, RavenMath::Vec3& outClosestPoint, float& outDistanceToEdge);
}

// EUPHORIA_EUPHORIA_UTILITIES_H
#endif
