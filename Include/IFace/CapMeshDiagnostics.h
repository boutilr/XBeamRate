///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright © 1999-2026  Washington State Department of Transportation
//                        Bridge and Structures Office
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the Alternate Route Open Source License as
// published by the Washington State Department of Transportation,
// Bridge and Structures Office.
//
// This program is distributed in the hope that it will be useful, but
// distribution is AS IS, WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the Alternate Route Open Source License for more details.
//
// You should have received a copy of the Alternate Route Open Source
// License along with this program; if not, write to the Washington
// State Department of Transportation, Bridge and Structures Office,
// P.O. Box  47340, Olympia, WA 98503, USA or e-mail
// Bridge_Support@wsdot.wa.gov
///////////////////////////////////////////////////////////////////////

#pragma once

#include <charconv>
#include <cmath>
#include <cstddef>
#include <string_view>

inline constexpr double DefaultCapMeshDepthPercent = 5.0;

struct CapMeshExportOptions
{
   bool enabled;
   bool valid;
   bool useMidspanStiffness;
   double depthPercent;
};

inline CapMeshExportOptions GetCapMeshExportOptions()
{
   // The environment is inherited at launch. A separate process builds a
   // fresh analysis model and rating cache for each requested refinement.
   char value[64];
   ::SetLastError(ERROR_SUCCESS);
   const DWORD length = ::GetEnvironmentVariableA("XBR_MESH_DEPTH_PERCENT", value, static_cast<DWORD>(sizeof(value)));
   if (length == 0 && ::GetLastError() == ERROR_ENVVAR_NOT_FOUND)
   {
      return {false, true, false, DefaultCapMeshDepthPercent};
   }
   if (length == 0 || length >= sizeof(value))
   {
      return {true, false, false, DefaultCapMeshDepthPercent};
   }
   if (std::string_view(value, length) == "midspan")
   {
      return {true, true, true, DefaultCapMeshDepthPercent};
   }

   double percent = 0.0;
   const auto [end, error] = std::from_chars(value, value + length, percent);
   if (error != std::errc{} || end != value + length || !std::isfinite(percent) ||
       percent <= 0.0 || percent > 100.0)
   {
      return {true, false, false, DefaultCapMeshDepthPercent};
   }
   return {true, true, false, percent};
}

struct CapMeshStatistics
{
   std::size_t elementCount;
   std::size_t splitLimitHits;
};

// {AB6A0ED0-8F11-4EAA-A18C-E0FBC00B66D3}
DEFINE_GUID(IID_IXBRCapMeshDiagnostics,
0xab6a0ed0, 0x8f11, 0x4eaa, 0xa1, 0x8c, 0xe0, 0xfb, 0xc0, 0x0b, 0x66, 0xd3);
class IXBRCapMeshDiagnostics
{
public:
   virtual CapMeshStatistics GetCapMeshStatistics(PierIDType pierID) const = 0;
};
