/*********************************************************\
EuphoriaManagerPlugInInterface.h
\*********************************************************/

#ifndef _EuphoriaManagerPlugInInterface_H_
#define _EuphoriaManagerPlugInInterface_H_

#define DLL_EUPHORIA

//
// We define the 2 classes that helps us to build the plugIn Interface model : the virtual interface (xxxPlugin_root) and the wrapper (xxxPlugIn)
// we define the real meaning of xxxPlugIn_Interface depending on the mode -Dll or non Dll)
//


namespace Ronin
{
    class EuphoriaManager;
    class ComponentPlugInInterface;

    //this is the pure virtual interface that is seen outside of the current dll
    //this class is empty in non dll mode

    class EuphoriaManagerPlugIn_Root
    {

    public:

#if defined(DLL_Fortune) && defined(DLL_EUPHORIA)

        virtual ~EuphoriaManagerPlugIn_Root(){};
        virtual ComponentPlugInInterface* GetComponentInterface(RenID renID) =0;
        virtual void HandleMessage(const char* msg, Ronin::RenID target) =0;

#endif // defined(DLL_Fortune)

    };



    //this is the wrapper. we have the declaration here only.
    //implementation is a the bottom of the file

    class EuphoriaManagerPlugIn : public EuphoriaManagerPlugIn_Root
    {
    protected:
        friend class EuphoriaManager;
        EuphoriaManager* mParent;
        inline EuphoriaManagerPlugIn() : mParent(NULL){}
        inline void Set(EuphoriaManager* parent){mParent=parent;}

    public:

        inline ComponentPlugInInterface* GetComponentInterface(RenID renID);
        inline void HandleMessage(const char* msg, Ronin::RenID target);
    };

}

#endif   //_EuphoriaManagerPlugInInterface_H_


#if defined  PLUGIN_IMPL_EuphoriaManager && !defined PLUGIN_IMPL_GUARD_EuphoriaManager
#define PLUGIN_IMPL_GUARD_EuphoriaManager

//implementation of the wrapper class
//we need to specify the body of the wrapper implementation


namespace Ronin
{

    
    // GetComponentInterface
    ComponentPlugInInterface* EuphoriaManagerPlugIn::GetComponentInterface(RenID renID)
    {
        return mParent->GetComponentInterface(renID);
    }

    // HandleMessage
    void EuphoriaManagerPlugIn::HandleMessage(const char* msg, Ronin::RenID target)
    {
        mParent->HandleMessage(msg, target);
    }

}



#endif	//PLUGIN_IMPL_GUARD_EuphoriaManager


#ifndef EuphoriaManagerPlugInInterfaceDef

#if !defined(DLL_Fortune) || !defined(DLL_EUPHORIA)

//  Default model : for consoles , 360 and Ps3
//  The interface class is the wrapper , and everything is inlined

#define EuphoriaManagerPlugInInterfaceDef EuphoriaManagerPlugIn


#else   //!defined(DLL_Fortune)

//  Win32-DLL model 
//  The interface class is the pure virtual parent, and we use polymorphism to access the code inside the dll
//  

#define EuphoriaManagerPlugInInterfaceDef EuphoriaManagerPlugIn_Root

#endif  //!defined(DLL_Fortune)

// This allow class predeclaration, that is no possible by using a define direclty
namespace Ronin
{
    class EuphoriaManagerPlugInInterface : public EuphoriaManagerPlugInInterfaceDef
    {
    };
}

#endif //EuphoriaManagerPlugInInterfaceDef

#if !defined(EuphoriaManagerPlugInInterface_H) && (!defined(DLL_Fortune) || !defined(DLL_EUPHORIA))
#define EuphoriaManagerPlugInInterface_H
#include "EuphoriaManager.h"
#endif

#ifndef DLL_PluginSupport
__do_not_build;
#endif  //DLL_PluginSupport

