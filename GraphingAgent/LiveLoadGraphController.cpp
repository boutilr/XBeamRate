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
#include "LiveLoadGraphController.h"
#include "LiveLoadGraphBuilder.h"

#include <IFace\XBeamRateAgent.h>
#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>
#include <..\..\PGSuper\Include\IFace\Project.h>

#include <PsgLib\GirderLabel.h>
#include <PsgLib\BridgeDescription2.h>
#include <PsgLib\Helpers.h>

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

IMPLEMENT_DYNCREATE(CXBRLiveLoadGraphController,CEAFGraphControlWindow)

CXBRLiveLoadGraphController::CXBRLiveLoadGraphController()
{
}

BEGIN_MESSAGE_MAP(CXBRLiveLoadGraphController, CEAFGraphControlWindow)
	//{{AFX_MSG_MAP(CXBRLiveLoadGraphController)
   ON_WM_VSCROLL()
   ON_BN_CLICKED(IDC_EXPORT_GRAPH_BTN,OnGraphExportClicked)
   ON_UPDATE_COMMAND_UI(IDC_EXPORT_GRAPH_BTN,OnCommandUIGraphExport)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CXBRLiveLoadGraphController::OnInitDialog()
{
   FillPierList();
   FillRatingType();
   FillVehicleType();
   FillLoadingList();
     
   CEAFGraphControlWindow::OnInitDialog();
   CheckRadioButton(IDC_MOMENT,IDC_SHEAR,IDC_MOMENT);

   return TRUE;
}

void CXBRLiveLoadGraphController::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   UpdatePermitLaneLabel();
   CXBRLiveLoadGraphBuilder* pGraphBuilder = (CXBRLiveLoadGraphBuilder*)GetGraphBuilder();
   pGraphBuilder->Update();
}

#ifdef _DEBUG
void CXBRLiveLoadGraphController::AssertValid() const
{
	CEAFGraphControlWindow::AssertValid();
}

void CXBRLiveLoadGraphController::Dump(CDumpContext& dc) const
{
	CEAFGraphControlWindow::Dump(dc);
}
#endif //_DEBUG

void CXBRLiveLoadGraphController::UpdatePermitLaneLabel()
{
   pgsTypes::LoadRatingType ratingType = GetLoadRatingType();
   bool bIsEmergencyRatingType = ::IsEmergencyRatingType(ratingType);
   bool bIsPermitRatingType = ::IsPermitRatingType(ratingType);
   ATLASSERT(bIsEmergencyRatingType || bIsPermitRatingType);

   IndexType permitLaneIdx = GetPermitLaneIndex();
   CString strLabel;
   strLabel.Format(_T("%s vehicle in Lane %d"), bIsPermitRatingType ? _T("Permit") : _T("Emergency"), (permitLaneIdx+1));
   GetDlgItem(IDC_PERMIT_LANE_NOTE)->SetWindowText(strLabel);
}

IndexType CXBRLiveLoadGraphController::GetSelectedLiveLoadConfiguration()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   int curSel = plbLoading->GetCurSel();
   if (curSel == LB_ERR )
   {
      return INVALID_INDEX;
   }

   IndexType llConfigIdx = (IndexType)plbLoading->GetItemData(curSel);
   return llConfigIdx;
}

ActionType CXBRLiveLoadGraphController::GetActionType()
{
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

PierIDType CXBRLiveLoadGraphController::GetPierID()
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

PierIndexType CXBRLiveLoadGraphController::GetPierIndex()
{
   CComboBox* pcbPier = (CComboBox*)GetDlgItem(IDC_PIERS);
   int curSel = pcbPier->GetCurSel();
   PierIndexType pierIdx = (PierIndexType)(pcbPier->GetItemData(curSel));
   return pierIdx;
}

pgsTypes::LoadRatingType CXBRLiveLoadGraphController::GetLoadRatingType()
{
   CComboBox* pcbRatingType = (CComboBox*)GetDlgItem(IDC_RATING_TYPE);
   int curSel = pcbRatingType->GetCurSel();
   pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)(pcbRatingType->GetItemData(curSel));
   return ratingType;
}

VehicleIndexType CXBRLiveLoadGraphController::GetVehicleIndex()
{
   CComboBox* pcbVehicle = (CComboBox*)GetDlgItem(IDC_VEHICLE);
   int curSel = pcbVehicle->GetCurSel();
   VehicleIndexType vehicleIdx = (VehicleIndexType)(pcbVehicle->GetItemData(curSel));
   return vehicleIdx;
}

IndexType CXBRLiveLoadGraphController::GetPermitLaneIndex()
{
   pgsTypes::LoadRatingType ratingType = GetLoadRatingType();

   auto pBroker = EAFGetBroker();

   GET_IFACE2_NOCHECK(pBroker,IXBRRatingSpecification,pRatingSpec);
   
   if (pRatingSpec->IsWSDOTEmergencyRating(ratingType) || pRatingSpec->IsWSDOTPermitRating(ratingType))
   {
      CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_PERMIT_LANE_SPINNER);
      int curPos = pSpin->GetPos32();
      IndexType permitLaneIdx = (IndexType)(curPos-1);
      return permitLaneIdx;
   }
   else
   {
      return INVALID_INDEX;
   }
}

BOOL CALLBACK EnableMyChildWindow(HWND hwnd,LPARAM lParam)
{
   ::EnableWindow(hwnd,(int)lParam);
   return TRUE;
}

void CXBRLiveLoadGraphController::EnableControls(BOOL bEnable)
{
   EnumChildWindows(GetSafeHwnd(),EnableMyChildWindow,bEnable);
}

void CXBRLiveLoadGraphController::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
   FillPierList();
   FillRatingType();
   FillVehicleType();
   FillLoadingList();
}

void CXBRLiveLoadGraphController::FillPierList()
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

void CXBRLiveLoadGraphController::FillRatingType()
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRProject,pProject);

   PierIDType pierID = GetPierID();

   CComboBox* pcbRatingType = (CComboBox*)GetDlgItem(IDC_RATING_TYPE);
   int curSel = pcbRatingType->GetCurSel();
   pcbRatingType->ResetContent();
   int n = (int)pgsTypes::lrLoadRatingTypeCount;
   for ( int i = 0; i < n; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;

      std::_tstring strLiveLoad = pProject->GetLiveLoadName(pierID,ratingType,0);
      if ( strLiveLoad != NO_LIVE_LOAD_DEFINED )
      {
         int idx = pcbRatingType->AddString(GetLiveLoadTypeName(ratingType));
         pcbRatingType->SetItemData(idx,(DWORD_PTR)ratingType);
      }
   }

   curSel = pcbRatingType->SetCurSel(curSel);
   if ( curSel == CB_ERR )
   {
      pcbRatingType->SetCurSel(0);
   }
}

void CXBRLiveLoadGraphController::FillVehicleType()
{
   CComboBox* pcbVehicle = (CComboBox*)GetDlgItem(IDC_VEHICLE);
   int curSel = pcbVehicle->GetCurSel();

   pcbVehicle->ResetContent();

   PierIDType pierID = GetPierID();
   pgsTypes::LoadRatingType ratingType = GetLoadRatingType();

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRProject,pProject);

   VehicleIndexType nLiveLoads = pProject->GetLiveLoadReactionCount(pierID,ratingType);
   for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nLiveLoads; vehicleIdx++ )
   {
      std::_tstring strLiveLoad = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);
      if ( strLiveLoad != NO_LIVE_LOAD_DEFINED )
      {
         int idx = pcbVehicle->AddString(strLiveLoad.c_str());
         pcbVehicle->SetItemData(idx,(DWORD_PTR)vehicleIdx);
      }
   }

   curSel = pcbVehicle->SetCurSel(curSel);
   if ( curSel == CB_ERR )
   {
      pcbVehicle->SetCurSel(0);
   }
}

void CXBRLiveLoadGraphController::Next()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   int curSel = plbLoading->GetCurSel();
   curSel++;
   curSel = plbLoading->SetCurSel(curSel);
   if ( curSel == LB_ERR )
   {
      plbLoading->SetCurSel(0);
   }
   LoadingChanged();
}

void CXBRLiveLoadGraphController::Prev()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   int curSel = plbLoading->GetCurSel();
   curSel--;
   curSel = plbLoading->SetCurSel(curSel);
   if ( curSel == LB_ERR )
   {
      plbLoading->SetCurSel(plbLoading->GetCount()-1);
   }
   LoadingChanged();
}

void CXBRLiveLoadGraphController::RatingTypeChanged()
{
   FillVehicleType();
   FillLoadingList();

   pgsTypes::LoadRatingType ratingType = GetLoadRatingType();
   auto pBroker = EAFGetBroker();

   GET_IFACE2_NOCHECK(pBroker,IXBRRatingSpecification,pRatingSpec);
   if (pRatingSpec->IsWSDOTEmergencyRating(ratingType) || pRatingSpec->IsWSDOTPermitRating(ratingType))
   {
      GetDlgItem(IDC_PERMIT_LANE_NOTE)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_PERMIT_LANE)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_PERMIT_LANE_SPINNER)->ShowWindow(SW_SHOW);

      UpdatePermitLaneLabel();
   }
   else
   {
      GetDlgItem(IDC_PERMIT_LANE_NOTE)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_PERMIT_LANE)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_PERMIT_LANE_SPINNER)->ShowWindow(SW_HIDE);
   }
}

void CXBRLiveLoadGraphController::LoadingChanged()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   GetDlgItem(IDC_PERMIT_LANE_NOTE)->EnableWindow(TRUE);
   GetDlgItem(IDC_PERMIT_LANE)->EnableWindow(TRUE);
   GetDlgItem(IDC_PERMIT_LANE_SPINNER)->EnableWindow(TRUE);

   int curSel = plbLoading->GetCurSel();
   if ( curSel == LB_ERR )
   {
      return;
   }
   IndexType llConfigIdx = (IndexType)plbLoading->GetItemData(curSel);

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRProductForces,pProductForces);

   PierIDType pierID = GetPierID();
   IndexType nLoadedLanes = pProductForces->GetLoadedLaneCount(pierID,llConfigIdx);

   CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_PERMIT_LANE_SPINNER);
   int curPos = pSpin->GetPos32();
   pSpin->SetRange32(1,(int)nLoadedLanes);

   if ( (IndexType)curPos <= nLoadedLanes )
   {
      pSpin->SetPos32(curPos);
   }
   else
   {
      pSpin->SetPos32(0);
   }
}

void CXBRLiveLoadGraphController::FillLoadingList()
{
   CListBox* plbLoading = (CListBox*)GetDlgItem(IDC_LOADING);
   plbLoading->ResetContent();

   PierIDType pierID = GetPierID();
   if ( !IsStandAlone() && pierID == INVALID_ID )
   {
      return;
   }

   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBRProductForces,pProductForces);

   pgsTypes::LoadRatingType ratingType = GetLoadRatingType();
   IndexType nLiveLoadConfigs = pProductForces->GetLiveLoadConfigurationCount(pierID,ratingType);
   for ( IndexType llConfigIdx = 0; llConfigIdx < nLiveLoadConfigs; llConfigIdx++ )
   {
      CString str;
      str.Format(_T("%d"),(llConfigIdx+1));
      int idx = plbLoading->AddString(str);
      plbLoading->SetItemData(idx,llConfigIdx);
   }
}

void CXBRLiveLoadGraphController::OnGraphExportClicked()
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

   strDefaultFileName.Replace(' ', '_'); // prefer not to have spaces or ,'s in file names
   strDefaultFileName.Replace(',', '_');

   ((CXBRLiveLoadGraphBuilder*)GetGraphBuilder())->ExportGraphData(strDefaultFileName);
}

// this has to be implemented otherwise button will not be enabled.
void CXBRLiveLoadGraphController::OnCommandUIGraphExport(CCmdUI* pCmdUI)
{
   pCmdUI->Enable(TRUE);
}
