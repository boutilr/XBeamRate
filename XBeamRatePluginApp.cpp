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

// Plugin.cpp : Implementation of CXBeamRatePluginApp
#include "stdafx.h"
#include "resource.h"

#include "XBeamRateApp.h"

#include "XBeamRatePluginApp.h"

#include "XBeamRateDocTemplate.h"

#include "XBeamRateDoc.h"
#include "XBeamRateView.h"
#include "XBeamRateChildFrame.h"

#include <EAF\EAFUnits.h>
#include <MFCTools\AutoRegistry.h>

#include "XBeamRateCommandLineInfo.h"
#include "XBeamRateCommandLineProcessor.h"

#include <XBeamRateVersionInfoImpl.h>

void CXBeamRatePluginApp::GetAppUnitSystem(IAppUnitSystem** ppAppUnitSystem)
{
   m_AppUnitSystem.CopyTo(ppAppUnitSystem);
}

BOOL CXBeamRatePluginApp::Init(CEAFApp* pParent)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   LoadRegistryValues();

   auto plugin = std::dynamic_pointer_cast<WBFL::EAF::IPluginApp>(shared_from_this());
   m_DocumentationImpl.Init(plugin);

   return TRUE;
}

void CXBeamRatePluginApp::Terminate()
{
   SaveRegistryValues();
}

void CXBeamRatePluginApp::IntegrateWithUI(BOOL bIntegrate)
{
}

std::vector<CEAFDocTemplate*> CXBeamRatePluginApp::CreateDocTemplates()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   std::vector<CEAFDocTemplate*> templates;

   std::unique_ptr<CXBeamRateDocTemplate> pDocTemplate = std::make_unique<CXBeamRateDocTemplate>(
      IDR_XBEAMRATE,
      nullptr,
		RUNTIME_CLASS(CXBeamRateDoc),
		RUNTIME_CLASS(CXBeamRateChildFrame),
		RUNTIME_CLASS(CXBeamRateView),
      nullptr,1);

   auto plugin = std::dynamic_pointer_cast<WBFL::EAF::IPluginApp>(shared_from_this());
   pDocTemplate->SetPluginApp(plugin);

   templates.push_back(pDocTemplate.release());
   return templates;
}

HMENU CXBeamRatePluginApp::GetSharedMenuHandle()
{
   return nullptr;
}

CString CXBeamRatePluginApp::GetName()
{
   return CString("XBRate");
}

CString CXBeamRatePluginApp::GetDocumentationSetName()
{
   return GetName();
}

CString CXBeamRatePluginApp::GetDocumentationURL()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return m_DocumentationImpl.GetDocumentationURL();
}

CString CXBeamRatePluginApp::GetDocumentationMapFile()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return m_DocumentationImpl.GetDocumentationMapFile();
}

void CXBeamRatePluginApp::LoadDocumentationMap()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return m_DocumentationImpl.LoadDocumentationMap();
}

std::pair<WBFL::EAF::HelpResult,CString> CXBeamRatePluginApp::GetDocumentLocation(LPCTSTR lpszDocSetName,UINT nID)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return m_DocumentationImpl.GetDocumentLocation(lpszDocSetName,nID);
}


void CXBeamRatePluginApp::LoadRegistryValues()
{
   LoadCustomReportInformation();
}

void CXBeamRatePluginApp::SaveRegistryValues()
{
   SaveCustomReportInformation();
}

void CXBeamRatePluginApp::LoadCustomReportInformation()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFCustomReportMixin::LoadCustomReportInformation();
}

void CXBeamRatePluginApp::SaveCustomReportInformation()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   CEAFCustomReportMixin::SaveCustomReportInformation();
}

////////////////////////////////////////////////////////
// IEAFCommandLineInfo

CString CXBeamRatePluginApp::GetCommandLineAppName() const
{
   return GetAppName();
}

CString CXBeamRatePluginApp::GetUsageMessage()
{
   CXBRateCommandLineInfo cmdInfo;
   return cmdInfo.GetUsageMessage();
}

BOOL CXBeamRatePluginApp::ProcessCommandLineOptions(CEAFCommandLineInfo& cmdInfo)
{
   CXBRateCommandLineProcessor processor;
   return processor.ProcessCommandLineOptions(cmdInfo);
}
