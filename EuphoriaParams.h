#ifndef _EuphoriaParams_H_
#define _EuphoriaParams_H_
#pragma once

#include "RavenMath.h"
#include "Rens/RenTypes.h"
#include "EuphoriaStruct.h"

namespace euphoria
{
    // ENUMERATIONS
    enum ePerformanceType
    {
        eInvalid = -1,
        eEPA,
        eExplosion,
        ePunch,
        eShove,
        eThrow,
        eHitReact,
        eFalling,
        eGunshot,
        eBalance,
        eBlend,
        eSize
    };

    /*!
     * Base class for Euphoria parameter structures
     *
     * @todo Move appropriate copy constructor stuff up to the base class
     */
    DECLARE_ALIGNED class EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // MEMBERS
        const ePerformanceType mType;
        Ronin::RenID mAttacker;

        // METHODS
        inline EuphoriaParamsBase(ePerformanceType type) : mType(type), mAttacker(Ronin::RenID_NULL)
        {
        }

#if defined(PLATFORM_WINDOWS)
        inline bool CheckType(ePerformanceType type) const
        {
            return mType == type;
        }

        /*!
         * Creates a temporary variable of the requested type in order to make
         * sure that the existing parameter structure is of the requested type.
         */
        template<class CHILD> const CHILD* const Get() const
        {
            CHILD tmp;
            LECASSERT(CheckType(tmp.mType));
            return static_cast<const CHILD* const>(this);
        }
// PLATFORM_WINDOWS
#else
        /*!
         * Casts the parameter structure to the requested type without type checking.
         */
        template<class CHILD> const CHILD* const Get() const
        {
            return static_cast<const CHILD* const>(this);
        }
 // PLATFORM_WINDOWS
#endif
    } END_DECLARE_ALIGNED;

    /*!
     * Explosion parameters
     */
    DECLARE_ALIGNED class ExplosionParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // MEMBERS
        RavenMath::Vec3 mSourcePos;
        RavenMath::Vec3 mNormal;
        float mSourceMass;
        float mVelocity;

        // CREATORS
        inline ExplosionParams();
        inline ExplosionParams(const ExplosionParams& rhs);
        inline ~ExplosionParams();

        // OPERATORS
        inline ExplosionParams& operator=(const ExplosionParams& rhs);
    } END_DECLARE_ALIGNED;

    ExplosionParams::ExplosionParams() : EuphoriaParamsBase(eExplosion)
    {
    }

    ExplosionParams::ExplosionParams(const ExplosionParams& rhs)
        : EuphoriaParamsBase(eExplosion)
    {
        mAttacker = rhs.mAttacker;
        mNormal = rhs.mNormal;
        mSourceMass = rhs.mSourceMass;
        mSourcePos = rhs.mSourcePos;
        mVelocity = rhs.mVelocity;
    }

    ExplosionParams::~ExplosionParams()
    {

    }

    ExplosionParams& ExplosionParams::operator=(const ExplosionParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
            mNormal = rhs.mNormal;
            mSourceMass = rhs.mSourceMass;
            mSourcePos = rhs.mSourcePos;
            mVelocity = rhs.mVelocity;
        }
        return *this;
    }

    /*!
     * Punch parameters
     */
    DECLARE_ALIGNED class PunchParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // MEMBERS
        RavenMath::Vec3 mForceNormal;   // which direction?
        float mForceMagnitude;          // how hard was the punch?
        crc32_t mImpactBone;            // where hit

        // CREATORS
        inline PunchParams();
        inline PunchParams(const PunchParams& rhs);
        inline ~PunchParams();

        // OPERATORS
        inline PunchParams& operator=(const PunchParams& rhs);
    } END_DECLARE_ALIGNED;

    PunchParams::PunchParams()
        : EuphoriaParamsBase(ePunch)
    {
    }

    PunchParams::PunchParams(const PunchParams& rhs)
        : EuphoriaParamsBase(ePunch)
    {
        mAttacker = rhs.mAttacker;
        mForceNormal = rhs.mForceNormal;
        mForceMagnitude = rhs.mForceMagnitude;
        mImpactBone = rhs.mImpactBone;
    }

    PunchParams::~PunchParams()
    {
    }

    PunchParams& PunchParams::operator=(const PunchParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
            mForceNormal = rhs.mForceNormal;
            mForceMagnitude = rhs.mForceMagnitude;
            mImpactBone = rhs.mImpactBone;
        }
        return *this;
    }

    /*!
     * Shove parameters
     *
     * @note This (and Punch) may end up being "physically" driven, in which case the param structs should be unified
     */
    DECLARE_ALIGNED class ShoveParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16);

    public:
        // MEMBERS
        RavenMath::Vec3 mForceNormal;   // which direction?
        float mForceMagnitude;          // how hard was the punch?
        crc32_t mImpactBone;            // where hit

        // CREATORS
        inline ShoveParams();
        inline ShoveParams(const ShoveParams& rhs);
        inline ~ShoveParams();

        // OPERATORS
        inline ShoveParams& operator=(const ShoveParams& rhs);
    } END_DECLARE_ALIGNED;

    ShoveParams::ShoveParams() : EuphoriaParamsBase(eShove)
    {
    }

    ShoveParams::ShoveParams(const ShoveParams& rhs)
        : EuphoriaParamsBase(eShove)
    {
        mAttacker = rhs.mAttacker;
        mForceMagnitude = rhs.mForceMagnitude;
        mForceNormal = rhs.mForceNormal;
        mImpactBone = rhs.mImpactBone;
    }

    ShoveParams::~ShoveParams()
    {
    }

    ShoveParams& ShoveParams::operator=(const ShoveParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
            mForceMagnitude = rhs.mForceMagnitude;
            mForceNormal = rhs.mForceNormal;
            mImpactBone = rhs.mImpactBone;
        }
        return *this;
    }

    /*!
     * Throw parameters
     */
    DECLARE_ALIGNED class ThrowParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // CREATORS
        inline ThrowParams();
        inline ThrowParams(const ThrowParams& rhs);
        inline ~ThrowParams();

        // OPERATORS
        inline ThrowParams& operator=(const ThrowParams& rhs);
    } END_DECLARE_ALIGNED;

    ThrowParams::ThrowParams()
        : EuphoriaParamsBase(eThrow)
    {
    }

    ThrowParams::ThrowParams(const ThrowParams& rhs)
        : EuphoriaParamsBase(eThrow)
    {
        mAttacker = rhs.mAttacker;
    }

    ThrowParams::~ThrowParams()
    {
    }

    ThrowParams& ThrowParams::operator=(const ThrowParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }

	/*!
	* HitReact parameters
	*/
	DECLARE_ALIGNED class HitReactParams : public EuphoriaParamsBase
	{
		LECALIGNEDCLASS(16)

	public:
		// CREATORS
		inline HitReactParams();
        inline HitReactParams(const HitReactParams& rhs);
        inline ~HitReactParams();

        // OPERATORS
        inline HitReactParams& operator=(const HitReactParams& rhs);
	} END_DECLARE_ALIGNED;

    inline HitReactParams::HitReactParams()
        : EuphoriaParamsBase(eHitReact)
    {
    }
    inline HitReactParams::HitReactParams(const HitReactParams& rhs)
        : EuphoriaParamsBase(eHitReact)
    {
        mAttacker = rhs.mAttacker;
    }

    inline HitReactParams::~HitReactParams()
    {

    }

    HitReactParams& HitReactParams::operator=(const HitReactParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }

    /*!
     * Falling parameters
     */
    DECLARE_ALIGNED class FallingParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // CREATORS
        inline FallingParams();
        inline FallingParams(const FallingParams& rhs);
        inline ~FallingParams();

        // OPERATORS
        inline FallingParams& operator=(const FallingParams& rhs);
    } END_DECLARE_ALIGNED;

    inline FallingParams::FallingParams()
        : EuphoriaParamsBase(eFalling)
    {
    }

    inline FallingParams::FallingParams(const FallingParams& rhs)
        : EuphoriaParamsBase(eFalling)
    {
        mAttacker = rhs.mAttacker;
    }

    inline FallingParams::~FallingParams()
    {
    }

    FallingParams& FallingParams::operator=(const FallingParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }

    /*!
     * Gunshot parameters
     */
    DECLARE_ALIGNED class GunshotParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // CREATORS
        inline GunshotParams();
        inline GunshotParams(const GunshotParams& rhs);
        inline ~GunshotParams();

        // OPERATORS
        inline GunshotParams& operator=(const GunshotParams& rhs);
    } END_DECLARE_ALIGNED;

    GunshotParams::GunshotParams()
        : EuphoriaParamsBase(eGunshot)
    {
    }

    GunshotParams::GunshotParams(const GunshotParams& rhs)
        : EuphoriaParamsBase(eGunshot)
    {
        mAttacker = rhs.mAttacker;
    }

    GunshotParams::~GunshotParams()
    {
    }
    
    GunshotParams& GunshotParams::operator=(const GunshotParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }

    /*!
     * Balance parameters
     */
    DECLARE_ALIGNED class BalanceParams : public EuphoriaParamsBase
    {
        LECALIGNEDCLASS(16)

    public:
        // CREATORS
        inline BalanceParams();
        inline BalanceParams(const BalanceParams& rhs);
        inline ~BalanceParams();

        // OPERATORS
        inline BalanceParams& operator=(const BalanceParams& rhs);
    } END_DECLARE_ALIGNED;

    BalanceParams::BalanceParams()
        : EuphoriaParamsBase(eBalance)
    {
    }

    BalanceParams::BalanceParams(const BalanceParams& rhs)
        : EuphoriaParamsBase(eBalance)
    {
        mAttacker = rhs.mAttacker;
    }

    BalanceParams::~BalanceParams()
    {
    }

    BalanceParams& BalanceParams::operator=(const BalanceParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }

	/*!
	 * Blend parameters
	 */
	DECLARE_ALIGNED class BlendParams : public EuphoriaParamsBase
	{
		LECALIGNEDCLASS(16)

	public:
		// MEMBERS
		GetupPosition mInitialPosition;

		// CREATORS
		inline BlendParams();
        inline BlendParams(const BlendParams& rhs);
        inline ~BlendParams();

        // OPERATORS
        inline BlendParams& operator=(const BlendParams& rhs);
    } END_DECLARE_ALIGNED;

    BlendParams::BlendParams()
        : EuphoriaParamsBase(eBlend), mInitialPosition(eGetupNone)
    {
    }

    BlendParams::BlendParams(const BlendParams& rhs)
        : EuphoriaParamsBase(eBlend), mInitialPosition(eGetupNone)
    {
        mAttacker = rhs.mAttacker;
        mInitialPosition = rhs.mInitialPosition;
    }

    BlendParams::~BlendParams()
    {
    }

    BlendParams& BlendParams::operator=(const BlendParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
            mInitialPosition = rhs.mInitialPosition;
        }
        return *this;
    }

	/*!
	 * EPA parameters
	 */
	DECLARE_ALIGNED class EPAParams : public EuphoriaParamsBase
	{
		LECALIGNEDCLASS(16)

	public:
		// CREATORS
		inline EPAParams();
        inline EPAParams(const EPAParams& rhs);
        inline ~EPAParams();

        // OPERATORS
        inline EPAParams& operator=(const EPAParams& rhs);
    } END_DECLARE_ALIGNED;

    EPAParams::EPAParams()
        : EuphoriaParamsBase(eEPA)
    {
    }

    EPAParams::EPAParams(const EPAParams& rhs)
        : EuphoriaParamsBase(eEPA)
    {
        mAttacker = rhs.mAttacker;
    }

    EPAParams::~EPAParams()
    {
    }

    EPAParams& EPAParams::operator=(const EPAParams& rhs)
    {
        if (this != &rhs)
        {
            mAttacker = rhs.mAttacker;
        }
        return *this;
    }
}

// _EuphoriaParams_H_
#endif
