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
#include "GraphingAgent.h"
#include "resource.h"
#include "AnalysisResultsGraphController.h"
#include "AnalysisResultsGraphBuilder.h"

#include <IFace\XBeamRateAgent.h>
#include <..\..\PGSuper\Include\IFace\Project.h>

#include <PsgLib\GirderLabel.h>
#include <PsgLib\BridgeDescription2.h>
#include <EAF\EAFUtilities.h>
#include <XBeamRateExt\XBeamRateUtilities.h>
#include <Graphs\ExportGraphXYTool.h>


static LPCTSTR GetActionName(ActionType action)
{
   switch (action)
   {
   case actionShear:
      return _T("Shear");
   case actionMoment:
      return _T("Moment");
   case actionLoadRating:
      return _T("Rating Factor");
   default:
      ATLASSERT(false); // is there a new action type?
      return _T("Unknown Action");
   };
}


IMPLEMENT_DYNCREATE(CXBRAnalysisResultsGraphController,CEAFGraphControlWindow)

CXBRAnalysisResultsGraphController::CXBRAnalysisResultsGraphController()
{
}

BEGIN_MESSAGE_MAP(CXBRAnalysisResultsGraphController, CEAFGraphControlWindow)
	//{{AFX_MSG_MAP(CXBRAnalysisResultsGraphController)
   ON_BN_CLICKED(IDC_EXPORT_GRAPH_BTN,OnGraphExportClicked)
   ON_UPDATE_COMMAND_UI(IDC_EXPORT_GRAPH_BTN,OnCommandUIGraphExport)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CXBRAnalysisResultsGraphController::OnInitDialog()
{
   FillPierList();
   FillLoadingList();
   
   CEAFGraphControlWindow::OnInitDialog();
   CheckRadioButton(IDC_MOMENT,IDC_SHEAR,IDC_MOMENT);

   return TRUE;
}

#ifdef _DEBUG
void CXBRAnalysisResultsGraphController::AssertValid() const
{
	CEAFGraphControlWindow::AssertValid();
}

void CXBRAnalysisResultsGraphController::Dump(CDumpContext& dc) const
{
	CEAFGraphControlWindow::Dump(dc);
}
#endif //_DEBUG

CXBRAnalysisResultsGraphDefinitions CXBRAnalysisResultsGraphController::GetSelectedGraphDefinitions()
{
   CXBRAnalysisResultsGraphBuilder* pGraphBuilder = (CXBRAnalysisResultsGraphBuilder*)GetGraphBuilder();
   const CXBRAnalysisResultsGraphDefinitions& graphDefinitions = pGraphBuilder->GetGraphDefinitions();

   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   int nSel = plbLoading->GetSelCount();
   CArray<int,int> selectedItems;
   selectedItems.SetSize(nSel);
   plbLoading->GetSelItems(nSel,selectedItems.GetData());

   CXBRAnalysisResultsGraphDefinitions selectedGraphDefinitions;
   for ( int i = 0; i < nSel; i++ )
   {
      int idx = selectedItems.GetAt(i);
      IDType graphID = (IDType)plbLoading->GetItemData(idx);
      const CXBRAnalysisResultsGraphDefinition& graphDef(graphDefinitions.FindGraphDefinition(graphID));
      selectedGraphDefinitions.AddGraphDefinition(graphDef);
   }

   return selectedGraphDefinitions;
}

ActionType CXBRAnalysisResultsGraphController::GetActionType()
{
#pragma Reminder("UPDATE: need to add load rating graph action")
   int graphType = GetCheckedRadioButton(IDC_MOMENT,IDC_SHEAR);
   if(  graphType == IDC_MOMENT )
   {
      return actionMoment;
   }
   else
   {
      return actionShear;
   }
}

PierIDType CXBRAnalysisResultsGraphController::GetPierID()
{
   auto pierIdx = GetPierIndex();
   if (pierIdx == INVALID_INDEX)
      return INVALID_ID;

   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker, IBridgeDescription, pIBridgeDesc);
   const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
   const CPierData2* pPierData = pBridgeDesc->GetPier(pierIdx);
   return pPierData->GetID();
}

PierIndexType CXBRAnalysisResultsGraphController::GetPierIndex()
{
   CComboBox* pcbPier = (CComboBox*)GetDlgItem(IDC_PIERS);
   int curSel = pcbPier->GetCurSel();
   PierIndexType pierIdx = (PierIndexType)(pcbPier->GetItemData(curSel));
   return pierIdx;
}

BOOL CALLBACK EnableChildWindow(HWND hwnd,LPARAM lParam)
{
   ::EnableWindow(hwnd,(int)lParam);
   return TRUE;
}

void CXBRAnalysisResultsGraphController::EnableControls(BOOL bEnable)
{
   EnumChildWindows(GetSafeHwnd(),EnableChildWindow,bEnable);
}

void CXBRAnalysisResultsGraphController::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   FillPierList();

   CXBRAnalysisResultsGraphBuilder* pGraphBuilder = (CXBRAnalysisResultsGraphBuilder*)GetGraphBuilder();
   pGraphBuilder->UpdateGraphDefinitions();
}

void CXBRAnalysisResultsGraphController::FillPierList()
{
   CComboBox* pcbPiers = (CComboBox*)GetDlgItem(IDC_PIERS);
   int curSel = pcbPiers->GetCurSel();
   pcbPiers->ResetContent();

   if ( IsStandAlone() )
   {
      int idx = pcbPiers->AddString(_T("Stand Alone Mode")); // put a dummy pier in the combo box
      pcbPiers->SetItemData(idx,(DWORD_PTR)INVALID_INDEX);
      // so when we get the pier index there is something to return
      pcbPiers->ShowWindow(SW_HIDE);
   }
   else
   {
      // We are extending PGSuper/PGSplice
      // Fill the combo box with piers
      auto pBroker = EAFGetBroker();


      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);
      const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
      PierIndexType nPiers = pBridgeDesc->GetPierCount();
      for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
      {
         // Can only load rate piers with a physical description
         const CPierData2* pPierData = pBridgeDesc->GetPier(pierIdx);
         if ( pPierData->GetPierModelType() == pgsTypes::pmtPhysical )
         {
            CString strPier;
            strPier.Format(_T("Pier %s"),LABEL_PIER(pierIdx));
            int idx = pcbPiers->AddString(strPier);
            pcbPiers->SetItemData(idx,(DWORD_PTR)pierIdx);
         }
      }
   }

   pcbPiers->SetCurSel(curSel == CB_ERR ? 0 : curSel);
}

void CXBRAnalysisResultsGraphController::FillLoadingList()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   int nSel = plbLoading->GetSelCount();
   CArray<int, int> selectedItems;
   selectedItems.SetSize(nSel);
   plbLoading->GetSelItems(nSel, selectedItems.GetData());

   plbLoading->ResetContent();

   CXBRAnalysisResultsGraphBuilder* pGraphBuilder = (CXBRAnalysisResultsGraphBuilder*)GetGraphBuilder();
   const CXBRAnalysisResultsGraphDefinitions& graphDefinitions = pGraphBuilder->GetGraphDefinitions();
   IndexType nGraphs = graphDefinitions.GetGraphDefinitionCount();
   for ( IndexType graphIdx = 0; graphIdx < nGraphs; graphIdx++ )
   {
      const CXBRAnalysisResultsGraphDefinition& graphDefinition(graphDefinitions.GetGraphDefinition(graphIdx));
      
      int idx = plbLoading->AddString(graphDefinition.m_Name.c_str());
      plbLoading->SetItemData(idx,graphDefinition.m_ID);
   }

   for (int i = 0; i < nSel; i++)
   {
      int idx = selectedItems[i];
      plbLoading->SetSel(idx);
   }
}

void CXBRAnalysisResultsGraphController::OnGraphExportClicked()
{
   // Build default file name
   CString strProjectFileNameNoPath = CExportGraphXYTool::GetTruncatedFileName();

   auto pierIdx = GetPierIndex();
   
   ActionType action = GetActionType();
   CString actionName = GetActionName(action);

   CString strDefaultFileName = strProjectFileNameNoPath;
   
   if (pierIdx != INVALID_INDEX)
   {
      CString pierName = _T("Pier_") + CString(LABEL_PIER(pierIdx));
      strDefaultFileName += _T("_") + pierName;
   }

   strDefaultFileName += _T("_") + actionName;

   strDefaultFileName.Replace(' ','_'); // prefer not to have spaces or ,'s in file names
   strDefaultFileName.Replace(',','_');

   ((CXBRAnalysisResultsGraphBuilder*)GetGraphBuilder())->ExportGraphData(strDefaultFileName);
}

// this has to be implemented otherwise button will not be enabled.
void CXBRAnalysisResultsGraphController::OnCommandUIGraphExport(CCmdUI* pCmdUI)
{
   pCmdUI->Enable(TRUE);
}

