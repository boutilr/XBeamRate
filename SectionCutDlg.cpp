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

// SectionCutDlg.cpp : implementation file
//

#include "stdafx.h"
#include <AgentTools.h>
#include "resource.h"
#include "SectionCutDlg.h"
#include <ostream>

#include <EAF\EAFDisplayUnits.h>
#include <EAF\EAFDocument.h>

#include <IFace\PointOfInterest.h>
#include <IFace\Pier.h>

#include <MFCTools\Format.h>


/////////////////////////////////////////////////////////////////////////////
// CSectionCutDlg dialog

CSectionCutDlg::CSectionCutDlg(PierIDType pierID,const xbrPointOfInterest& initialPoi,CWnd* pParent) 
: CDialog(CSectionCutDlg::IDD, pParent),
m_PierID(pierID),
m_SliderPos(0)
{
   m_InitialPOI = initialPoi;
}

void CSectionCutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSectionCutDlgEx) 
	//}}AFX_DATA_MAP

   DDX_Control(pDX, IDC_SLIDER, m_Slider);
   DDX_Control(pDX, IDC_LOCATION, m_Label);
   DDX_Slider(pDX, IDC_SLIDER, m_SliderPos);
}

BEGIN_MESSAGE_MAP(CSectionCutDlg, CDialog)
	//{{AFX_MSG_MAP(CSectionCutDlg)
   ON_WM_HSCROLL()
	ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSectionCutDlg message handlers

BOOL CSectionCutDlg::OnInitDialog() 
{
   UpdatePOI();

   CDialog::OnInitDialog();

   // initial the slider range
   m_Slider.SetRange(0,(int)(m_vPOI.size()-1)); // the range is number of spaces along slider... 

   // initial the slider position to the current poi location
   IndexType pos = m_vPOI.size()/2; // default is mid-span
   std::vector<xbrPointOfInterest>::iterator iter;
   for ( iter = m_vPOI.begin(); iter != m_vPOI.end(); iter++ )
   {
      xbrPointOfInterest& poi = *iter;
      if ( poi.GetID() == m_InitialPOI.GetID() )
      {
         pos = (iter - m_vPOI.begin());
      }
   }
   m_Slider.SetPos((int)pos);

   UpdateSliderLabel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSectionCutDlg::OnHelp() 
{
   EAFHelp( EAFGetDocument()->GetDocumentationSetName(), IDH_SECTION_CUT );
}

xbrPointOfInterest CSectionCutDlg::GetPOI()
{
   ASSERT((int)m_SliderPos < (int)m_vPOI.size());
   xbrPointOfInterest poi = m_vPOI[m_SliderPos];
   return poi;
}

void CSectionCutDlg::UpdatePOI()
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRPointOfInterest,pPOI);
   m_vPOI = pPOI->GetXBeamPointsOfInterest(m_PierID);
   if (m_Slider.GetSafeHwnd() != nullptr )
   {
      m_Slider.SetRange(0,(int)(m_vPOI.size()-1)); // the range is number of spaces along slider... 
                                                   // subtract one so we don't go past the end of the array
   }
}

void CSectionCutDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   UpdateSliderLabel();
   CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSectionCutDlg::UpdateSliderLabel()
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   GET_IFACE2(pBroker,IXBRPointOfInterest,pPoi);

   ASSERT((int)m_SliderPos < (int)m_vPOI.size());
   xbrPointOfInterest poi = m_vPOI[m_Slider.GetPos()];

   Float64 Xp = pPoi->ConvertPoiToPierCoordinate(m_PierID,poi);

   GET_IFACE2(pBroker,IXBRPier,pPier);
   Float64 Xxb = pPier->ConvertPierToCrossBeamCoordinate(m_PierID,Xp);
   CString strLabel;
   strLabel.Format(_T("Location from Left Side of Cross Beam, %s"),::FormatDimension(Xxb,pDisplayUnits->GetXSectionDimUnit()));

   m_Label.SetWindowText(strLabel);
}
