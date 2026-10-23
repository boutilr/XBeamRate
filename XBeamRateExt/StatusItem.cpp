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
#include <EAF/EAFStatusCenter.h>
#include <XBeamRateExt\StatusItem.h>


xbrBridgeStatusItem::xbrBridgeStatusItem(StatusGroupIDType statusGroupID,StatusCallbackIDType callbackID,LPCTSTR strDescription) :
WBFL::EAF::StatusItem(statusGroupID,callbackID,strDescription)
{
}

bool xbrBridgeStatusItem::IsEqual(std::shared_ptr<const WBFL::EAF::StatusItem> pOther) const
{
   auto other = std::dynamic_pointer_cast<const xbrBridgeStatusItem>(pOther);
   if ( !other )
   {
      return false;
   }

   if ( CString(GetDescription()) != CString(other->GetDescription()) )
   {
      return false;
   }

   return true;
}

//////////////////////////////
xbrBridgeStatusCallback::xbrBridgeStatusCallback(WBFL::EAF::StatusSeverityType severity,UINT helpID):
m_Severity(severity), m_HelpID(helpID)
{
}

WBFL::EAF::StatusSeverityType xbrBridgeStatusCallback::GetSeverity() const
{
   return m_Severity;
}

void xbrBridgeStatusCallback::Execute(std::shared_ptr<WBFL::EAF::StatusItem> pStatusItem)
{
   EAFShowStatusMessage(pStatusItem,m_Severity,TRUE, m_Severity == WBFL::EAF::StatusSeverityType::Error ? TRUE : FALSE);
}
