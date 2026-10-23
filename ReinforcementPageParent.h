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
#include <IFace\ExtendUI.h>
#include <XBeamRateExt\ReinforcementPage.h>
#include <XBeamRateExt\PierData.h>

// This class is a wrapper between the reinforcement property page
// and the pier data
class CReinforcementPageParent : public IReinforcementPageParent
{
public:
   CReinforcementPageParent();
   void SetEditPierData(IEditPierData* pEditPierData,const xbrPierData& pierData);

// IReinforcementPageParent
   virtual CPierData2* GetPierData() override;
   virtual xbrPierData* GetXBRPierData() override;
   virtual CConcreteMaterial& GetConcrete() override;
   virtual xbrTypes::PierType GetPierType() override;
   virtual WBFL::Materials::Rebar::Type& GetRebarType() override;
   virtual WBFL::Materials::Rebar::Grade& GetRebarGrade() override;
   virtual CConcreteMaterial& GetConcreteMaterial() override;
   virtual xbrLongitudinalRebarData& GetLongitudinalRebar() override;
   virtual xbrStirrupData& GetLowerXBeamStirrups() override;
   virtual xbrStirrupData& GetFullDepthStirrups() override;
   virtual pgsTypes::ConditionFactorType& GetConditionFactorType() override;
   virtual Float64& GetConditionFactor() override;

private:
   IEditPierData* m_pEditPierData;
   xbrPierData m_PierData;
};