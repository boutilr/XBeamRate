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


// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "dllmain.h"

#include "PierAgentCLSID.h"
#include "PierAgentImp.h"
#include <EAF\ComponentModule.h>

WBFL::EAF::ComponentModule _Module;

EAF_BEGIN_OBJECT_MAP(ObjectMap)
   EAF_OBJECT_ENTRY(CLSID_XBeamRatePierAgent, CPierAgentImp)
EAF_END_OBJECT_MAP()


/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      _Module.Init(ObjectMap);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
      _Module.Term();
   }
   return TRUE;    // ok
}
