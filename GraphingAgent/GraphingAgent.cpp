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

// GraphingAgent.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f GraphingAgentps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include "dllmain.h"
#include <initguid.h>
#include "GraphingAgent.h"

#include "GraphingAgentCLSID.h"

#include "GraphingAgentImp.h"

#include "XBeamRateCatCom.h"

#include <EAF/EAFTransactions.h>
#include <EAF/EAFDisplayUnits.h>
#include <EAF/EAFProgress.h>
#include <EAF/EAFStatusCenter.h>
#include <EAF/EAFGraphManager.h>

#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <IFace\PointOfInterest.h>
#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>
#include <IFace\XBeamRateAgent.h>
#include <IFace\Pier.h>
#include <..\..\PGSuper\Include\IFace\Project.h>
#include <IFace\Views.h>
#include <EAF\EAFUIIntegration.h>
