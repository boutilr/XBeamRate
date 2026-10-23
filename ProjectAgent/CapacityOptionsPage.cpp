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
#include "resource.h"
#include "CapacityOptionsPage.h"
#include "OptionsDlg.h"

#include <EAF\EAFDocument.h>

#include <txnEditOptions.h>

#include "..\Documentation\XBRate.hh"



// CCapacityOptionsPage dialog

IMPLEMENT_DYNAMIC(CCapacityOptionsPage, CPropertyPage)

CCapacityOptionsPage::CCapacityOptionsPage()
	: CPropertyPage(CCapacityOptionsPage::IDD)
{

}

CCapacityOptionsPage::~CCapacityOptionsPage()
{
}

void CCapacityOptionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   COptionsDlg* pParent = (COptionsDlg*)GetParent();
   DDX_Text(pDX,IDC_PHI_COMPRESSION,pParent->m_Options.m_PhiC);
   DDX_Text(pDX,IDC_PHI_TENSION,pParent->m_Options.m_PhiT);
   DDX_Text(pDX,IDC_PHI_SHEAR,pParent->m_Options.m_PhiV);
}


BEGIN_MESSAGE_MAP(CCapacityOptionsPage, CPropertyPage)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CCapacityOptionsPage message handlers

void CCapacityOptionsPage::OnHelp() 
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_OPTIONS_CAPACITY);
}
