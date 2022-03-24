#include "EuphoriaComp.h"
#include "BoneCRC.h"
#include "EuphoriaEvent.h"
#include "EuphoriaManager.h"
#include "EuphoriaParams.h"
#include "Performances/BalancePerformance.h"
#include "Performances/BlendPerformance.h"
#include "Performances/EPAPerformance.h"
#include "Performances/ExplosionPerformance.h"
#include "Performances/FallingPerformance.h"
#include "Performances/GunshotPerformance.h"
#include "Performances/HitReactPerformance.h"
#include "Performances/PunchPerformance.h"
#include "Performances/ShovePerformance.h"
#include "Performances/ThrowPerformance.h"
// For BallAndSocketConstraint
#include "RoninPhysics_Base.h"
// Component Interfaces
#include "AnimationComponentPlugInInterface.h"
#include "EdgeSystemManagerPlugInInterface.h"
#include "FortuneGameCompPlugInInterface.h"
#include "PhysicsComponentPlugInInterface.h"
#include "ScriptComponentPlugInInterface.h"
// Schemas
#include "DefEuphoriaPerformance.schema.h"

#define EUPHORIA_MILESTONE_HACK
#ifdef EUPHORIA_MILESTONE_HACK
    // Declare any state variables supporting the MILESTONE HACK
    float HACK_timeSpentTooStill = 0.0f;
    RavenMath::Vec3 HACK_posWhenLastResetTimer = { 0, 0, 0 };
// EUPHORIA_MILESTONE_HACK
#endif

namespace Ronin 
{
    // USING DIRECTIVES
    using namespace euphoria;
    using namespace GameHooks;
    using namespace RavenMath;
    using namespace RoninPhysics;
    using namespace lec;
    using lec::AssetKey;                // Here to avoid ambiguous symbol errors

    /*!
     * This private class provides the infrastructure to map performance parameter types
     * to the methods in the Euphoria component which handle them.
     */
    class RerouteSystem
    {
    private:
        // TYPEDEFS
        typedef void (*RerouteFunction)(EuphoriaComp* euphoriaComp, const EuphoriaParamsBase* const pParams);

        // PRIVATE STRUCTS
        /*!
         * This structure provides a mapping between performance type and the specific method
         * that starts it.
         */
        struct Reroute
        {
            ePerformanceType type;
            RerouteFunction func;
        };

        // STATIC MEMBERS
        static const Reroute mskReroutePtrs[eSize];

        // STATIC METHODS
        /*!
         * Template method that calls the component's overloaded methods based on
         * the parameter type.
         */
        template<class PARAM> static void StartSpecificPerformance(EuphoriaComp* pEuphoriaComp, const EuphoriaParamsBase* const pParams)
        {
            LECASSERT(pEuphoriaComp);
            LECASSERT(pParams);
            const PARAM* const pTypedParams = pParams->Get<PARAM>();
            pEuphoriaComp->StartSpecificPerformance(*pTypedParams);
        }

    public:
        // STATIC MANIPULATORS
        static void CallMappedMethod(EuphoriaComp* pEuphoriaComp, const EuphoriaParamsBase* const pParams)
        {
            LECASSERT(pParams->mType == mskReroutePtrs[pParams->mType].type);
            (mskReroutePtrs[pParams->mType].func)(pEuphoriaComp, pParams);
        }
    };

    /*!
     * Creates the mappings between the performance types and the methods that handle them
     *
     * @note The Reroute table entries MUST be in the same order as the PerformanceType
     *       enumerations in defined EuphoriaParams.h. Failure to order them the same
     *       will result in asserts!
     */
    const RerouteSystem::Reroute RerouteSystem::mskReroutePtrs[eSize] =
    {
        // See above NOTE before adding to this table!
        { eEPA, &RerouteSystem::StartSpecificPerformance<EPAParams> },
        { eExplosion, &RerouteSystem::StartSpecificPerformance<ExplosionParams> },
        { ePunch, &RerouteSystem::StartSpecificPerformance<PunchParams> },
        { eShove, &RerouteSystem::StartSpecificPerformance<ShoveParams> },
        { eThrow, &RerouteSystem::StartSpecificPerformance<ThrowParams> },
        { eHitReact, &RerouteSystem::StartSpecificPerformance<HitReactParams> },
        { eFalling, &RerouteSystem::StartSpecificPerformance<FallingParams> },
        { eGunshot, &RerouteSystem::StartSpecificPerformance<GunshotParams> },
        { eBalance, &RerouteSystem::StartSpecificPerformance<BalanceParams> },
        { eBlend, &RerouteSystem::StartSpecificPerformance<BlendParams> }
    };


    /*!
     * Constructor
     */
    EuphoriaComp::EuphoriaComp(RenID renID):   
    mRenID(renID),
        mAttackerRenID(RENID_NULL),
        mpAnimationComp(NULL),
        mpPhysicsComp(NULL),
        mpScriptComp(NULL),
        mpCurrentPerformance(NULL),
        mkConstraintBreakThreshold(100.0f),
        mGrabDelayTimer(0.0f),
        mkGrabDelayThreshold(0.25),
        mbIsRecoveryEnabled(true),
        mbIsTransitioning(false),
        mbTruncateMovementEnable(true)
    {
        mPlugInInterface.Set(this);

        mpHandConstraint[eLeftArm] = NULL;
        mpHandConstraint[eRightArm] = NULL;

        mbIsGrabbingWith[eLeftArm] = false;
        mbIsGrabbingWith[eRightArm] = false;
    }

    /*!
     * Destructor
     */
    EuphoriaComp::~EuphoriaComp()
    {
    }

    /*!
     * Instantiates one of each type of Euphoria performance for this NPC. Sets
     * custom attributes from data when available.
     */
    void EuphoriaComp::CreatePerformances()
    {
        mpPerfBlend = lec_new BlendPerformance(this);
        mpPerfThrow = lec_new ThrowPerformance(this, mGrabbedEdge);
        mpPerfPunch = lec_new PunchPerformance(this);
        mpPerfEPA = lec_new EPAPerformance(this, mGrabbedEdge);
        mpPerfExplosion = lec_new ExplosionPerformance(this);
        mpPerfShove = lec_new ShovePerformance(this);
        mpPerfFalling = lec_new FallingPerformance(this, mGrabbedEdge);
        mpPerfGunshot = lec_new GunshotPerformance(this);
        mpPerfHitReact = lec_new HitReactPerformance(this);
        mpPerfBalance = lec_new BalancePerformance(this);

        // Data-drive settings for the performances, when available
        // First check for the existence of the Fortune Euphoria attrib group
        FortuneGameRenPlugInInterface* ri = gpEuphoriaManager->GetFortuneGamePlugIn()->GetRenInterface();
        const AssetKey key = ri->RenAttribGetAsset(mRenID, ATTRIB_FORTUNE_EUPHORIA_SETTINGS);
        if (key == ASSETKEY_NONE)
        {
            // No settings attrib group, bail.
            return;
        }

        // Check for settings
        AssetHandle ah = AssetCatalog::LoadAssetKey(key, ASSET_TYPE_FORTUNE_PERFORMANCE_SETTINGS);
        if (ah == ASSETHANDLE_NONE)
        {
            // No settings, bail.
            return;
        }

        // Lock and load settings
        DefEuphoriaPerformanceSettings* pDeps;
        AutoLockDefAsset::LockAsset(ah, &pDeps);

        // Initialize all performances
        mpPerfBlend->Initialize(pDeps->mBlendSettings);
        mpPerfThrow->Initialize(pDeps->mThrowSettings);
        mpPerfPunch->Initialize(pDeps->mPunchSettings);
        mpPerfEPA->Initialize(pDeps->mEPASettings);
        mpPerfExplosion->Initialize(pDeps->mExplosionSettings);
        mpPerfShove->Initialize(pDeps->mShoveSettings);
        mpPerfFalling->Initialize(pDeps->mFallingSettings);
        //mpPerfGunshot->Initialize(pDeps->mGunshotSettings);
        mpPerfHitReact->Initialize(pDeps->mHitReactSettings);
        //mpPerfBalance->Initialize(pDeps->mBalanceSettings);

        AssetCatalog::UnlockAsset(ah);
    }

    /*!
     * Destroys all of the performances created for this NPC.
     */
    void EuphoriaComp::DestroyPerformances()
    {
        lec_delete mpPerfBalance;
        mpPerfBalance = NULL;

        lec_delete mpPerfHitReact;
        mpPerfHitReact = NULL;

        lec_delete mpPerfGunshot;
        mpPerfGunshot = NULL;

        lec_delete mpPerfFalling;
        mpPerfFalling = NULL;

        lec_delete mpPerfShove;
        mpPerfShove = NULL;

        lec_delete mpPerfExplosion;
        mpPerfExplosion = NULL;

        lec_delete mpPerfEPA;
        mpPerfEPA = NULL;

        lec_delete mpPerfPunch;
        mpPerfPunch = NULL;

        lec_delete mpPerfThrow;
        mpPerfThrow = NULL;

        lec_delete mpPerfBlend;
        mpPerfBlend = NULL;
    }

    void EuphoriaComp::Destroy()
    {
        // End behavior control as early as possible to avoid dangling ptr problems
        if (mpCurrentPerformance != NULL)
        {
            StopPerformance();
        }

        DestroyPerformances();
    }

    /*!
     * Acquires and verifies pointers to the following component interfaces:
     * Animation, Physics, Script, Game and Ren (not really a comp)
     */
    void EuphoriaComp::InitCrossComp()
    {
        // Get animation component
        mpAnimationComp = gpEuphoriaManager->GetFortuneGamePlugIn()->GetAnimationCompInterface(mRenID);
        LECASSERT(mpAnimationComp);
        // Get physics component
        mpPhysicsComp = gpEuphoriaManager->GetFortuneGamePlugIn()->GetPhysicsCompInterface(mRenID);
        LECASSERT(mpPhysicsComp);
        // Get script component
        mpScriptComp = gpEuphoriaManager->GetFortuneGamePlugIn()->GetScriptCompInterface(mRenID);
        LECASSERT(mpScriptComp);
        // Get game component
        mpGameComp = gpEuphoriaManager->GetFortuneGamePlugIn()->GetFortuneGameCompInterface(mRenID);
        LECASSERT(mpGameComp);
        // Get the interface to work with rens
        mpRenInterface = gpEuphoriaManager->GetFortuneGamePlugIn()->GetRenInterface();
        LECASSERT(mpRenInterface);
        // Get the interface to work with the rest of the engine
        mpEngineInterface = gpEuphoriaManager->GetFortuneGamePlugIn();
        LECASSERT(mpEngineInterface);

        CreatePerformances();
    }

    void EuphoriaComp::InitPostCrossComp()
    {
        // [8/23/2007 kguran]
        // Moved this here from FortuneCharacterManager::InitPostCrossComp
        // [5/9/2007 kguran]
        // Create a script component so characters can go into Euphoria
        // This is a TEMP solution but it might last a while...
        // Note: the script attaches a listener and so requires a BehaviorSklMgr
        // in the ren's anim comp or else it doesn't work like it should. That is
        // why this has been moved from CreateDependentComps
        mpScriptComp->AttachScript("BotPerfManager");
    }

    /*!
     *
     */
    void EuphoriaComp::Think(float elapsedTime)
    {
        if (mpCurrentPerformance != NULL)
        {
            CheckForBrokenConstraints();

            mpCurrentPerformance->Think(elapsedTime);

            // Increment delay timer
            mGrabDelayTimer += elapsedTime;

#define EUPHORIA_MILESTONE_HACK
#ifdef EUPHORIA_MILESTONE_HACK
            Vec3 currentPos = mpRenInterface->RenGetPosition(mRenID);
            float sqrDistMoved = Vec3DistSqr(HACK_posWhenLastResetTimer, currentPos);
            if (sqrDistMoved < 1.0f)
            {
                // Haven't moved enough
                HACK_timeSpentTooStill += elapsedTime;
                if (HACK_timeSpentTooStill > 13.0f)
                {
                    Vec4 red = { 1, 0, 0, 1 };
                    mpEngineInterface->DEBUGDrawQueueText2DTimed("FORCE QUITTING euphoria (Think)", 10, 30, red, 2);

                    StopPerformance();
                }
            }
            else
            {
                // Moved, reset
                HACK_timeSpentTooStill = 0.0f;
                Vec3Set(HACK_posWhenLastResetTimer, currentPos);
            }
// EUPHORIA_MILESTONE_HACK
#endif
        }

        if (mpAnimationComp->IsBehaviorActiveAndDriving())
        {
            //! @note The block below is outside the mpCurrentPerformance != NULL block
            //        because the ability to run the Lua blend requires it (note that the
            //        Lua version sets mpCurrentPerformance to NULL). This entire block can
            //        be moved inside the above block but it must come *before* the Think
            //        call because it's possible that Think will EndBehaviorControl which will
            //        cause things to fail when RPEs are NULL as a result. This code should
            //        be relocated once Lua is gone for good.
            if (!mbIsTransitioning && mbIsRecoveryEnabled)
            {
                CheckBodyForMovement(elapsedTime);
                if (IsBodyStationary())
                {
                    mbIsTransitioning = true;
#define EUPHORIA_USE_CPP_BLEND_PERFORMANCE
#ifndef EUPHORIA_USE_CPP_BLEND_PERFORMANCE
                    // Get recover position, and use the Lua version of the blend, for now
                    GetupPosition position = DetermineGetupPosition();
                    LECUNUSED(position);
                    char params[64];
                    sprintf(params, "name=EuphoriaStationary,position=%d", static_cast<int>(position));
                    gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
                    mpCurrentPerformance = NULL;
#else
                    BlendParams rParams;
                    rParams.mAttacker = mAttackerRenID;
                    rParams.mInitialPosition = DetermineGetupPosition();
                    StartPerformance(rParams);
#endif
                }
            }
        }
    }

    void EuphoriaComp::DebugDraw() const
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EuphoriaComp::DebugDraw\n");
    }

    /*!
     * Starts the Blend Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const BlendParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Blending back to animation!\n");

        mpPerfBlend->SetParams(rParams);
        mpCurrentPerformance = mpPerfBlend;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
    }

    /*!
     * Starts the EPA Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const EPAParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "EPA'd!\n");

        mpPerfEPA->SetParams(rParams);
        mpCurrentPerformance = mpPerfEPA;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
    }

    /*!
     * Starts the Explosion Performance. This is an overloaded method.
     *
     * @todo Unify the messages (there should be only one going into the BotPerfMgr)
     */
    void EuphoriaComp::StartSpecificPerformance(const ExplosionParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Explodified!\n");

#define EUPHORIA_USE_CPP_EXPLOSION_PERFORMANCE
#ifndef EUPHORIA_USE_CPP_EXPLOSION_PERFORMANCE
        // Send in the source ren, position, the normal to the character, and the mass and velocity (speed)
        char params[128];
        sprintf(params, "sr=%d,sx=%f,sy=%f,sz=%f,nx=%f,ny=%f,nz=%f,m=%f,v=%f",
            rParams.mAttacker,
            rParams.mSourcePos.x, rParams.mSourcePos.y, rParams.mSourcePos.z,
            rParams.mNormal.x, rParams.mNormal.y, rParams.mNormal.z,
            rParams.mSourceMass, rParams.mVelocity);

        // Unify these two messages
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg("EuphoriaExplosion", RENID_NULL, mRenID, 0.0f);
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
#else
        mpPerfExplosion->SetParams(rParams);
        mpCurrentPerformance = mpPerfExplosion;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
#endif
    }

    /*!
     * Starts the Punch Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const PunchParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Punched!\n");

#define EUPHORIA_USE_CPP_PUNCH_PERFORMANCE
#ifndef EUPHORIA_USE_CPP_PUNCH_PERFORMANCE
        // Send in the attacker's ren id, the force vector, the magnitude of the force, 
        // and the (CRC of the) bone that was hit.
        char params[128];
        sprintf(params, "sr=%d,fx=%f,fy=%f,fz=%f,fm=%f,ib=%d",
            rParams.mAttacker,
            rParams.mForceNormal[0],
            rParams.mForceNormal[1],
            rParams.mForceNormal[2],
            rParams.mForceMagnitude,
            rParams.mImpactBone);

        // Unify these messages
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg("EuphoriaPunched", RENID_NULL, mRenID, 0.0f);
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
#else
        mpPerfPunch->SetParams(rParams);
        mpCurrentPerformance = mpPerfPunch;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
#endif
    }

    /*!
     * Starts the Shove Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const ShoveParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Shoved!\n");

#define EUPHORIA_USE_CPP_SHOVE_PERFORMANCE
#ifndef EUPHORIA_USE_CPP_SHOVE_PERFORMANCE
        // Send in the attacker's ren id, the force vector, the magnitude of the force, 
        // and the (CRC of the) bone that was hit.
        char params[128];
        sprintf(params, "sr=%d,fx=%f,fy=%f,fz=%f,fm=%f,ib=%d",
            rParams.mAttacker,
            rParams.mForceNormal[0],
            rParams.mForceNormal[1],
            rParams.mForceNormal[2],
            rParams.mForceMagnitude,
            rParams.mImpactBone);

        // Unify these messages
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg("EuphoriaShoved", RENID_NULL, mRenID, 0.0f);
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
#else
        mpPerfShove->SetParams(rParams);
        mpCurrentPerformance = mpPerfShove;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
#endif
    }

    /*!
     * Starts the Throw Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const ThrowParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Thrown!\n");

#define EUPHORIA_USE_CPP_THROW_PERFORMANCE
#if !defined(EUPHORIA_USE_CPP_THROW_PERFORMANCE)
        // Build the throw message and send it
        char params[128];
        sprintf(params, "sr=%d", rParams.mAttacker);

        // Unify these messages
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg("EuphoriaThrown", RENID_NULL, mRenID, 0.0f);
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
#else
        mpPerfThrow->SetParams(rParams);
        mpCurrentPerformance = mpPerfThrow;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
#endif
    }

    /*!
     * Starts the HitReact Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const HitReactParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "HitReacting!\n");

#define EUPHORIA_USE_CPP_HITREACT_PERFORMANCE
#if !defined(EUPHORIA_USE_CPP_HITREACT_PERFORMANCE)
        // Build the hit react message and send it
        char params[128];
        sprintf(params, "sr=%d", rParams.mAttacker);

        // Unify these messages
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg("EuphoriaHitReact", RENID_NULL, mRenID, 0.0f);
        gpEuphoriaManager->GetFortuneGamePlugIn()->RoninLua_SendMsg(params, RENID_NULL, mRenID, 0.0f);
#else
        mpPerfHitReact->SetParams(rParams);
        mpCurrentPerformance = mpPerfHitReact;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
#endif

    }

    /*!
     * Starts the Falling Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const FallingParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Falling!\n");

        mpPerfFalling->SetParams(rParams);
        mpCurrentPerformance = mpPerfFalling;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
    }

    /*!
     * Starts the Gunshot Reaction Performance. This is an overloaded method.
     */
    void EuphoriaComp::StartSpecificPerformance(const GunshotParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Shot!\n");

        mpPerfGunshot->SetParams(rParams);
        mpCurrentPerformance = mpPerfGunshot;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
    }

    void EuphoriaComp::StartSpecificPerformance(const BalanceParams& rParams)
    {
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Balancing!\n");

        mpPerfBalance->SetParams(rParams);
        mpCurrentPerformance = mpPerfBalance;
        // @note NO NEED FOR START TO BE VIRTUAL IF CALLED HERE...
        mpCurrentPerformance->Start();
    }

    /*!
     * EXPOSED METHOD
     *
     * Starts a new performance based on the parameter type. Starting all performances
     * through this method ensures that the state machines cleanup properly.
     *
     * @note Starting the Blend is a special case
     * @note Make sure that mAttackerRenID and other members are properly reset
     */
    void EuphoriaComp::StartPerformance(const EuphoriaParamsBase& rParams)
    {
#define EUPHORIA_MILESTONE_HACK
#ifdef EUPHORIA_MILESTONE_HACK
        // Reset any state variable supporting the MILESTONE HACK
        HACK_timeSpentTooStill = 0.0f;
// EUPHORIA_MILESTONE_HACK
#endif

        // Stop any currently executing performance
        if (mpCurrentPerformance != NULL)
        {
            mpCurrentPerformance->Stop();
            mpCurrentPerformance = NULL;
        }

        // Reset the transitioning flag to enable stationary checking, ONLY for non-Blend performances
        if (rParams.mType != eBlend)
        {
            mbIsTransitioning = false;
            ResetStationaryTimer();
        }

        // Reset grab timer for performances that can grab
        mGrabDelayTimer = 0.0f;

        // Make sure the required resources are available. If euphoria is already
        // running then no agent is needed (since we already have one.)
        if (IsEuphoriaActive() || mpAnimationComp->IsAgentAvailable())
        {
            // Set attacker
            mAttackerRenID = rParams.mAttacker;

            // Start the requested performance
            RerouteSystem::CallMappedMethod(this, &rParams);
        }
        else
        {
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "No Agents Available! Performance ABORTED!\n");
        }
    }

    /*!
     * EXPOSED METHOD
     *
     * Forces euphoria to stop running and readies the component for new performances
     */
    void EuphoriaComp::StopPerformance()
    {
        if (mpCurrentPerformance != NULL)
        {
            // Allow any currently running performance to cleanup
            mpCurrentPerformance->Stop();
            mpCurrentPerformance = NULL;

            // Force behaviors to stop running
            mpAnimationComp->EndBehaviorControl();

            // Clean up any grabbing
            if (mbIsGrabbingWith[eLeftArm])
            {
                ReleaseConstraint(eLeftArm);
            }
            if (mbIsGrabbingWith[eRightArm])
            {
                ReleaseConstraint(eRightArm);
            }

            // Make sure transitioning is reset
            mbIsTransitioning = false;
        }
    }

    /*!
     * Sets the current performance to NULL so that the Blend no longer
     * runs in IDLE during the think after it is finished
     *
     * @note This method exists to allow the component to return to normal after blending
     */
    void EuphoriaComp::DisconnectPerformance()
    {
        if (mpCurrentPerformance != NULL)
        {
            mpCurrentPerformance = NULL;

            // Make sure transitioning is reset
            mbIsTransitioning = false;
        }
    }

    /*!
     * EXPOSED METHOD
     *
     * Determines if we should get up from front, back, left, right or upright
     *
     * @todo Get rid of the local static constants
     * @note Is the "Not in Euphoria" block necessary?
     * @todo Can this be moved to the BlendPerformance?
     */
    GetupPosition EuphoriaComp::DetermineGetupPosition() const
    {
        // Constants
        static const float kf45Dot = Cosine(DegToRad(45.0f));
        static const float kf55Dot = Cosine(DegToRad(55.0f));
        static const float kf90Dot = Cosine(DegToRad(90.0f));
        static const float kf125Dot = Cosine(DegToRad(125.0f));
        static const float kfMinUprightDot = kf45Dot;
        static const Vec3 kUpIdentityVec = { 0.0f, 1.0f, 0.0f };
        static const Vec3 kForwardIdentityVec = { 0.0f, 0.0f, 1.0f };

        // The following will return false if the agent is not in the world
        RoninPhysicsEntity* pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eSpine0);
        Mat44 spineTransform;
        if (!pRPE->GetTransform(spineTransform))
        {
            // Not in Euphoria, don't need to getup
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
            return eGetupNone;
        }

        // Check for death
        if (mpGameComp->IsDead())
        {
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: DEAD\n");
            return eGetupDead;
        }

        // Agent is in the world and hence, Euphoria, so continue...
        Vec3 rootPos;
        Vec3Set(rootPos, ROW_TRANSLATION(spineTransform));

        Vec3 rootForwardVec;
        Mat33MulVec3(rootForwardVec, spineTransform, kForwardIdentityVec);

        Vec3 rootUpVec;
        Mat33MulVec3(rootUpVec, spineTransform, kUpIdentityVec);					

        // First, determine if we're upright (more or less vertical)...
        float fDot = Vec3Dot(rootUpVec, kUpIdentityVec);
        if (fDot >= kfMinUprightDot)
        {
            // Vertical
            // Determine whether the agent is in the air
            float32 distanceToCheck = 1.2f;
            bool bSolidBeneath = gpEuphoriaManager->GetFortuneGamePlugIn()->IsSupportedEuphoria(BoneCRC::eSpine0, distanceToCheck, mpPhysicsComp, mRenID);
            if (!bSolidBeneath)
            {
                // We're floating in the air, how can we get up?
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
                return eGetupNone;
            }

            // First, let's check to make sure the feet are an adequate distance below the root
            RoninPhysicsEntity* pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eRightFoot0);
            Vec3 rightFootPos;
            pRPE->GetPosition(rightFootPos);

            pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eLeftFoot0);
            Vec3 leftFootPos;
            pRPE->GetPosition(leftFootPos);

            const float32 kfUprightFootDiff = 0.5f;
            float32 fYDiff = rootPos[1] - rightFootPos[1];
            if (fYDiff < kfUprightFootDiff)
            {
                // Our root is upright, but our right foot is above our hips, which we can't handle yet
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
                return eGetupNone;
            }

            fYDiff = rootPos[1] - leftFootPos[1];
            if (fYDiff < kfUprightFootDiff)
            {           
                // Our root is upright, but our left foot is above our hips, which we can't handle yet
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
                return eGetupNone;
            }

            // Now, let's check to see if the head is doing something sensible
            //             RoninPhysicsEntity* pHeadRPE = mpAnimationComp->GetBodyPartForBone(kHead);
            //             LECASSERT(pHeadRPE);
            pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eNeck2);
            Vec3 headPos;
            pRPE->GetPosition(headPos);
            //             pHeadRPE->GetPosition(headPos);

            // Project the head and root positions onto the XZ plane
            headPos[1] = 0.0f;
            rootPos[1] = 0.0f;

            // Determine the XZ distance from the root to the head
            Vec3 rootToHeadXZ;
            Vec3Sub(rootToHeadXZ, headPos, rootPos);
            float fSqDiff = Vec3MagSqr(rootToHeadXZ);

            // Determine which way the head is leaning...we'll use a 45 degree arc to determine if it's "forward" or not
            Vec3 rootForwardXZ = rootForwardVec;
            rootForwardXZ[1] = 0.0f;
            Vec3Normalize(rootToHeadXZ, rootToHeadXZ);
            Vec3Normalize(rootForwardXZ, rootForwardXZ);
            float fDot = Vec3Dot(rootToHeadXZ, rootForwardXZ);
            float fMaxXZDistSq = (0.25f * 0.25f);
            if (fDot > kf45Dot)
            {
                // The head is leaning forward, so give it some additional room
                fMaxXZDistSq = (0.4f * 0.4f);
            }

            // Is the spine is bent too far over to recover? We can't handle that yet
            if (fSqDiff > fMaxXZDistSq)
            {
                // TODO: make this a better spine check
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
                return eGetupNone;
            }
            // We're upright
            return eGetupUpright;
        }
        else
        {
            // Horizontal
            // Determine whether the agent is in the air
            float32 distanceToCheck = 0.5f;
            bool bSolidBeneath = gpEuphoriaManager->GetFortuneGamePlugIn()->IsSupportedEuphoria(BoneCRC::eSpine0, distanceToCheck, mpPhysicsComp, mRenID);
            if (!bSolidBeneath)
            {
                // We're floating in the air, how can we get up?
                LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
                return eGetupNone;
            }

            // Test the root forward vector to determine if we're laying on our front, side or back
            fDot = Vec3Dot(kUpIdentityVec, rootForwardVec);
            if (fDot <= kf125Dot)
            {
                // We're on our stomach
                return eGetupFront;
            }
            else if (fDot <= kf55Dot)
            {
                // On the side...now determine which side
                Vec3 tmp;
                Vec3Cross(tmp, rootUpVec, rootForwardVec);
                float fSideDot = Vec3Dot(tmp, kUpIdentityVec);
                if (fSideDot >= kf90Dot)
                {
                    return eGetupRight;
                }
                else
                {
                    return eGetupLeft;
                }
            }
            else
            {
                // On our back
                return eGetupBack;
            }
        }
        LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Getup: None\n");
        return eGetupNone;
    }

    /*!
     * Checks for and releases broken constraints.
     *
     * @note This method must be updated if NPCs can grab w/ more than just the two hands
     */
    void EuphoriaComp::CheckForBrokenConstraints()
    {
        // Left
        if (mpHandConstraint[eLeftArm] && mpHandConstraint[eLeftArm]->HasBroke())
        {
            ReleaseConstraint(eLeftArm);
        }

        // Right
        if (mpHandConstraint[eRightArm] && mpHandConstraint[eRightArm]->HasBroke())
        {
            ReleaseConstraint(eRightArm);
        }
    }

    /*!
     * Constrains a limb to a point in world space or local to an RPE. Resets the
     * grab timer to prevent the character from repeatedly grabbing and releasing.
     *
     * @note Always succeeds
     * @note NPC grabbing is not supported
     *
     * @todo Get rid of _UA ops
     */
    void EuphoriaComp::ConstrainLimb(ReachingArm reachingArm, RoninPhysicsEntity* pEntityToGrab, const Vec3& vWorldReachForPos)
    {
        LECASSERT(!mpHandConstraint[reachingArm]);

        if (IsHandConstrained(reachingArm))
        {
            return;
        }

        // Handle ReachingArm-specific stuff
        Vec3 vHandOffset;
        RoninPhysicsEntity* pHand = NULL;
        switch (reachingArm)
        {
        case eLeftArm:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Constraining Left Hand! (EuphoriaComp::ConstrainLimb)\n");
            Vec3Set(vHandOffset, 0.0f, -0.05f, -0.05f);
            pHand = mpAnimationComp->GetBodyPartByBone(BoneCRC::eLeftMiddle0);     // BoneCRC::eLeftHand0
            mpAnimationComp->StartOverlay(OverlayParameters::eLeftHand, CRCINIT("LEFT_HAND_HANG"), CRC32_NULL);
            break;

        case eRightArm:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Constraining RIGHT Hand! (EuphoriaComp::ConstrainLimb)\n");
            Vec3Set(vHandOffset, 0.0f, 0.05f, 0.05f);
            pHand = mpAnimationComp->GetBodyPartByBone(BoneCRC::eRightMiddle0);    // BoneCRC::eRightHand0
            mpAnimationComp->StartOverlay(OverlayParameters::eRightHand, CRCINIT("RIGHT_HAND_HANG"), CRC32_NULL);
            break;

        default:
            // Should never get here, something really bad happened
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Invalid ReachingArm in FallingPerformance::ConstrainLimb!\n");
            LECASSERT(0);
            return;
        }

        // Determine the final value to be used for the 'reach for' position. When
        // the entity to grab exists, this world-space position will be converted
        // to a local-space position relative to the entity. A NULL entity means
        // the original world-space value is used as the grab position.
        Vec3 vFinalEntityOffset(vWorldReachForPos);
        if (pEntityToGrab != NULL)
        {
            // Calculate entity offset
            Mat44 entityXform;
            pEntityToGrab->GetTransform(entityXform);
            Mat44InvertOrtho(entityXform, entityXform);
            Mat44MulPoint3(vFinalEntityOffset, entityXform, vWorldReachForPos);
        }

        BallAndSocketConstraintInfo bsInfo;
        bsInfo.mpEntity1 = pHand;
        bsInfo.mpEntity2 = pEntityToGrab;
        Vec3Set_UA(bsInfo.mEntity1Offset, vHandOffset);
        Vec3Set_UA(bsInfo.mEntity2Offset, vFinalEntityOffset);
        bsInfo.mfBreakThreshold = mkConstraintBreakThreshold;
        mpHandConstraint[reachingArm] = mpEngineInterface->CreateBallAndSocketConstraint(bsInfo);
        mbIsGrabbingWith[reachingArm] = true;

        // Reset the grab delay timer to prevent dithering (grab-release-grab-release-etc)
        mGrabDelayTimer = 0.0f;
    }

    /*!
     * Hand-agnostic constraint releaser.
     */
    void EuphoriaComp::ReleaseConstraint(ReachingArm reachingArm)
    {
        if (mpHandConstraint[reachingArm])
        {
            lec_delete mpHandConstraint[reachingArm];
            mpHandConstraint[reachingArm] = NULL;

            mbIsGrabbingWith[reachingArm] = false;

            // Reset grab delay timer
            mGrabDelayTimer = 0.0f;
        }

        switch (reachingArm)
        {
        case eLeftArm:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Releasing Left Hand! (EuphoriaComp::ReleaseConstraint)\n");
            break;

        case eRightArm:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Releasing Right Hand! (EuphoriaComp::ReleaseConstraint)\n");
            break;

        default:
            // Should never get here
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Unknown Hand Releasing! (EuphoriaComp::ReleaseConstraint)\n");
            LECASSERT(0);
            break;
        }
    }

    /*!
     * Takes care of constraint management on behalf of the euphoria behaviors
     */
    void EuphoriaComp::HandleEuphoriaConstraintMessages(EuphoriaEvent::Type eventType)
    {
        Vec3 closestPoint;
        Vec3SetZero(closestPoint);
        RoninPhysicsEntity* pHand = NULL;
        Vec3 handPosition;
        Vec3SetZero(handPosition);
        float distanceToEdge = 0.0f;

        EdgeSystemManagerPlugInInterface* pEdgeMgr = mpEngineInterface->GetEdgeSystemInterface();

        switch (eventType)
        {
        case EuphoriaEvent::eBehaviorConstrainLeftHand:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Event: eBehaviorConstrainLeftHand\n");

            pHand = mpAnimationComp->GetBodyPartByBone(BoneCRC::eLeftHand0);
            pHand->GetPosition(handPosition);
            distanceToEdge = pEdgeMgr->DistanceToEdge(mGrabbedEdge.mHandle, handPosition, closestPoint);
            ConstrainLimb(eLeftArm, mGrabbedEdge.mpOwnerRPE, closestPoint);
            break;

        case EuphoriaEvent::eBehaviorConstrainRightHand:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Event: eBehaviorConstrainRightHand\n");

            pHand = mpAnimationComp->GetBodyPartByBone(BoneCRC::eRightHand0);
            pHand->GetPosition(handPosition);
            distanceToEdge = pEdgeMgr->DistanceToEdge(mGrabbedEdge.mHandle, handPosition, closestPoint);
            ConstrainLimb(eRightArm, mGrabbedEdge.mpOwnerRPE, closestPoint);
            break;

        case EuphoriaEvent::eBehaviorUnConstrainLeftHand:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Event: eBehaviorUnConstrainLeftHand\n");

            ReleaseConstraint(eLeftArm);
            break;

        case EuphoriaEvent::eBehaviorUnConstrainRightHand:
            LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Event: eBehaviorUnConstrainRightHand\n");

            ReleaseConstraint(eRightArm);
            break;

        default:
            //LECTRACECH(LecTraceChannel::CHANNEL_EUPHORIA, "Unhandled Euphoria Event!\n");
            break;
        }
    }

    /*!
     * If the body is stationary (within a tolerance specified by mfStationaryMinSpeed)
     * then the elapsed time is added to the total time the body has been stationary.
     * If the body is NOT stationary, the time spent stationary is reset to zero.
     *
     * @note This method (as opposed to the Scum version) assumes that the bones
     *       have RPEs because the character is in Euphoria (or else we would not
     *       be calling this.) Also, we insist that these bones exist in the rig
     *       being tested.
     */
    void EuphoriaComp::CheckBodyForMovement(float elapsedTime)
    {
        // Use a sampling of bones to test whether the body is stationary
        // Sum the velocities of the sample bones...
        Vec3 averageVelocity = { 0.0f, 0.0f, 0.0f };
        Vec3 vVel;
        // Unrolled loop...
        // Note: If the number of bones in this check is changed, the number must
        //       be updated below as well
        RoninPhysicsEntity* pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eSpine0);
        pRPE->GetLinearVelocity(vVel);
        Vec3Add(averageVelocity, averageVelocity, vVel);
        pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eRightHand0);
        pRPE->GetLinearVelocity(vVel);
        Vec3Add(averageVelocity, averageVelocity, vVel);
        pRPE = mpAnimationComp->GetBodyPartByBone(BoneCRC::eLeftFoot0);
        pRPE->GetLinearVelocity(vVel);
        Vec3Add(averageVelocity, averageVelocity, vVel);
        // ...and find the average velocity
        // Note: the constant used in the next line is 1 over the number of bones
        //       used for the calculation above
        Vec3Mul(averageVelocity, averageVelocity, 1.0f / 3.0f);

        // Subtract out any movement from the character's supporting platform
        Vec3 supportingVelocity;
        Mat44 rotationChange;
        mpAnimationComp->DetermineSupportingVelocity(supportingVelocity, rotationChange);
        Vec3Sub(averageVelocity, averageVelocity, supportingVelocity);

        // Convert velocity to speed
        float32 fSpeed = Vec3Mag(averageVelocity);

        // If the speed is slow enough count the elapsed time, otherwise reset the timer
        if (fSpeed < mfStationaryMinSpeed)
        {
            mfStationaryElapsedTime += elapsedTime;
        }
        else
        {
            mfStationaryElapsedTime = 0.0f;
        }
    }

    /*!
     * Passes animation events to the currently running performance
     */
    void EuphoriaComp::HandleAnimationEvent(crc32_t eventNameCRC)
    {
        if (mpCurrentPerformance != NULL)
        {
            EuphoriaAnimationEvent rEvent = CreateEuphoriaAnimationEvent(eventNameCRC);
            mpCurrentPerformance->OnEvent(rEvent);
        }
    }

    /*!
     * Passes a behavior event to the currently running performance
     */
    void EuphoriaComp::HandleBehaviorEvent(BehaviorFeedback::BehaviorEventType eventType, ParameterOverrideDataInterface* pData, const RoninPhysics::CollisionData* pCollisionData)
    {
        if (mpCurrentPerformance)
        {
            EuphoriaBehaviorEvent rEvent = CreateEuphoriaBehaviorEvent(eventType, pData, pCollisionData);
            mpCurrentPerformance->OnEvent(rEvent);
        }
    }

    /*!
     * Ensure that the character is facing in the appropriate direction.
     * This method is only used for getups from the LEFT or RIGHT side. All other
     * getups have transforms that don't need to be modified.
     *
     * @todo Consider moving this into to the BLEND performance
     * @todo Rename this, change its usage, and get rid of the early out test
     */
    void EuphoriaComp::OrientRen(GetupPosition position)
    {
        static const crc32_t kBoneRoot = CRCINIT("Spine[0]");

        // For back, front and upright, the transforms look OK, so bail
        if (eGetupBack == position || eGetupFront == position || eGetupUpright == position)
        {
            return;
        }

        // Get the root transform
        RoninPhysicsEntity* pRPE = mpAnimationComp->GetBodyPartByBone(kBoneRoot);
        Mat44 rootTransform;
        pRPE->GetTransform(rootTransform);

        // Use the forward vector to figure the angle to the new orientation
        Vec3 forwardVec = ROW_Y_BASIS(rootTransform);
        // Project the vector onto the XZ plane
        forwardVec[1] = 0.0f;
        LECASSERT(Vec3MagSqr_UA(forwardVec) > rm_epsilonFloat);
        Vec3Normalize(forwardVec, forwardVec);

        // Determine the angle to the forward vector
        const Vec3 forwardIdentityVec = { 0.0f, 0.0f, 1.0f };
        float cosAngle = Vec3Dot(forwardVec, forwardIdentityVec);
        float angleRadians = ArcCos(cosAngle);
        // Flip the angle if the forward vec on the other side of Z
        if (forwardVec[0] < 0)
        {
            angleRadians = -angleRadians;
        }

        // Adjust for root trackers being perp to the body when getting up from a side
        if (eGetupRight == position)
        {
            angleRadians += rm_piFloat * 0.5f;
        }
        else if (eGetupLeft == position)
        {
            angleRadians -= rm_piFloat * 0.5f;
        }

        // Create the Y-only rotation; Setting to Identity avoids NaNs in the matrix
        Mat44 newTransform;
        Mat44SetIdentity(newTransform);
        Mat33SetRotation(newTransform, angleRadians, kVecUnitY);

        // Copy in the position
        const float* pCurrentPosition = mpRenInterface->RenGetPosition(mRenID);
        Vec3Set_UA(ROW_TRANSLATION(newTransform), pCurrentPosition);

        VALIDATE_MATRIX(newTransform);
        mpRenInterface->RenSetMatrix(mRenID, newTransform);
    }
} // namespace Ronin
