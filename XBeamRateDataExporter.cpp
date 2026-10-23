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
#include "XBeamRateAgent.h"
#include "XBeamRateDataExporter.h"

#include <IFace\Project.h>

STDMETHODIMP CXBeamRateDataExporter::Init(UINT nCmdID)
{
   return S_OK;
}

CString CXBeamRateDataExporter::GetMenuText() const
{
   return CString("Piers to XBRate");
}

HBITMAP CXBeamRateDataExporter::GetBitmapHandle() const
{
   return nullptr;
}

CString CXBeamRateDataExporter::GetCommandHintText() const
{
   return CString("Export a pier to XBRate\nTool tip text");
}

HRESULT CXBeamRateDataExporter::Export(std::shared_ptr<WBFL::EAF::Broker> pBroker)
{
   GET_IFACE2(pBroker, IXBRExport, pExport);
   if ( !pExport )
   {
      AfxMessageBox(_T("XBRate Extension is not enabled. Piers cannot be exported"),MB_ICONEXCLAMATION | MB_OK);
      return E_FAIL;
   }

   return pExport->BatchExport();
}
