/*********************************************************\
EuphoriaConstants.h
\*********************************************************/

#ifndef _EuphoriaConstants_H_
#define _EuphoriaConstants_H_
#pragma once

#include "RavenMath.h"
#include "LECVectorAligned.h"
#include "Rens/RenTypes.h"
#include "RoninConfig.h"

namespace Ronin
{
    inline crc32_t GetEuphoriaPlugInName()
    {
        static crc32_t name = stricrc32("FortuneEuphoria");
        return name;
    }
}

// _EuphoriaConstants_H_
#endif
