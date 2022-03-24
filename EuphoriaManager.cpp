/*********************************************************\
EuphoriaComp.cpp

author : VDB
\*********************************************************/

#include "EuphoriaManager.h"
#include "EuphoriaComp.h"
#include "LECAttrib.h"
// Interfaces
#include "PhysicsComponentPlugInInterface.h"
// Schemas
#include "DefEuphoria.schema.h"
//#include "DefEuphoriaPerformance.schema.h"

// For verifying the behavior CRCs
#ifdef _DEBUG
#include "EuphoriaBehaviorCRC.h"
#endif

namespace Ronin
{
    // USING DECLARATIONS
    using lec::LecTraceChannel;
    using namespace RavenMath;

    // GLOBALS
    EuphoriaManager* gpEuphoriaManager = NULL;

    ManagerInterface** gEuphoriaManagerCreator()
    {
        gpEuphoriaManager = lec_new EuphoriaManager();
        return reinterpret_cast<ManagerInterface**>(&gpEuphoriaManager);
    }

#ifdef DLL_Fortune
    extern "C" __declspec( dllexport )   ManagerInterface** Initialize(GamePlugInInterface* )
    {
        return gEuphoriaManagerCreator();
    };
#else
    
#endif  //DLL_Fortune

    /*!
     * Sets the plugin interface, verifies Behavior CRCs in Debug.
     */
    EuphoriaManager::EuphoriaManager(void) : 
        mpFortuneGame(NULL)
    {
        mPlugInInterface.Set(this);

#ifdef _DEBUG
        // check Euphoria Behavior CRC values
        // If any of these assert, check EuphoriaBehaviorCRC.h & BoneCRC.h while you're at it.
        LECASSERT(EuphoriaBehaviorCRC::eBalanceArms == CRCINIT("BalanceArms_BalancingAct"));
        LECASSERT(EuphoriaBehaviorCRC::eBlendToAnim == CRCINIT("BlendToAnimation_Shared"));
        LECASSERT(EuphoriaBehaviorCRC::eBlendToFrame == CRCINIT("BlendToAnimationFrame_Shared"));
        LECASSERT(EuphoriaBehaviorCRC::eBlockWithArms == CRCINIT("BlockWithArms_BalancingAct"));
        LECASSERT(EuphoriaBehaviorCRC::eBreak == CRCINIT("Break_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eCatchFall == CRCINIT("CatchFall_Shoved"));
        LECASSERT(EuphoriaBehaviorCRC::eCrunch == CRCINIT("Crunch_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eCrunch_HR == CRCINIT("Crunch_PerfHitReact2"));
        LECASSERT(EuphoriaBehaviorCRC::eFall == CRCINIT("Fall_Punch"));
        LECASSERT(EuphoriaBehaviorCRC::eFallDown == CRCINIT("FallDown_BalancingAct"));
        LECASSERT(EuphoriaBehaviorCRC::eFeetFirstFall == CRCINIT("FeetFirstFall_DropZone"));
        LECASSERT(EuphoriaBehaviorCRC::eFlail == CRCINIT("FlailThroughAir_DropZone"));
        LECASSERT(EuphoriaBehaviorCRC::eFly == CRCINIT("Fly_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eGrab == CRCINIT("GrabLedge_GrabNGo"));
        LECASSERT(EuphoriaBehaviorCRC::eHang == CRCINIT("Hang_GrabNGo"));
        LECASSERT(EuphoriaBehaviorCRC::eHeadFirstFall == CRCINIT("HeadFirstFall_DropZone"));
        LECASSERT(EuphoriaBehaviorCRC::eHeadHit == CRCINIT("HeadHit_Blend_Fortune"));
        LECASSERT(EuphoriaBehaviorCRC::eImpact == CRCINIT("Impact_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eImpact_HR == CRCINIT("Impact_PerfHitReact2"));
        LECASSERT(EuphoriaBehaviorCRC::eImpactReaction == CRCINIT("ImpactReaction_DropZone"));
        LECASSERT(EuphoriaBehaviorCRC::eLandingFall == CRCINIT("LandingFallWindmill_BalancingAct"));
        LECASSERT(EuphoriaBehaviorCRC::ePunch == CRCINIT("Force_Punch"));
        LECASSERT(EuphoriaBehaviorCRC::eReact == CRCINIT("React_PerfHitReact2"));
        LECASSERT(EuphoriaBehaviorCRC::eSlide == CRCINIT("Slide_GrabNGo"));
        LECASSERT(EuphoriaBehaviorCRC::eStagger == CRCINIT("Stagger_Shoved"));
        LECASSERT(EuphoriaBehaviorCRC::eStaggerBalance == CRCINIT("StaggerBalance6_BalancingAct"));
        LECASSERT(EuphoriaBehaviorCRC::eStaggerPunch == CRCINIT("Stagger_Punch"));
        LECASSERT(EuphoriaBehaviorCRC::eSuperBlend == CRCINIT("SuperBlendToAnimation_Shared"));
        LECASSERT(EuphoriaBehaviorCRC::eTorque == CRCINIT("Torque_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eTransfer == CRCINIT("Transfer_PerfThrow"));
        LECASSERT(EuphoriaBehaviorCRC::eTransfer_HR == CRCINIT("Transfer_PerfHitReact2"));
        LECASSERT(EuphoriaBehaviorCRC::eUnstick == CRCINIT("Blend_Unstick"));
#endif
    }

    //-------------------------------------------------------------------------
    EuphoriaManager::~EuphoriaManager(void)
    {
	    DestroyAll();
    }

    void EuphoriaManager::Initialize(GamePlugInInterface* gameEnginePlugIn)
    {
        mpFortuneGame = reinterpret_cast<GameHooks::FortuneGamePlugInInterface*>(gameEnginePlugIn);
        gameEnginePlugIn->RegisterManager(this, GetEuphoriaPlugInName());
    }

    //-------------------------------------------------------------------------
    void EuphoriaManager::DestroyAll(void)
    {
	    const EuphoriaManagerImplMap::iterator itrEnd = mAllEuphoriaComponents.end();
	    for (EuphoriaManagerImplMap::iterator itr = mAllEuphoriaComponents.begin(); itr != itrEnd; ++itr)
	    {
		    EuphoriaComp *pEuphoriaComp = *itr;
            pEuphoriaComp->Destroy();
            lec_delete pEuphoriaComp;
	    }

	    mAllEuphoriaComponents.clear();
    }

    //-------------------------------------------------------------------------
    EuphoriaComp* EuphoriaManager::CreateComp(RenID id)
    {
        EuphoriaComp* pEuphoriaComp = lec_new EuphoriaComp(id);
	    mAllEuphoriaComponents.insert(pEuphoriaComp);
        return pEuphoriaComp;
    }

    GameHooks::FortuneGamePlugInInterface* EuphoriaManager::GetFortuneGamePlugIn()
    {
        return mpFortuneGame;
    }

    ManagerPlugInInterface* EuphoriaManager::GetInterface()
    {
        return reinterpret_cast<ManagerPlugInInterface*>(&mPlugInInterface);
    }

    void EuphoriaManager::HotLoad(ManagerInterface* mI)
    {
    }

    void EuphoriaManager::HotUnLoad(ManagerInterface* )
    {
    }

    void EuphoriaManager::InitCrossComp(RenID renID)
    {
        // Check for Comp
        EuphoriaComp *pNC = GetEuphoriaComp(renID);

        if(pNC)
        {
            // Hook up Comp's cross-component pointers
            pNC->InitCrossComp();
        }
    }

    void EuphoriaManager::InitPostCrossComp(RenID renID)
    {
        // Check for Comp
        EuphoriaComp *pNC = GetEuphoriaComp(renID);

        if(pNC)
        {
            // Hook up Comp's cross-component pointers
            pNC->InitPostCrossComp();
        }
    }

    /*!
     * Figures out whether or not a Euphoria component is required for this actor
     * and creates it if so.
     *
     * @note Creation of a Euphoria Component is predicated on whether the actor
     *       has been authorized to run performances. This attribute MUST be explicitly
     *       set to false as it defaults to true when it is not specified.
     */
    void EuphoriaManager::CreateActorComp(const AttribMap& ActorAttribs, RenID renID)
    {
        // SHARED ATTRIBUTES (FROM SCHEMA)
        if (AttribMapTagExists(ActorAttribs, ATTRIB_GROUP_EUPHORIA))
        {
            if (AttribMapGetBool(ActorAttribs, ATTRIB_EUPHORIA_CAN_RUN_PERFORMANCES))
            {
                EuphoriaComp* dummy = CreateComp(renID);
                LECUNUSED(dummy);
            }
        }
    }

    //-------------------------------------------------------------------------
    void EuphoriaManager::DestroyComp(RenID renID)
    {
        EuphoriaComp *pEuphoriaComp = GetEuphoriaComp(renID);
        if(pEuphoriaComp)
        {   
            // remove it from the container
            mAllEuphoriaComponents.erase(pEuphoriaComp);

            pEuphoriaComp->Destroy();
	        lec_delete pEuphoriaComp;
        } 
    }


    ///-------------------------------------------------------------------------
    void EuphoriaManager::ThinkAll(float elapsedTime)
    {
        // Only run if not paused
        if (elapsedTime > 0.0f)
        {
            const EuphoriaManagerImplMap::iterator it_end = mAllEuphoriaComponents.end();
            EuphoriaManagerImplMap::iterator it_comp = mAllEuphoriaComponents.begin();
            while (it_comp != it_end)
            {
                EuphoriaComp *pEuphoriaComp = *it_comp;
                LECASSERT(pEuphoriaComp);
                pEuphoriaComp->Think(elapsedTime);
                ++it_comp;
            }
        }
    }

    //-------------------------------------------------------------------------
    void EuphoriaManager::DebugDrawAll()
    { 
        const EuphoriaManagerImplMap::iterator it_end = mAllEuphoriaComponents.end();
        EuphoriaManagerImplMap::iterator it_comp = mAllEuphoriaComponents.begin();
        while (it_comp != it_end)
        {
            EuphoriaComp *pEuphoriaComp = *it_comp;
            LECASSERT(pEuphoriaComp);
            pEuphoriaComp->DebugDraw();
            ++it_comp;
        }
    }

    void EuphoriaManager::WorldLoad(::AssetKey keyWorld, const DefWorld *pWorldDef)
    {
//         AssetKey ak = GetFortuneGamePlugIn()->AttribMapGetAsset(pWorldDef->mAttribs, ATTRIB_NAVMESH);
//         if (!ak)
//             return;
//         AssetHandle handle = GetFortuneGamePlugIn()->LoadAssetKey(ak, ASSET_TYPE_NAVMESH);
//         if (!handle)
//             return;
//         SetNavMesh(handle);
    }

    void EuphoriaManager::WorldUnLoad()
    {
        DestroyAll();
    }

    void EuphoriaManager::AddDebugMenus()
    {
        //mpPrivateMembers->mNavDebugDisplay.AddDebugMenus();   
    }

    
    // EXPOSED MANANGER FUNCTION
    ComponentPlugInInterface* EuphoriaManager::GetComponentInterface(RenID renID)
    {
//        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "GetComponentInterface called!\n");
        EuphoriaComp* pEuphoriaComp = GetEuphoriaComp(renID);
        if (pEuphoriaComp)
        {
            return reinterpret_cast<ComponentPlugInInterface*>(pEuphoriaComp->GetInterface());
        }
        return NULL;
    }

    // EXPOSED MANANGER FUNCTION
    void EuphoriaManager::HandleMessage(const char* msg, RenID target)
    {
//        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EuphoriaManager::HandleMessage called!\n");
    }
}
