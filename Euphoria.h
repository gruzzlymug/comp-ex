/*********************************************************\
Euphoria.h
\*********************************************************/

#ifndef _Euphoria_H
#define _Euphoria_H
#pragma once

#include "EuphoriaConstants.h"

#include "EuphoriaManagerPlugInInterface.h"
#include "EuphoriaCompPlugInInterface.h"

#ifndef DLL_Fortune

#include "EuphoriaManager.h"

namespace Ronin
{
    inline EuphoriaManagerPlugInInterface* GetEuphoriaMgrInterface() 
    {
        return reinterpret_cast<EuphoriaManagerPlugInInterface*>(gpEuphoriaManager->GetInterface()); 
    }

    inline ManagerInterface* GetEuphoriaMgr() 
    {
        return gpEuphoriaManager; 
    }

    inline EuphoriaCompPlugInInterface* GetEuphoriaCompPlugInInterface(RenID rendId)
    {
        return reinterpret_cast<EuphoriaCompPlugInInterface*>(gpEuphoriaManager->GetComponentInterface(rendId));
    }
}

// DLL_Fortune
#else

#include "ManagersRegistry.h"

namespace Ronin
{
    inline ManagerInterface* GetEuphoriaMgr()
    {
        return GetManagersRegistry()->GetManager(GetEuphoriaPlugInName());
    }

    inline EuphoriaManagerPlugInInterface* GetEuphoriaMgrInterface()
    {
        ManagerInterface* manager = GetEuphoriaMgr();
        if (!manager)
            return NULL;
        return reinterpret_cast<EuphoriaManagerPlugInInterface*>(manager->GetInterface());
    }

    inline EuphoriaCompPlugInInterface* GetEuphoriaCompPlugInInterface(RenID rendId)
    {
        return reinterpret_cast<EuphoriaCompPlugInInterface*>(GetEuphoriaMgr()->GetComponentInterface(rendId));
    }
}

// DLL_Fortune
#endif

// _Euphoria_H
#endif
