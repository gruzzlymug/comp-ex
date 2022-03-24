#ifndef EUPHORIA_BEHAVIOR_CRC_H
#define EUPHORIA_BEHAVIOR_CRC_H

/*!
 * This structure holds CRCs for euphoria behaviors as enumerations. This allows
 * code using them to load CRCs as immediate values rather than having to call stricrc32
 * every time the CRC is needed, which is unnecessary and much slower.
 *
 * Using enumerations is also preferable to statics global to a translation unit (bad)
 * or local to a function (worse). Using statics requires a memory read and for local
 * statics, a test to see if the variable has been initialized every time the function
 * is called. Having this stuff defined in one place also promotes consistency across
 * modules.
 *
 * There are a couple downsides to using enumerations for CRCs. If the CRC generation
 * algorithm changes, then each of these constants will need to be updated manually.
 * Also, the PC and 360 compiler gives signed/unsigned mismatch warnings when using
 * an enum with a negative value as a crc32_t function parameter. crc32_t is a typedef
 * for an unsigned int. To avoid this warning or having to cast everywhere, a Microsoft-specific
 * extension is required to explicitly specify the underlying type for the enums.
 * Another pragma is required to avoid warnings about the non-standard extension.
 * Setting it up this way allows the mess to be isolated within this file where it
 * can be managed.
 *
 * gcc handles enums as unsigneds just fine.
 */
struct EuphoriaBehaviorCRC
{
#ifdef PLATFORM_PS3
    enum
#else
// Disable warning about non-standard extension used to specify underlying type for enum
#pragma warning(disable : 4480)
    enum : unsigned int
#endif
    {
        // Balance Performance
        eBlockWithArms  = 0x56e4d202, //  1457836546, "BlockWithArms_BalancingAct"
        eStaggerBalance = 0x51c41231, //  1371804209, "StaggerBalance6_BalancingAct"
        eBalanceArms    = 0xafc45887, // -1346086777, "BalanceArms_BalancingAct"
        eLandingFall    = 0x574d62a9, //  1464689321, "LandingFallWindmill_BalancingAct"
        eFallDown       = 0xa6344d61, // -1506521759, "FallDown_BalancingAct"

        // Blend Performance
        // Note: uses eCatchFall from Shove Performance
        eBlendToFrame   = 0x375eb3e8, //   928953320, "BlendToAnimationFrame_Shared"
        eSuperBlend     = 0xb7227790, // -1222477936, "SuperBlendToAnimation_Shared"
        eBlendToAnim    = 0x3bd9a2b3, //  1004118707, "BlendToAnimation_Shared"
        eUnstick        = 0xfade5d3d, //   -86090435, "Blend_Unstick"
        eHeadHit        = 0xc479fbc9, //  -998638647, "HeadHit_Blend_Fortune"

        // Explosion Performance
        eForce          = 0x4c382826, //  1278748710, "Force_PerfExplosion"

        // Falling Performance
        eFlail          = 0xc279479c, // -1032239204, "FlailThroughAir_DropZone"
        eHeadFirstFall  = 0xb3a05993, // -1281336941, "HeadFirstFall_DropZone"
        eFeetFirstFall  = 0x25bdf540, //   633206080, "FeetFirstFall_DropZone"
        eGrab           = 0xdbc2781a, //  -608012262, "GrabLedge_GrabNGo"
        eHang           = 0x201499db, //   538221019, "Hang_GrabNGo"
        eSlide          = 0x1445a011, //   340107281, "Slide_GrabNGo"
        eImpactReaction = 0xd8791725, //  -663152859, "ImpactReaction_DropZone"

        // Hit React Performance
        eReact          = 0xe7d41823, //  -405530589, "React_PerfHitReact2"
        eTransfer_HR    = 0x513f2d38, //  1363094840, "Transfer_PerfHitReact2"
        eCrunch_HR      = 0x9433ac99, // -1808552807, "Crunch_PerfHitReact2"
        eImpact_HR      = 0xb014c5fa, // -1340815878, "Impact_PerfHitReact2"

        // Punch Performance
        ePunch          = 0x5c868676, //  1552320118, "Force_Punch"
        eStaggerPunch   = 0x6f164534, //  1863730484, "Stagger_Punch"
        eFall           = 0x479b57db, //  1201362907, "Fall_Punch"

        // Shove Performance
        eStagger        = 0x59713e57, //  1500593751, "Stagger_Shoved"
        eCatchFall      = 0x15d5d6dc, //   366335708, "CatchFall_Shoved"

        // Throw Performance
        eFly            = 0x636c62e9, //  1668047593, "Fly_PerfThrow"
        eTransfer       = 0x20bda229, //   549298729, "Transfer_PerfThrow"
        eBreak          = 0x128bb71f, //   311146271, "Break_PerfThrow"
        eCrunch         = 0xa68287a5, // -1501395035, "Crunch_PerfThrow"
        eTorque         = 0xdb3b7039, //  -616861639, "Torque_PerfThrow"
        eImpact         = 0x45e48c0f, //  1172605967, "Impact_PerfThrow"
    };
};

#ifndef PLATFORM_PS3
#pragma warning(default : 4480)
#endif

// EUPHORIA_BEHAVIOR_CRC_H
#endif
