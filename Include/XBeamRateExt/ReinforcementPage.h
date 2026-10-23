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

#include <XBeamRateExt\XBRExtExp.h>
#include <MFCTools\RebarMaterialComboBox.h>
#include <PsgLib\ConcreteMaterial.h>
#include <PsgLib\PierData2.h>
#include <XBeamRateExt\PierData.h>

class IEditPierData;
class CLongitudinalRebarGrid;
class CStirrupGrid;
class xbrLongitudinalRebarData;
class xbrStirrupData;

interface IReinforcementPageParent
{
   // Returns data for the pier that is being edited
   // Return nullptr if a pier object (part of a bridge object) is not being edited
   virtual CPierData2* GetPierData() = 0;

   // Returns data for the pier that is being edited
   // Return nullptr if the pier that is being edited is part of a bridge
   virtual xbrPierData* GetXBRPierData() = 0;

   virtual CConcreteMaterial& GetConcrete() = 0;
   virtual xbrTypes::PierType GetPierType() = 0;

   virtual WBFL::Materials::Rebar::Type& GetRebarType() = 0;
   virtual WBFL::Materials::Rebar::Grade& GetRebarGrade() = 0;
   virtual CConcreteMaterial& GetConcreteMaterial() = 0;
   virtual xbrLongitudinalRebarData& GetLongitudinalRebar() = 0;
   virtual xbrStirrupData& GetLowerXBeamStirrups() = 0;
   virtual xbrStirrupData& GetFullDepthStirrups() = 0;

   virtual pgsTypes::ConditionFactorType& GetConditionFactorType() = 0;
   virtual Float64& GetConditionFactor() = 0;
};

// CReinforcementPage dialog

// This property page does double duty... it provides a single UI element for
// both standalone XBeam models and for XBeam models that are part of PGSuper/PGSplice piers
//
// When used with standalone XBeamRate, it is a normal property page. When used with
// PGSuper/PGSplice piers, this property page is an extension to Edit Pier dialog.

class XBREXTCLASS CReinforcementPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CReinforcementPage)

public:
   CReinforcementPage(IReinforcementPageParent* pParent);
	virtual ~CReinforcementPage();

   IReinforcementPageParent* GetPageParent();

// Dialog Data

   void GetRebarMaterial(WBFL::Materials::Rebar::Type* pType,WBFL::Materials::Rebar::Grade* pGrade);

   void OnEnableDelete(UINT nIDC,bool bEnableDelete);

protected:
   IReinforcementPageParent* m_pParent;

	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	CEdit	m_ctrlEc;
	CButton m_ctrlEcCheck;
	CEdit	m_ctrlFc;

	//{{AFX_MSG(CPierLayoutPage)
	virtual BOOL OnInitDialog() override;
   virtual BOOL OnSetActive() override;
   afx_msg void OnMoreProperties();
   afx_msg void OnBnClickedEc();
   afx_msg void OnAddRebar();
   afx_msg void OnRemoveRebar();
   afx_msg void OnAddLowerXBeam();
   afx_msg void OnRemoveLowerXBeam();
   afx_msg void OnAddFullDepth();
   afx_msg void OnRemoveFullDepth();
   afx_msg void OnChangeFc();
   afx_msg void OnLowerXBeamSymmetry();
   afx_msg void OnFullDepthSymmetry();
   afx_msg void OnConditionFactorTypeChanged();
   afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

   CRebarMaterialComboBox m_cbRebar;
   CString m_strUserEc;

   CLongitudinalRebarGrid* m_pRebarGrid;

   CStirrupGrid* m_pLowerXBeamGrid;
   CStirrupGrid* m_pFullDepthGrid;

   void UpdateConcreteTypeLabel();
   void UpdateEc();
   void UpdateStirrupGrids();
};
