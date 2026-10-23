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

// ConcreteDetailsDlg.cpp : implementation file
//

///////////////////////////////////////////////////////////////////////////
// NOTE: Duplicate code warning
//
// This dialog along with all its property pages are basically repeated in
// the PGSuperLibrary project. I could not get a single implementation to
// work because of issues with the module resources.
//
// If changes are made here, the same changes are likely needed in
// the other location.

#include "stdafx.h"
#include "ProjectAgent.h"
#include "resource.h"
#include "ConcreteDetailsDlg.h"

#include <System\Tokenizer.h>
#include <LRFD\ConcreteUtil.h>
#include <EAF\EAFDisplayUnits.h>
#include <IFace\Project.h>


/////////////////////////////////////////////////////////////////////////////
// CConcreteDetailsDlg dialog


CConcreteDetailsDlg::CConcreteDetailsDlg(CWnd* pParent /*=nullptr*/,UINT iSelectPage/*=0*/)
	: CPropertySheet(_T("Concrete Details"),pParent, iSelectPage)
{
	//{{AFX_DATA_INIT(CConcreteDetailsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
   Init();
}

BEGIN_MESSAGE_MAP(CConcreteDetailsDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CConcreteDetailsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConcreteDetailsDlg message handlers

BOOL CConcreteDetailsDlg::OnInitDialog() 
{
	CPropertySheet::OnInitDialog();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConcreteDetailsDlg::Init()
{
   m_psh.dwFlags |= PSH_HASHELP | PSH_NOAPPLYNOW;

   m_General.m_psp.dwFlags  |= PSP_HASHELP;
   m_AASHTO.m_psp.dwFlags   |= PSP_HASHELP;

   AddPage( &m_General );
   AddPage( &m_AASHTO );
}

CString CConcreteDetailsDlg::UpdateEc(pgsTypes::ConcreteType type, const CString& strFc,const CString& strDensity,const CString& strK1,const CString& strK2)
{
   CString strEc;
   Float64 fc, density, k1,k2;
   Float64 ec = 0;
   if (WBFL::System::Tokenizer::ParseDouble(strFc, &fc) && 
       WBFL::System::Tokenizer::ParseDouble(strDensity,&density) &&
       WBFL::System::Tokenizer::ParseDouble(strK1,&k1) &&
       WBFL::System::Tokenizer::ParseDouble(strK2,&k2) &&
       0 <= density && 0 <= fc && 0 <= k1 && 0 <= k2
       )
   {
      auto pBroker = EAFGetBroker();

         GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

         const WBFL::Units::Pressure& stress_unit = pDisplayUnits->GetStressUnit().UnitOfMeasure;
         const WBFL::Units::Density& density_unit = pDisplayUnits->GetDensityUnit().UnitOfMeasure;

         fc       = WBFL::Units::ConvertToSysUnits(fc,      stress_unit);
         density  = WBFL::Units::ConvertToSysUnits(density, density_unit);

         ec = k1*k2*WBFL::LRFD::ConcreteUtil::ModE((WBFL::Materials::ConcreteType)type,fc,density,false);

         strEc.Format(_T("%s"),FormatDimension(ec,pDisplayUnits->GetModEUnit(),false));
   }

   return strEc;
}
