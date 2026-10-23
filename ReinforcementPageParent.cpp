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
#include "ReinforcementPageParent.h"
#include <XBeamRateExt\XBeamRateUtilities.h>



CReinforcementPageParent::CReinforcementPageParent()
{
   m_pEditPierData = nullptr;
}

void CReinforcementPageParent::SetEditPierData(IEditPierData* pEditPierData,const xbrPierData& pierData)
{
   m_pEditPierData = pEditPierData;
   m_PierData = pierData;
}

CPierData2* CReinforcementPageParent::GetPierData()
{
   return m_pEditPierData->GetPierData();
}

xbrPierData* CReinforcementPageParent::GetXBRPierData()
{
   return &m_PierData;
}

CConcreteMaterial& CReinforcementPageParent::GetConcrete()
{
   return m_pEditPierData->GetPierData()->GetConcrete();
}

xbrTypes::PierType CReinforcementPageParent::GetPierType()
{
   if ( m_pEditPierData->GetPierData()->IsBoundaryPier() )
   {
      return ::GetPierType(m_pEditPierData->GetPierData()->GetBoundaryConditionType());
   }
   else
   {
      return ::GetPierType(m_pEditPierData->GetPierData()->GetSegmentConnectionType());
   }
}

WBFL::Materials::Rebar::Type& CReinforcementPageParent::GetRebarType()
{
   return m_PierData.GetRebarType();
}

WBFL::Materials::Rebar::Grade& CReinforcementPageParent::GetRebarGrade()
{
   return m_PierData.GetRebarGrade();
}

CConcreteMaterial& CReinforcementPageParent::GetConcreteMaterial()
{
   return m_PierData.GetConcreteMaterial();
}

xbrLongitudinalRebarData& CReinforcementPageParent::GetLongitudinalRebar()
{
   return m_PierData.GetLongitudinalRebar();
}

xbrStirrupData& CReinforcementPageParent::GetLowerXBeamStirrups()
{
   return m_PierData.GetLowerXBeamStirrups();
}

xbrStirrupData& CReinforcementPageParent::GetFullDepthStirrups()
{
   return m_PierData.GetFullDepthStirrups();
}

pgsTypes::ConditionFactorType& CReinforcementPageParent::GetConditionFactorType()
{
   return m_PierData.GetConditionFactorType();
}

Float64& CReinforcementPageParent::GetConditionFactor()
{
   return m_PierData.GetConditionFactor();
}
