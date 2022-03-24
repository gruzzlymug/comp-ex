/*********************************************************\
EuphoriaCompPlugInInterface.h
\*********************************************************/

#ifndef _EuphoriaCompPlugInInterface_H_
#define _EuphoriaCompPlugInInterface_H_

#define DLL_EUPHORIA

//
// We define the 2 classes that helps us to build the plugIn Interface model : the virtual interface (xxxPlugin_root) and the wrapper (xxxPlugIn)
// we define the real meaning of xxxPlugIn_Interface depending on the mode -Dll or non Dll)
//
#include "RoninConfig.h"
#include "EuphoriaStruct.h"
#include "BehaviorSklMgr.h"
#include "ParameterOverrideDataInterface.h"

namespace euphoria
{
    // FORWARD DECLARATIONS
    class EuphoriaParamsBase;
    class EuphoriaPerformance;
}

namespace Ronin
{
    class EuphoriaComp;

    /*!
     * This is the pure virtual interface that is seen outside of the current DLL.
     * Note that this class is empty in non-DLL mode.
     */
    class EuphoriaCompPlugIn_Root
    {
    public:

#if defined(DLL_Fortune) && defined(DLL_EUPHORIA)

        virtual ~EuphoriaCompPlugIn_Root(){};
        virtual bool IsEuphoriaActive() const = 0;
        virtual void StartPerformance(const euphoria::EuphoriaParamsBase& rParams) = 0;
        virtual void StopPerformance() = 0;
        virtual void SetStationaryTimer(float speedThreshold, float waitTime) = 0;
        virtual euphoria::GetupPosition DetermineGetupPosition() = 0;
        virtual void SetTruncateMovementEnable(bool bEnable) = 0;
        virtual bool GetTruncateMovementEnable() const = 0;
        virtual void HandleBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData) = 0;
		virtual void HandleAnimationEvent(crc32_t eventNameCRC) = 0;
		virtual void OrientRen(euphoria::GetupPosition position) = 0;

// defined(DLL_Fortune) && defined(DLL_EUPHORIA)
#endif

    };

    /*!
     * This is the wrapper. We have the declaration here only, the implementation
     * is at the bottom of the file.
     */
    class EuphoriaCompPlugIn : public EuphoriaCompPlugIn_Root
    {
    protected:
        // FRIEND DECLARATIONS
        friend class EuphoriaComp;

        // MEMBERS
        EuphoriaComp* mpParent;

        // METHODS
        inline EuphoriaCompPlugIn() : mpParent(NULL){}
        inline void Set(EuphoriaComp* parent) { mpParent = parent; }

    public:
        inline bool IsEuphoriaActive() const;
        inline void StartPerformance(const euphoria::EuphoriaParamsBase& rParams);
        inline void StopPerformance();
        inline void SetStationaryTimer(float speedThreshold, float waitTime);
        inline euphoria::GetupPosition DetermineGetupPosition();
        inline void SetTruncateMovementEnable(bool bEnable);
        inline bool GetTruncateMovementEnable() const;
        inline void HandleBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData);
		inline void HandleAnimationEvent(crc32_t eventNameCRC);
		inline void OrientRen(euphoria::GetupPosition position);
    };
}

//_EuphoriaCompPlugInInterface_H_
#endif


#if defined PLUGIN_IMPL_EuphoriaComp && !defined PLUGIN_IMPL_GUARD_EuphoriaComp
// Make sure there is only one implementation with the following #define
#define PLUGIN_IMPL_GUARD_EuphoriaComp

//implementation of the wrapper class
//we need to specify the body of the wrapper implementation

namespace Ronin
{
    bool EuphoriaCompPlugIn::IsEuphoriaActive() const
    {
        return mpParent->IsEuphoriaActive();
    }

    void EuphoriaCompPlugIn::StartPerformance(const euphoria::EuphoriaParamsBase& rParams)
    {
        mpParent->StartPerformance(rParams);
    }

    void EuphoriaCompPlugIn::StopPerformance()
    {
        mpParent->StopPerformance();
    }

    void EuphoriaCompPlugIn::SetStationaryTimer(float speedThreshold, float waitTime)
    {
        mpParent->SetStationaryTimer(speedThreshold, waitTime);
    }

    euphoria::GetupPosition EuphoriaCompPlugIn::DetermineGetupPosition()
    {
        return mpParent->DetermineGetupPosition();
    }

    void EuphoriaCompPlugIn::SetTruncateMovementEnable(bool bEnable)
    {
        mpParent->SetTruncateMovementEnable(bEnable);
    }

    bool EuphoriaCompPlugIn::GetTruncateMovementEnable() const
    {
        return mpParent->GetTruncateMovementEnable();
    }

    void EuphoriaCompPlugIn::HandleBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData)
    {
        mpParent->HandleBehaviorEvent(eventType, pData, pCollisionData);
    }

	void EuphoriaCompPlugIn::HandleAnimationEvent(crc32_t eventNameCRC)
	{
		return mpParent->HandleAnimationEvent(eventNameCRC);
    }

	void EuphoriaCompPlugIn::OrientRen(euphoria::GetupPosition position)
	{
		return mpParent->OrientRen(position);
	}
}

// PLUGIN_IMPL_GUARD_EuphoriaComp
#endif

#ifndef EuphoriaCompPlugInInterfaceDef

#if !defined(DLL_Fortune) || !defined(DLL_EUPHORIA)

//  Default model : for consoles , 360 and Ps3
//  The interface class is the wrapper , and everything is inlined

#define EuphoriaCompPlugInInterfaceDef EuphoriaCompPlugIn

#else   //!defined(DLL_Fortune)

//  Win32-DLL model 
//  The interface class is the pure virtual parent, and we use polymorphism to access the code inside the dll
//  

#define EuphoriaCompPlugInInterfaceDef EuphoriaCompPlugIn_Root

#endif  //!defined(DLL_Fortune)

// This allow class predeclaration, that is no possible by using a define directly
namespace Ronin
{
    class EuphoriaCompPlugInInterface : public EuphoriaCompPlugInInterfaceDef
    {
    };
}

#endif //EuphoriaCompPlugInInterfaceDef

#if !defined(EuphoriaCompPlugInInterface_H) && (!defined(DLL_Fortune) || !defined(DLL_EUPHORIA))
#define EuphoriaCompPlugInInterface_H
#include "EuphoriaComp.h"
#endif

#ifndef DLL_PluginSupport
__do_not_build;
#endif  //DLL_PluginSupport
