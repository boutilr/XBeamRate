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
#include "resource.h"
#include "dllmain.h"

#include "ProjectAgentCLSID.h"
#include "ProjectAgentImp.h"
#include <EAF\ComponentModule.h>

WBFL::EAF::ComponentModule Module_;

EAF_BEGIN_OBJECT_MAP(ObjectMap)
   EAF_OBJECT_ENTRY(CLSID_XBeamRateProjectAgent, CProjectAgentImp)
EAF_END_OBJECT_MAP()

class CProjectAgentModuleApp : public CWinApp
{
public:

// Overrides
	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CProjectAgentModuleApp, CWinApp)
END_MESSAGE_MAP()

CProjectAgentModuleApp theApp;

BOOL CProjectAgentModuleApp::InitInstance()
{
   Module_.Init(ObjectMap);
   GXInit();

   SetRegistryKey(_T("Washington State Department of Transportation"));

   return CWinApp::InitInstance();
}

int CProjectAgentModuleApp::ExitInstance()
{
   Module_.Term();
   GXForceTerminate();
	return CWinApp::ExitInstance();
}
