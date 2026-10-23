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

// PierDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ProjectAgent.h"
#include "PierDlg.h"




// CPierDlg

IMPLEMENT_DYNAMIC(CPierDlg, CPropertySheet)

CPierDlg::CPierDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
   m_ReinforcementPage(this)
{
   Init();
}

CPierDlg::CPierDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage),
   m_ReinforcementPage(this)
{
   Init();
}

CPierDlg::~CPierDlg()
{
}

void CPierDlg::SetEditPierData(const txnEditPierData& pierData)
{
   m_PierData = pierData;
}

txnEditPierData CPierDlg::GetEditPierData()
{
   VerifyLongitudinalReinforcement();
   return m_PierData;
}

//////////////////////////////////////////////////
// IReinforcementPageParent
CPierData2* CPierDlg::GetPierData()
{
   return nullptr; // we are not associated with pier data for an entire bridge
}

xbrPierData* CPierDlg::GetXBRPierData()
{
   return &m_PierData.m_PierData;
}

CConcreteMaterial& CPierDlg::GetConcrete()
{
   return m_PierData.m_PierData.GetConcreteMaterial();
}

xbrTypes::PierType CPierDlg::GetPierType()
{
   return m_PierData.m_PierData.GetPierType();
}

WBFL::Materials::Rebar::Type& CPierDlg::GetRebarType()
{
   return m_PierData.m_PierData.GetRebarType();
}

WBFL::Materials::Rebar::Grade& CPierDlg::GetRebarGrade()
{
   return m_PierData.m_PierData.GetRebarGrade();
}

CConcreteMaterial& CPierDlg::GetConcreteMaterial()
{
   return m_PierData.m_PierData.GetConcreteMaterial();
}

xbrLongitudinalRebarData& CPierDlg::GetLongitudinalRebar()
{
   return m_PierData.m_PierData.GetLongitudinalRebar();
}

xbrStirrupData& CPierDlg::GetLowerXBeamStirrups()
{
   return m_PierData.m_PierData.GetLowerXBeamStirrups();
}

xbrStirrupData& CPierDlg::GetFullDepthStirrups()
{
   return m_PierData.m_PierData.GetFullDepthStirrups();
}

pgsTypes::ConditionFactorType& CPierDlg::GetConditionFactorType()
{
   return m_PierData.m_PierData.GetConditionFactorType();
}

Float64& CPierDlg::GetConditionFactor()
{
   return m_PierData.m_PierData.GetConditionFactor();
}

//////////////////////////////////////////////////
void CPierDlg::Init()
{
   m_psh.dwFlags |= PSH_HASHELP | PSH_NOAPPLYNOW;

   m_SuperstructurePage.m_psp.dwFlags  |= PSP_HASHELP;
   m_SubstructurePage.m_psp.dwFlags    |= PSP_HASHELP;
   m_ReinforcementPage.m_psp.dwFlags   |= PSP_HASHELP;
   m_BearingsPage.m_psp.dwFlags        |= PSP_HASHELP;
   m_DesignLiveLoadPage.m_psp.dwFlags |= PSP_HASHELP;
   m_LegalLiveLoadPage.m_psp.dwFlags |= PSP_HASHELP;
   m_PermitLiveLoadPage.m_psp.dwFlags |= PSP_HASHELP;

   AddPage(&m_SuperstructurePage);
   AddPage(&m_SubstructurePage);
   AddPage(&m_ReinforcementPage);
   AddPage(&m_BearingsPage);
   AddPage(&m_DesignLiveLoadPage);
   AddPage(&m_LegalLiveLoadPage);
   AddPage(&m_PermitLiveLoadPage);
}


BEGIN_MESSAGE_MAP(CPierDlg, CPropertySheet)
END_MESSAGE_MAP()

// CPierDlg message handlers

bool RemoveTopBars(xbrLongitudinalRebarData::RebarRow& row)
{
   return row.Datum == xbrTypes::Top ? true : false;
}

void CPierDlg::VerifyLongitudinalReinforcement()
{
   // make sure the reinforcement in the pier data is correct
   // If the pier type was changed from Integral to Expansion or Continuous
   // the Top cross beam bars are not valid and should be removed
   if ( m_PierData.m_PierData.GetPierType() != xbrTypes::pctIntegral )
   {
      xbrLongitudinalRebarData& rebar( m_PierData.m_PierData.GetLongitudinalRebar() );
      rebar.RebarRows.erase(std::remove_if(rebar.RebarRows.begin(),rebar.RebarRows.end(),RemoveTopBars),rebar.RebarRows.end());
   }
}
