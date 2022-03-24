#include "EuphoriaUtilities.h"
#include "BoneCRC.h"
#include "AnimationComponentPlugInInterface.h"
#include "EdgeSystemManagerPlugInInterface.h"
#include "EuphoriaComp.h"
#include "EuphoriaManager.h"
#include "EuphoriaStruct.h"
#include "FortuneGamePlugInInterface.h"
#include "PhysicsComponentPlugInInterface.h"

namespace euphoria
{
    // USING DIRECTIVES
    using namespace RavenMath;
    using namespace Ronin;

    /*!
     * Returns current speed of the euphoria character based on the Spine 0 bone
     *
     * @note Returns velocity and speed
     */
    float GetSpeedAndVelocity(const EuphoriaComp* const pEuphoriaComp, Vec3& outVelocity)
    {
        AnimationCompPlugInInterface* pAnimComp = pEuphoriaComp->GetAnimationInterface();
        LECASSERT(pAnimComp);

        RoninPhysicsEntity* pRPE = pAnimComp->GetBodyPartByBone(BoneCRC::eSpine0);
        LECASSERT(pRPE);

        Vec3 currentVel;
        pRPE->GetLinearVelocity(currentVel);
        float fSpeed = Vec3Mag(currentVel);
        Vec3Set(outVelocity, currentVel);

        return fSpeed;
    }

    /*!
     * Is the character is falling?
     *
     * Checks the ren's velocity against gravity up...if the angle is greater than
     * 45 degrees then we should switch into the Falling performance
     *
     * @note Might want to incorporate some speed threshold or distance to ground checks
     */
    bool ShouldFallingPerformanceTakeOver(const EuphoriaComp* const pEuphoriaComp)
    {
        Vec3 velocity;
        float speed = GetSpeedAndVelocity(pEuphoriaComp, velocity);

        // Check the magnitude of the velocity
        if (speed < 1.0f)
        {
            return false;
        }

        Vec3 safety = { 0, 0, 0 };
        Vec3NormalizeSafe(velocity, velocity, safety);

        // The angle between the velocity and the down vector must be less than
        // 45 degrees.
        Vec3 down = { 0, -1, 0 };
        float cosAngle = Vec3Dot(velocity, down);
        if (cosAngle > KCOS45)
        {
            return true;
        }

        return false;
    }

    /*!
     * @note This is specific for 2-armed NPCs only which is not optimal (see todo below)
     *
     * @todo Refactor this so that it only 'grabs' with one appendage and push logic to calling function(s)
     */
    bool TryToGrabNearbyEdges(EuphoriaComp* pOwner, GrabbedEdgeInfo& outGrabbedEdge)
    {
        AnimationCompPlugInInterface* pAnimComp = pOwner->GetAnimationInterface();

        // Left hand
        Vec3 posLeftHand;
        pAnimComp->GetBodyPartByBone(BoneCRC::eLeftHand0)->GetPosition(posLeftHand);
        Vec3 vReachForPosLeft;
        float unusedDistanceLeft;       //! @note this value is not used
        bool bFoundEdgeLeft = SearchForGrabbableEdge(posLeftHand, outGrabbedEdge, vReachForPosLeft, unusedDistanceLeft);
        if (bFoundEdgeLeft)
        {
            pOwner->ConstrainLimb(eLeftArm, outGrabbedEdge.mpOwnerRPE, vReachForPosLeft);
            bool bGrabbedSomething = pOwner->IsHandConstrained(eLeftArm);
            if (bGrabbedSomething)
            {
                return true;
            }
        }

        // Try the right hand if the left didn't succeed
        Vec3 posRightHand;
        pAnimComp->GetBodyPartByBone(BoneCRC::eRightHand0)->GetPosition(posRightHand);
        Vec3 vReachForPosRight;
        float unusedDistanceRight;       //! @note this value is not used
        bool bFoundEdgeRight = SearchForGrabbableEdge(posRightHand, outGrabbedEdge, vReachForPosRight, unusedDistanceRight);
        if (bFoundEdgeRight)
        {
            pOwner->ConstrainLimb(eRightArm, outGrabbedEdge.mpOwnerRPE, vReachForPosRight);
            bool bGrabbedSomething = pOwner->IsHandConstrained(eRightArm);
            if (bGrabbedSomething)
            {
                return true;
            }
        }

        return false;
    }

    /*!
     * @note This constant used to be a variable in the FallingPerformance but since
     *       grabbing can be done multiple places it's moved here until the data-driving of
     *       euphoria is in place.
     */
    namespace
    {
        static float skMaxReachDistance = 0.3f;
    }

    /*!
     * Looks for an edge within range to grab. Only one edge is needed to start grabbing.
     *
     * @note Edge Types SHOULD be specified, not kAny
     * @todo this should return the edge handle, not the distance? probably shouldn't return anything, actually...
     */
    bool SearchForGrabbableEdge(const Vec3& posGrabbingHand, GrabbedEdgeInfo& outGrabbedEdge, Vec3& outClosestPoint, float& outDistanceToEdge)
    {
        outDistanceToEdge = FLT_MAX;

        const uint32 maxEdges = 1;
        EdgeHandle edges[maxEdges];

        GameHooks::FortuneGamePlugInInterface* pEngineInterface = gpEuphoriaManager->GetFortuneGamePlugIn();
        EdgeSystemManagerPlugInInterface* pEdgeMgr = pEngineInterface->GetEdgeSystemInterface();
        uint32 numEdgesFound = pEdgeMgr->FindEdgesInRadius(edges, maxEdges, posGrabbingHand, skMaxReachDistance, EdgeSystem::kAny);
        if (numEdgesFound > 0)
        {
            outGrabbedEdge.mHandle = edges[0];
            pEdgeMgr->GetEdgePosition(outGrabbedEdge.mHandle, outGrabbedEdge.mvStart, outGrabbedEdge.mvEnd);

            outDistanceToEdge = pEdgeMgr->DistanceToEdge(outGrabbedEdge.mHandle, posGrabbingHand, outClosestPoint);

            // Check to see whether this edge is attached to a physics object
            RenID edgeOwner = pEdgeMgr->GetEdgeRen(outGrabbedEdge.mHandle);
            PhysicsCompPlugInInterface* pPhysicsComp = pEngineInterface->GetPhysicsCompInterface(edgeOwner);
            if (pPhysicsComp != NULL)
            {
                // Get the physics comp of what is to be grabbed
                RoninPhysicsEntity* pGrabbableEntity = pPhysicsComp->GetFirstPhysicsBody();
                LECASSERT(pGrabbableEntity);

                // If the object is physical, convert edges to local space
                RavenMath::Mat44 EntityInverseTransform;
                pGrabbableEntity->GetTransform(EntityInverseTransform);

                Mat44Invert(EntityInverseTransform, EntityInverseTransform);
                Mat44MulPoint3(outGrabbedEdge.mvStart, EntityInverseTransform, outGrabbedEdge.mvStart);
                Mat44MulPoint3(outGrabbedEdge.mvEnd, EntityInverseTransform, outGrabbedEdge.mvEnd);

                // Save to the out variable
                outGrabbedEdge.mpOwnerRPE = pGrabbableEntity;
            }
            return true;
        }

        return false;
    }
}
