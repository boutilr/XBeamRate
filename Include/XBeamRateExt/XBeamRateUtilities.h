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

#include <XBeamRateExt\XBRExtExp.h>
#include <PsgLib\PierData2.h>

xbrTypes::PierType XBREXTFUNC GetPierType(pgsTypes::BoundaryConditionType bcType);
xbrTypes::PierType XBREXTFUNC GetPierType(pgsTypes::PierSegmentConnectionType connType);
bool XBREXTFUNC IsStandAlone();
bool XBREXTFUNC IsPGSExtension();
bool XBREXTFUNC CanModelPier(PierIDType pierID,StatusGroupIDType statusGroupID,StatusCallbackIDType callbackID);

void XBREXTFUNC GetLaneInfo(Float64 Wcc,Float64* pWlane,IndexType* pnLanes,Float64* pWloadedLane);

#define RATING_LIMIT_STATE_COUNT 9
#define GET_INDEX(ls) GetIndexFromLimitState(ls)
int XBREXTFUNC GetIndexFromLimitState(pgsTypes::LimitState ls);
