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

// BearingsPage.cpp : implementation file
//

#include "stdafx.h"
#include "ProjectAgent.h"
#include "resource.h"
#include "BearingsPage.h"
#include "PierDlg.h"

#include <EAF\EAFDisplayUnits.h>
#include <EAF\EAFDocument.h>
#include <MFCTools\CustomDDX.h>

#include "..\Documentation\XBRate.hh"

#pragma warning(disable:4244)



void DDX_BearingGrid(CDataExchange* pDX,CBearingLayoutGrid& grid,xbrBearingLineData& brgLineData,std::vector<txnDeadLoadReaction>& deadLoadReactions)
{
   if ( pDX->m_bSaveAndValidate )
   {
      grid.GetBearingData(brgLineData,deadLoadReactions);
   }
   else
   {
      grid.SetBearingData(brgLineData,deadLoadReactions);
   }
}


// CBearingsPage dialog

IMPLEMENT_DYNAMIC(CBearingsPage, CPropertyPage)

CBearingsPage::CBearingsPage()
	: CPropertyPage(CBearingsPage::IDD)
{

}

CBearingsPage::~CBearingsPage()
{
}

void CBearingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

    auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_Text(pDX,IDC_DC_STRENGTH_I,pParent->m_PierData.m_gDC_StrengthI);
   DDX_Text(pDX,IDC_DW_STRENGTH_I,pParent->m_PierData.m_gDW_StrengthI);
   DDX_Text(pDX,IDC_CR_STRENGTH_I,pParent->m_PierData.m_gCR_StrengthI);
   DDX_Text(pDX,IDC_SH_STRENGTH_I,pParent->m_PierData.m_gSH_StrengthI);
   DDX_Text(pDX,IDC_PS_STRENGTH_I,pParent->m_PierData.m_gPS_StrengthI);

   DDX_Text(pDX,IDC_DC_STRENGTH_II,pParent->m_PierData.m_gDC_StrengthII);
   DDX_Text(pDX,IDC_DW_STRENGTH_II,pParent->m_PierData.m_gDW_StrengthII);
   DDX_Text(pDX,IDC_CR_STRENGTH_II,pParent->m_PierData.m_gCR_StrengthII);
   DDX_Text(pDX,IDC_SH_STRENGTH_II,pParent->m_PierData.m_gSH_StrengthII);
   DDX_Text(pDX,IDC_PS_STRENGTH_II,pParent->m_PierData.m_gPS_StrengthII);

   DDX_Text(pDX,IDC_DC_SERVICE_I,pParent->m_PierData.m_gDC_ServiceI);
   DDX_Text(pDX,IDC_DW_SERVICE_I,pParent->m_PierData.m_gDW_ServiceI);
   DDX_Text(pDX,IDC_CR_SERVICE_I,pParent->m_PierData.m_gCR_ServiceI);
   DDX_Text(pDX,IDC_SH_SERVICE_I,pParent->m_PierData.m_gSH_ServiceI);
   DDX_Text(pDX,IDC_PS_SERVICE_I,pParent->m_PierData.m_gPS_ServiceI);

   IndexType nBearingLines = pParent->m_PierData.m_PierData.GetBearingLineCount();
   DDX_CBItemData(pDX,IDC_BEARING_LINE_COUNT,nBearingLines);

   // Initialize both sets of controls, including the hidden ahead line. Use a
   // local default for a missing line so opening the page does not add a line.
   const IndexType nExchangeLines = pDX->m_bSaveAndValidate ? nBearingLines : 2;
   for ( IndexType brgLineIdx = 0; brgLineIdx < nExchangeLines; brgLineIdx++ )
   {
      xbrBearingLineData brgLineData;
      if ( brgLineIdx < pParent->m_PierData.m_PierData.GetBearingLineCount() )
      {
         brgLineData = pParent->m_PierData.m_PierData.GetBearingLineData(brgLineIdx);
      }

      // Display back-line offsets as positive without modifying the model
      // during initialization or leaving its sign changed if validation fails.
      if ( !pDX->m_bSaveAndValidate && brgLineIdx == 0 )
      {
         brgLineData.GetBearingLineOffset() *= -1;
      }

      DDX_CBEnum(pDX,IDC_BACK_REACTION_TYPE+brgLineIdx,pParent->m_PierData.m_DeadLoadReactionType[brgLineIdx]);
      DDX_BearingGrid(pDX,m_Grid[brgLineIdx],brgLineData,pParent->m_PierData.m_DeadLoadReactions[brgLineIdx]);
      DDX_CBIndex(pDX,IDC_BACK_REF_BEARING_LIST+brgLineIdx,brgLineData.GetRefBearingIndex());
      DDX_OffsetAndTag(pDX,IDC_BACK_REF_BEARING_LOCATION+brgLineIdx,IDC_BACK_REF_BEARING_LOCATION_UNIT+brgLineIdx,brgLineData.GetRefBearingOffset(),pDisplayUnits->GetSpanLengthUnit());
      DDX_CBEnum(pDX,IDC_BACK_REF_BEARING_DATUM+brgLineIdx,brgLineData.GetRefBearingDatum());
      DDX_UnitValueAndTag(pDX,IDC_BACK_BRG_OFFSET+brgLineIdx,IDC_BACK_BRG_OFFSET_UNIT+brgLineIdx,brgLineData.GetBearingLineOffset(),pDisplayUnits->GetComponentDimUnit());

      if ( pDX->m_bSaveAndValidate )
      {
         if ( brgLineIdx == 0 )
         {
            brgLineData.GetBearingLineOffset() *= -1;
         }
         pParent->m_PierData.m_PierData.SetBearingLineData(brgLineIdx,brgLineData);
      }
   }

   if ( pDX->m_bSaveAndValidate )
   {
      // Persist the selected count; in particular, remove a former ahead line.
      pParent->m_PierData.m_PierData.SetBearingLineCount(nBearingLines);
   }

   // make sure the unit labels are set for both bearing lines (one bearing line could be missed in the loop above)
   DDX_Tag(pDX,IDC_BACK_REF_BEARING_LOCATION_UNIT,pDisplayUnits->GetSpanLengthUnit());
   DDX_Tag(pDX,IDC_AHEAD_REF_BEARING_LOCATION_UNIT,pDisplayUnits->GetSpanLengthUnit());
   DDX_Tag(pDX,IDC_AHEAD_BRG_OFFSET_UNIT,pDisplayUnits->GetComponentDimUnit());
}


BEGIN_MESSAGE_MAP(CBearingsPage, CPropertyPage)
	ON_COMMAND(ID_HELP, OnHelp)
   ON_COMMAND(IDC_ADD_BACK, OnAddBack)
   ON_COMMAND(IDC_REMOVE_BACK, OnRemoveBack)
   ON_COMMAND(IDC_ADD_AHEAD, OnAddAhead)
   ON_COMMAND(IDC_REMOVE_AHEAD, OnRemoveAhead)
   ON_COMMAND(IDC_COPY_AHEAD, OnCopyAhead)
   ON_COMMAND(IDC_COPY_BACK, OnCopyBack)
   ON_CBN_SELCHANGE(IDC_BEARING_LINE_COUNT, OnBearingLineCountChanged)
   ON_CBN_SELCHANGE(IDC_BACK_REACTION_TYPE, OnBackReactionTypeChanged)
   ON_CBN_SELCHANGE(IDC_AHEAD_REACTION_TYPE, OnAheadReactionTypeChanged)
END_MESSAGE_MAP()


// CBearingsPage message handlers
BOOL CBearingsPage::OnInitDialog()
{
   CPierDlg* pParent = (CPierDlg*)GetParent();

   m_Grid[0].SubclassDlgItem(IDC_BACK_BEARING_GRID, this);
   m_Grid[0].CustomInit(pParent->m_PierData.m_DeadLoadReactionType[0]);

   m_Grid[1].SubclassDlgItem(IDC_AHEAD_BEARING_GRID, this);
   m_Grid[1].CustomInit(pParent->m_PierData.m_DeadLoadReactionType[1]);

   CComboBox* pcb = (CComboBox*)GetDlgItem(IDC_BEARING_LINE_COUNT);
   for ( IndexType brgLineIdx = 0; brgLineIdx < 2; brgLineIdx++ )
   {
      CString str;
      str.Format(_T("%d"),brgLineIdx+1);
      int idx = pcb->AddString(str);
      pcb->SetItemData(idx,(DWORD_PTR)(brgLineIdx+1));
   }

   FillRefBearingComboBox(0,true);
   FillRefBearingComboBox(1,true);

   FillRefBearingDatumComboBox(0);
   FillRefBearingDatumComboBox(1);

   FillReactionTypeComboBox(0);
   FillReactionTypeComboBox(1);

   CPropertyPage::OnInitDialog();

   OnBearingLineCountChanged();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CBearingsPage::OnAddBack()
{
   m_Grid[0].AddBearing();
   FillRefBearingComboBox(0);
}

void CBearingsPage::OnRemoveBack()
{
   m_Grid[0].RemoveSelectedBearings();
   FillRefBearingComboBox(0);
}

void CBearingsPage::OnAddAhead()
{
   m_Grid[1].AddBearing();
   FillRefBearingComboBox(1);
}

void CBearingsPage::OnRemoveAhead()
{
   m_Grid[1].RemoveSelectedBearings();
   FillRefBearingComboBox(1);
}

void CBearingsPage::OnCopyAhead()
{
   xbrBearingLineData brgLineData;
   std::vector<txnDeadLoadReaction> deadLoadReactions;
   m_Grid[0].GetBearingData(brgLineData,deadLoadReactions);
   m_Grid[1].SetBearingData(brgLineData,deadLoadReactions);
   FillRefBearingComboBox(1);

   CComboBox* pcbRefBearing = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_LIST);
   int curSel = pcbRefBearing->GetCurSel();
   pcbRefBearing = (CComboBox*)GetDlgItem(IDC_AHEAD_REF_BEARING_LIST);
   pcbRefBearing->SetCurSel(curSel);

   CString str;
   GetDlgItem(IDC_BACK_REF_BEARING_LOCATION)->GetWindowText(str);
   GetDlgItem(IDC_AHEAD_REF_BEARING_LOCATION)->SetWindowText(str);

   CComboBox* pcbDatum = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_DATUM);
   curSel = pcbDatum->GetCurSel();
   pcbDatum = (CComboBox*)GetDlgItem(IDC_AHEAD_REF_BEARING_DATUM);
   pcbDatum->SetCurSel(curSel);

   CComboBox* pcbReaction = (CComboBox*)GetDlgItem(IDC_BACK_REACTION_TYPE);
   curSel = pcbReaction->GetCurSel();
   pcbReaction = (CComboBox*)GetDlgItem(IDC_AHEAD_REACTION_TYPE);
   pcbReaction->SetCurSel(curSel);
   OnAheadReactionTypeChanged();

   GetDlgItem(IDC_BACK_BRG_OFFSET)->GetWindowText(str);
   GetDlgItem(IDC_AHEAD_BRG_OFFSET)->SetWindowText(str);
}

void CBearingsPage::OnCopyBack()
{
   xbrBearingLineData brgLineData;
   std::vector<txnDeadLoadReaction> deadLoadReactions;
   m_Grid[1].GetBearingData(brgLineData,deadLoadReactions);
   m_Grid[0].SetBearingData(brgLineData,deadLoadReactions);
   FillRefBearingComboBox(0);

   CComboBox* pcbRefBearing = (CComboBox*)GetDlgItem(IDC_AHEAD_REF_BEARING_LIST);
   int curSel = pcbRefBearing->GetCurSel();
   pcbRefBearing = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_LIST);
   pcbRefBearing->SetCurSel(curSel);

   CString str;
   GetDlgItem(IDC_AHEAD_REF_BEARING_LOCATION)->GetWindowText(str);
   GetDlgItem(IDC_BACK_REF_BEARING_LOCATION)->SetWindowText(str);

   CComboBox* pcbDatum = (CComboBox*)GetDlgItem(IDC_AHEAD_REF_BEARING_DATUM);
   curSel = pcbDatum->GetCurSel();
   pcbDatum = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_DATUM);
   pcbDatum->SetCurSel(curSel);

   CComboBox* pcbReaction = (CComboBox*)GetDlgItem(IDC_AHEAD_REACTION_TYPE);
   curSel = pcbReaction->GetCurSel();
   pcbReaction = (CComboBox*)GetDlgItem(IDC_BACK_REACTION_TYPE);
   pcbReaction->SetCurSel(curSel);
   OnBackReactionTypeChanged();

   GetDlgItem(IDC_AHEAD_BRG_OFFSET)->GetWindowText(str);
   GetDlgItem(IDC_BACK_BRG_OFFSET)->SetWindowText(str);
}

void CBearingsPage::OnBearingLineCountChanged()
{
   CComboBox* pCB = (CComboBox*)GetDlgItem(IDC_BEARING_LINE_COUNT);
   int curSel = pCB->GetCurSel();
   int show = (curSel == 0 ? SW_HIDE : SW_SHOW);

   GetDlgItem(IDC_BACK_LABEL)->SetWindowText(curSel == 0 ? _T("Along CL Pier") : _T("Along Back Bearing Line"));

   m_Grid[1].ShowWindow(show);
   GetDlgItem(IDC_COPY_AHEAD)->ShowWindow(show);
   GetDlgItem(IDC_COPY_BACK)->ShowWindow(show);
   GetDlgItem(IDC_BACK_BRG_OFFSET_LABEL1)->ShowWindow(show);
   GetDlgItem(IDC_BACK_BRG_OFFSET_LABEL2)->ShowWindow(show);
   GetDlgItem(IDC_BACK_BRG_OFFSET)->ShowWindow(show);
   GetDlgItem(IDC_BACK_BRG_OFFSET_UNIT)->ShowWindow(show);

   GetDlgItem(IDC_ADD_AHEAD)->ShowWindow(show);
   GetDlgItem(IDC_REMOVE_AHEAD)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_LABEL)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REACTION_TYPE_LABEL)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REACTION_TYPE)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REF_BEARING_LIST)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REF_BEARING_LOCATION)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REF_BEARING_LOCATION_UNIT)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_REF_BEARING_DATUM)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BEARING_LABEL)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BEARING_LABEL1)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BEARING_LABEL2)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BEARING_LABEL3)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BRG_OFFSET_LABEL1)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BRG_OFFSET_LABEL2)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BRG_OFFSET)->ShowWindow(show);
   GetDlgItem(IDC_AHEAD_BRG_OFFSET_UNIT)->ShowWindow(show);
}

void CBearingsPage::OnBackReactionTypeChanged()
{
   CComboBox* pcbReactionType = (CComboBox*)GetDlgItem(IDC_BACK_REACTION_TYPE);
   int curSel = pcbReactionType->GetCurSel();
   xbrTypes::ReactionLoadType reactionLoadType = (xbrTypes::ReactionLoadType)(pcbReactionType->GetItemData(curSel));
   m_Grid[0].SetReactionLoadType(reactionLoadType);
}

void CBearingsPage::OnAheadReactionTypeChanged()
{
   CComboBox* pcbReactionType = (CComboBox*)GetDlgItem(IDC_AHEAD_REACTION_TYPE);
   int curSel = pcbReactionType->GetCurSel();
   xbrTypes::ReactionLoadType reactionLoadType = (xbrTypes::ReactionLoadType)(pcbReactionType->GetItemData(curSel));
   m_Grid[1].SetReactionLoadType(reactionLoadType);
}

void CBearingsPage::FillRefBearingComboBox(IndexType brgLineIdx,bool bInitialFill)
{
   CComboBox* pcbRefBearing = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_LIST+brgLineIdx);
   int curSel = pcbRefBearing->GetCurSel();
   pcbRefBearing->ResetContent();

   CString strLabel;
   IndexType nBearings;
   if ( bInitialFill )
   {
      CPierDlg* pParent = (CPierDlg*)GetParent();
      if ( brgLineIdx < pParent->m_PierData.m_PierData.GetBearingLineCount() )
      {
         nBearings = pParent->m_PierData.m_PierData.GetBearingCount(brgLineIdx);
      }
      else
      {
         nBearings = 1;
      }
   }
   else
   {
      nBearings = m_Grid[brgLineIdx].GetBearingCount();
   }

   nBearings = Max((IndexType)1,nBearings); // there is always one bearing minimum

   for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
   {
      strLabel.Format(_T("%d"),(brgIdx+1));
      pcbRefBearing->AddString(strLabel);
   }

   if ( pcbRefBearing->SetCurSel(curSel) == CB_ERR )
   {
      pcbRefBearing->SetCurSel(0);
   }
}

void CBearingsPage::FillRefBearingDatumComboBox(IndexType brgLineIdx)
{
   // do we need two datums, one for each bearing line, or is one good enough?
   CComboBox* pcbRefBearingDatum = (CComboBox*)GetDlgItem(IDC_BACK_REF_BEARING_DATUM+brgLineIdx);
   int curSel = pcbRefBearingDatum->GetCurSel();
   pcbRefBearingDatum->ResetContent();

   int idx = pcbRefBearingDatum->AddString(_T("Alignment"));
   pcbRefBearingDatum->SetItemData(idx,(DWORD_PTR)pgsTypes::omtAlignment);

   idx = pcbRefBearingDatum->AddString(_T("Bridge Line"));
   pcbRefBearingDatum->SetItemData(idx,(DWORD_PTR)pgsTypes::omtBridge);

   if ( pcbRefBearingDatum->SetCurSel(curSel) == CB_ERR )
   {
      pcbRefBearingDatum->SetCurSel(0);
   }
}

void CBearingsPage::FillReactionTypeComboBox(IndexType brgLineIdx)
{
   CComboBox* pcbReactionType = (CComboBox*)GetDlgItem(IDC_BACK_REACTION_TYPE+brgLineIdx);
   int curSel = pcbReactionType->GetCurSel();
   pcbReactionType->ResetContent();

   int idx = pcbReactionType->AddString(_T("Concentrated"));
   pcbReactionType->SetItemData(idx,(DWORD_PTR)xbrTypes::rltConcentrated);

   idx = pcbReactionType->AddString(_T("Uniform of width, W"));
   pcbReactionType->SetItemData(idx,(DWORD_PTR)xbrTypes::rltUniform);

   if ( pcbReactionType->SetCurSel(curSel) == CB_ERR )
   {
      pcbReactionType->SetCurSel(0);
   }
}

void CBearingsPage::OnHelp()
{
   EAFHelp(EAFGetDocument()->GetDocumentationSetName(), IDH_PERMANENT_LOAD);
}
