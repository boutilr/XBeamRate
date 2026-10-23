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

#include <EAF\ComponentObject.h>
#include <EAF\PluginApp.h>
#include <WBFLUnitServer.h>
#include <EAF\EAFCustomReport.h>
#include <EAF\PluginAppDocumentationImpl.h>
#include <EAF\EAFUnits.h>

/////////////////////////////////////////////////////////////////////////////
// CXBeamRatePluginApp
class CXBeamRatePluginApp : public WBFL::EAF::ComponentObject,
   public CEAFCustomReportMixin,
   public WBFL::EAF::IPluginApp,
   public WBFL::EAF::IAppCommandLine
{
public:
	CXBeamRatePluginApp()
	{
      m_bDisplayFavoriteReports = FALSE;
	  EAFCreateAppUnitSystem(&m_AppUnitSystem);
	}

   LPCTSTR GetAppName() const { return _T("XBeamRate"); }

   void GetAppUnitSystem(IAppUnitSystem** ppAppUnitSystem);

// IPluginApp
public:
   BOOL Init(CEAFApp* pParent) override;
   void Terminate() override;
   void IntegrateWithUI(BOOL bIntegrate) override;
   std::vector<CEAFDocTemplate*> CreateDocTemplates() override;
   HMENU GetSharedMenuHandle() override;
   CString GetName() override;
   CString GetDocumentationSetName() override;
   CString GetDocumentationURL() override;
   CString GetDocumentationMapFile() override;
   void LoadDocumentationMap() override;
   std::pair<WBFL::EAF::HelpResult,CString> GetDocumentLocation(LPCTSTR lpszDocSetName,UINT nID) override;

   void LoadCustomReportInformation() override;
   void SaveCustomReportInformation() override;

// IAppCommandLine
public:
   CString GetCommandLineAppName() const override;
   CString GetUsageMessage() override;
   BOOL ProcessCommandLineOptions(CEAFCommandLineInfo& cmdInfo) override;

private:
   CComPtr<IAppUnitSystem> m_AppUnitSystem;

   void LoadRegistryValues();
   void SaveRegistryValues();

   WBFL::EAF::PluginAppDocumentationImpl m_DocumentationImpl;
};
