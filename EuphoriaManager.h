/*********************************************************\
EuphoriaManager.h
\*********************************************************/

#ifndef _EuphoriaManager_H
#define _EuphoriaManager_H

#include "EuphoriaConstants.h"
#include "EuphoriaComp.h"

#include "EuphoriaManagerPlugInInterface.h"

#include "ManagersRegistryInterface.h"
#include "../include/FortuneGamePlugInInterface.h"

namespace Ronin
{
    // FORWARD DECLARATIONS
    class EuphoriaCompInterface;
    class EuphoriaComp;
    
    class EuphoriaManager : public ManagerInterface
    {
        // FRIENDS
        friend class EuphoriaManagerPlugIn;

    public:
        EuphoriaManager(void);
        virtual ~EuphoriaManager(void);

        virtual void DestroyAll(void);
        virtual void ThinkAll(float elapsedTime);
        virtual void DebugDrawAll(void);

        virtual void CreateActorComp(const AttribMap& ActorAttribs, RenID renID);
        virtual void InitCrossComp(RenID renID);
        virtual void InitPostCrossComp(RenID renID);
        virtual void DestroyComp(RenID renID);

        virtual void Initialize(GamePlugInInterface* plug);
        virtual void HotLoad(ManagerInterface*);
        virtual void HotUnLoad(ManagerInterface*);
        virtual ManagerPlugInInterface* GetInterface();
        
        void WorldLoad(::AssetKey keyWorld, const DefWorld *pWorldDef);
        void WorldUnLoad();
        void AddDebugMenus();

        inline EuphoriaComp* GetEuphoriaComp(RenID renID);
        
        //Euphoria Components
        
        GameHooks::FortuneGamePlugInInterface*     GetFortuneGamePlugIn();

        // Exposed functions in the Manager Interface
        virtual ComponentPlugInInterface* GetComponentInterface(RenID renID);
        void HandleMessage(const char* msg, Ronin::RenID target);

    private:
        GameHooks::FortuneGamePlugInInterface*         mpFortuneGame;

        RONIN_DEFINE_SORTED_COMPONENT_MAP_PUBLIC( EuphoriaComp, EuphoriaManagerImplMap, mAllEuphoriaComponents)

        EuphoriaManagerPlugIn   mPlugInInterface;
        EuphoriaComp* CreateComp(RenID renID);
    };

    extern EuphoriaManager *gpEuphoriaManager;

    EuphoriaComp* EuphoriaManager::GetEuphoriaComp(RenID renID)
    { 
        return mAllEuphoriaComponents.find(renID);
    }
}

#define PLUGIN_IMPL_EuphoriaManager
#include "EuphoriaManagerPlugInInterface.h"
#undef  PLUGIN_IMPL_EuphoriaManager

#endif // _EuphoriaManager_H
