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

#include "SuperstructureLayoutPage.h"
#include "PierLayoutPage.h"
#include <XBeamRateExt\ReinforcementPage.h>
#include "BearingsPage.h"
#include "DesignLiveLoadReactionsPage.h"
#include "LegalLiveLoadReactionsPage.h"
#include "PermitLiveLoadReactionsPage.h"
#include "txnEditPier.h"

// CPierDlg

class CPierDlg : public CPropertySheet, public IReinforcementPageParent
{
	DECLARE_DYNAMIC(CPierDlg)

public:
	CPierDlg(UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	CPierDlg(LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	virtual ~CPierDlg();

   void SetEditPierData(const txnEditPierData& pierData);
   txnEditPierData GetEditPierData();

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

protected:
	DECLARE_MESSAGE_MAP()

   void VerifyLongitudinalReinforcement();

   void Init();

   txnEditPierData m_PierData;

public:
   friend CSuperstructureLayoutPage;
   friend CPierLayoutPage;
   friend CReinforcementPage;
   friend CBearingsPage;
   friend CDesignLiveLoadReactionsPage;
   friend CLegalLiveLoadReactionsPage;
   friend CPermitLiveLoadReactionsPage;

   CSuperstructureLayoutPage m_SuperstructurePage;
   CPierLayoutPage m_SubstructurePage;
   CReinforcementPage m_ReinforcementPage;
   CBearingsPage m_BearingsPage;
   CDesignLiveLoadReactionsPage m_DesignLiveLoadPage;
   CLegalLiveLoadReactionsPage m_LegalLiveLoadPage;
   CPermitLiveLoadReactionsPage m_PermitLiveLoadPage;
};


