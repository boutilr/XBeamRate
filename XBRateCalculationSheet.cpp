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


#include "stdafx.h"
#include <AgentTools.h>
#include "XBRateCalculationSheet.h"
#include <IFace\Project.h>
#include <IFace\VersionInfo.h>

#include <EAF\EAFDocument.h>


XBRateCalculationSheet::XBRateCalculationSheet() :
WsdotCalculationSheet()
{
   auto pBroker = EAFGetBroker();
   GET_IFACE2(pBroker, IXBRProjectProperties,pProj);

   // set pgsuper-specific properties
   SetBridgeName(pProj->GetBridgeName());
   SetBridgeID(pProj->GetBridgeID());
   SetJobNumber(pProj->GetJobNumber());
   SetEngineer(pProj->GetEngineer());
   SetCompany(pProj->GetCompany());

   // set the bottom title
   GET_IFACE2(pBroker, IXBRVersionInfo,pVerInfo);

   CEAFDocument* pDoc = EAFGetDocument();
   CEAFDocTemplate* pDocTemplate = (CEAFDocTemplate*)pDoc->GetDocTemplate();
   auto pluginApp = pDocTemplate->GetPluginApp();
   CString strBottomTitle;
   strBottomTitle.Format(_T("%s™ Version %s, Copyright © %4d, WSDOT, All rights reserved"),pluginApp->GetName(),pVerInfo->GetVersion(true),WBFL::System::Date().Year());

   SetTitle(strBottomTitle);

   // set the document name
   CString path = pDoc->GetPathName();
   SetFileName(path);
}
