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

// BridgeLinkTestPlugin.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f BridgeLinkTestPluginps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <MFCTools\VersionInfo.h>

#include <WBFLGeometry.h>
#include <WBFLUnitServer.h>


#include <initguid.h>
#include <EAF\PluginApp.h>
#include "SectionCut.h"

// EAF Common Interfaces
#include <EAF\EAFDisplayUnits.h>
#include <EAF/EAFProgress.h>
#include <EAF/EAFStatusCenter.h>

// XBeam Rate Interfaces
#include <IFace\XBeamRateAgent.h>
#include <IFace\Project.h>
#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>
#include <IFace\Pier.h>
#include <IFace\RatingSpecification.h>
#include <IFace\Test.h>
#include <IFace\PointOfInterest.h>

// PGSuper/PGSplice Interfaces
#include <IFace\ExtendUI.h>
#include <..\..\PGSuper\Include\IFace\Project.h>
#include <..\..\PGSuper\Include\IFace\DocumentType.h>
#include <IFace\EditByUI.h>
#include <IFace\Bridge.h>
#include <IFace\Selection.h>
#include <IFace\ViewEvents.h>
#include <IFace\Intervals.h>
#include <..\..\PGSuper\Include\IFace\PointOfInterest.h>
#include <IFace\Alignment.h>

#include "PGSComponentInfo.h"


#include <WBFLUnitServer_i.c>
#include <WBFLGeometry_i.c>

#include "XBeamRateDocProxyAgent.h"
#include <XBeamRateCatCom.h>

#include <BridgeLinkCATID.h>
#include <PGSuperCatCom.h>
#include <PGSpliceCatCom.h>
#include <Plugins\PGSuperIEPlugin.h>

#include "XBeamRateApp.h"

#include <EAF\EAFDocTemplate.h>
#include "XBeamRateDoc.h"
#include "ComponentInfo.h"


#include "CLSID.h"
#include "XBeamRatePluginApp.h"
#include "XBeamRateDataExporter.h"
#include "PGSuperComponentInfo.h"
#include "PGSpliceComponentInfo.h"
#include "XBeamRateAgent.h"
#include <EAF\ComponentModule.h>
WBFL::EAF::ComponentModule Module_;

EAF_BEGIN_OBJECT_MAP(ObjectMap)
   EAF_OBJECT_ENTRY(CLSID_XBeamRatePluginApp, CXBeamRatePluginApp)
   EAF_OBJECT_ENTRY(CLSID_XBeamRateDataExporter, CXBeamRateDataExporter)
   EAF_OBJECT_ENTRY(CLSID_XBeamRateComponentInfo, CXBeamRateComponentInfo)
   EAF_OBJECT_ENTRY(CLSID_XBRatePGSuperComponentInfo, CPGSuperComponentInfo)
   EAF_OBJECT_ENTRY(CLSID_XBRatePGSpliceComponentInfo, CPGSpliceComponentInfo)
   EAF_OBJECT_ENTRY(CLSID_XBeamRateAgent, CXBeamRateAgent)
EAF_END_OBJECT_MAP()



BEGIN_MESSAGE_MAP(CXBeamRateApp, CWinApp)
	//{{AFX_MSG_MAP(CXBeamRateApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CXBeamRateApp theApp;

CXBeamRateApp::CXBeamRateApp()
{
}

BOOL CXBeamRateApp::InitInstance()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   Module_.Init(ObjectMap);

   EAFGetApp()->LoadManifest(_T("Manifest.XBeamRate"));

   GXInit();

   SetRegistryKey( _T("Washington State Department of Transportation") );

   // Using a shared menu
   // See MSKB Article ID: Q118435, "Sharing Menus Between MDI Child Windows"
   m_hSharedMenu = nullptr; /*::LoadMenu( m_hInstance, MAKEINTRESOURCE(IDR_???) );*/

   return CWinApp::InitInstance();
}

int CXBeamRateApp::ExitInstance()
{
   // if the doc template uses a shared menu, destroy it here
   // release the shared menu
   //::DestroyMenu( m_hSharedMenu );

   GXForceTerminate();
   Module_.Term();
   return CWinApp::ExitInstance();
}

CString CXBeamRateApp::GetVersion(bool bIncludeBuildNumber) const
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CWinApp* pApp = AfxGetApp();
   CString strExe( pApp->m_pszExeName );
   strExe += ".dll";

   CVersionInfo verInfo;
   verInfo.Load(strExe);
   
#if defined _DEBUG || defined _BETA_VERSION
   // always include the build number in debug and beta versions
   bIncludeBuildNumber = true;
#endif
   CString strVersion = verInfo.GetProductVersionAsString(bIncludeBuildNumber);

   return strVersion;
}

BOOL CXBeamRateApp::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
   // in order scan the message map for this plugin, the module state must be set
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return CWinApp::OnCmdMsg(nID,nCode,pExtra,pHandlerInfo);
}
