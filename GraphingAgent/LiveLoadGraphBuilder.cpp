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
#include "LiveLoadGraphBuilder.h"

#include <EAF\EAFGraphChildFrame.h>
#include <EAF\EAFGraphView.h>

#include <EAF\EAFUtilities.h>
#include <EAF\EAFDisplayUnits.h>
#include <EAF/AutoProgress.h>
#include <MathEx.h>
#include <Units\UnitValueNumericalFormatTools.h>

#include <IFace\XBeamRateAgent.h>
#include <IFace\Project.h>
#include <IFace\PointOfInterest.h>
#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>
#include <IFace\RatingSpecification.h>
#include <IFace\Pier.h>

#include <XBeamRateExt\XBeamRateUtilities.h>

#include <..\..\PGSuper\Include\IFace\Project.h>
#include <PsgLib\PierData2.h>
#include <PsgLib\Helpers.h>

#include <MFCTools\Format.h>
#include <MFCTools\Text.h>

#include <Graphs/ExportGraphXYTool.h>

#include <algorithm>


// create a dummy unit conversion tool to pacify the graph constructor
static WBFL::Units::LengthData DUMMY(WBFL::Units::Measure::Meter);
static WBFL::Units::LengthTool DUMMY_TOOL(DUMMY);

BEGIN_MESSAGE_MAP(CXBRLiveLoadGraphBuilder, CEAFAutoCalcGraphBuilder)
   ON_BN_CLICKED(IDC_MOMENT, &CXBRLiveLoadGraphBuilder::OnGraphTypeChanged)
   ON_BN_CLICKED(IDC_SHEAR, &CXBRLiveLoadGraphBuilder::OnGraphTypeChanged)
   ON_BN_CLICKED(IDC_NEXT, &CXBRLiveLoadGraphBuilder::OnNext)
   ON_BN_CLICKED(IDC_PREV, &CXBRLiveLoadGraphBuilder::OnPrev)
   ON_LBN_SELCHANGE(IDC_LOADING,&CXBRLiveLoadGraphBuilder::OnLbnSelChanged)
   ON_CBN_SELCHANGE(IDC_PIERS,&CXBRLiveLoadGraphBuilder::OnPierChanged)
   ON_CBN_SELCHANGE(IDC_RATING_TYPE,&CXBRLiveLoadGraphBuilder::OnRatingTypeChanged)
   ON_CBN_SELCHANGE(IDC_VEHICLE,&CXBRLiveLoadGraphBuilder::OnVehicleTypeChanged)
END_MESSAGE_MAP()


CXBRLiveLoadGraphBuilder::CXBRLiveLoadGraphBuilder() :
CEAFAutoCalcGraphBuilder(),
m_Graph(&DUMMY_TOOL, &DUMMY_TOOL),
m_pXFormat(nullptr),
m_pYFormat(nullptr)
{
   SetName(_T("Live Load Results"));
   InitGraph();
}

CXBRLiveLoadGraphBuilder::CXBRLiveLoadGraphBuilder(const CXBRLiveLoadGraphBuilder& other) :
CEAFAutoCalcGraphBuilder(other),
m_Graph(&DUMMY_TOOL, &DUMMY_TOOL),
m_pXFormat(nullptr),
m_pYFormat(nullptr)
{
   InitGraph();
}

CXBRLiveLoadGraphBuilder::~CXBRLiveLoadGraphBuilder()
{
   if ( m_pXFormat != nullptr )
   {
      delete m_pXFormat;
      m_pXFormat = nullptr;
   }

   if ( m_pYFormat != nullptr )
   {
      delete m_pYFormat;
      m_pYFormat = nullptr;
   }
}

CEAFGraphControlWindow* CXBRLiveLoadGraphBuilder::GetGraphControlWindow()
{
   return &m_GraphController;
}

std::unique_ptr<WBFL::Graphing::GraphBuilder> CXBRLiveLoadGraphBuilder::Clone() const
{
   // set the module state or the commands wont route to the
   // the graph control window
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return std::make_unique<CXBRLiveLoadGraphBuilder>(*this);
}

void CXBRLiveLoadGraphBuilder::CreateViewController(IEAFViewController** ppController)
{
   *ppController = nullptr;
}

BOOL CXBRLiveLoadGraphBuilder::CreateGraphController(CWnd* pParent,UINT nID)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   if ( !m_GraphController.Create(pParent,IDD_LIVE_LOAD_GRAPH_CONTROLLER, CBRS_LEFT, nID) )
   {
      TRACE0("Failed to create control bar\n");
      return FALSE; // failed to create
   }

   return TRUE;
}

void CXBRLiveLoadGraphBuilder::OnGraphTypeChanged()
{
   InvalidateGraph();
   Update();
}

void CXBRLiveLoadGraphBuilder::OnLbnSelChanged()
{
   m_GraphController.LoadingChanged();
   OnGraphTypeChanged();
}

void CXBRLiveLoadGraphBuilder::OnNext()
{
   m_GraphController.Next();
   OnGraphTypeChanged();
}

void CXBRLiveLoadGraphBuilder::OnPrev()
{
   m_GraphController.Prev();
   OnGraphTypeChanged();
}

void CXBRLiveLoadGraphBuilder::OnPierChanged()
{
   OnGraphTypeChanged();
}

void CXBRLiveLoadGraphBuilder::OnRatingTypeChanged()
{
   m_GraphController.RatingTypeChanged();
   OnGraphTypeChanged();
}

void CXBRLiveLoadGraphBuilder::OnVehicleTypeChanged()
{
   OnGraphTypeChanged();
}

bool CXBRLiveLoadGraphBuilder::UpdateNow()
{
   auto pBroker = EAFGetBroker();


   if ( IsPGSExtension() )
   {
      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);

      bool bCanUpdate = false;
      PierIndexType nPiers = pIBridgeDesc->GetPierCount();
      for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
      {
         const CPierData2* pPier = pIBridgeDesc->GetPier(pierIdx);
         if ( pPier->GetPierModelType() == pgsTypes::pmtPhysical )
         {
            m_GraphController.EnableControls(TRUE);
            bCanUpdate = true;
            break;
         }
      }

      if ( !bCanUpdate )
      {
         m_GraphController.EnableControls(FALSE);
         m_ErrorMsg = _T("Cross Beam results are not available.\nThe bridge model has only idealized piers.");
         return false;
      }
   }

   GET_IFACE2(pBroker,IEAFProgress,pProgress);
   WBFL::EAF::AutoProgress ap(pProgress,0);

   pProgress->UpdateMessage(_T("Building Graph"));

   CWaitCursor wait;

   // Update graph properties
   UpdateYAxisUnits();
   UpdateGraphTitle();
   UpdateGraphData();

   return true;
}

void CXBRLiveLoadGraphBuilder::InitGraph()
{
   m_Graph.SetGridPenStyle(GRAPH_GRID_PEN_STYLE, GRAPH_GRID_PEN_WEIGHT, GRAPH_GRID_COLOR);
   m_Graph.SetClientAreaColor(GRAPH_BACKGROUND);

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   m_pXFormat = new WBFL::Units::LengthTool(pDisplayUnits->GetSpanLengthUnit());
   m_Graph.SetXAxisValueFormat(m_pXFormat);
   m_Graph.SetXAxisTitle(std::_tstring(_T("Location (") + ((WBFL::Units::LengthTool*)m_pXFormat)->UnitTag() + _T(")")).c_str());
}

void CXBRLiveLoadGraphBuilder::UpdateYAxisUnits()
{
   delete m_pYFormat;

   ActionType actionType = m_GraphController.GetActionType();

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   switch(actionType)
   {
   case actionMoment:
      {
      const WBFL::Units::MomentData& momentUnit = pDisplayUnits->GetMomentUnit();
      m_pYFormat = new WBFL::Units::MomentTool(momentUnit);
      m_Graph.SetYAxisValueFormat(m_pYFormat);
      std::_tstring strYAxisTitle = _T("Moment (") + ((WBFL::Units::MomentTool*)m_pYFormat)->UnitTag() + _T(")");
      m_Graph.SetYAxisTitle(strYAxisTitle.c_str());
      break;
      }
   case actionShear:
      {
      const WBFL::Units::ForceData& shearUnit = pDisplayUnits->GetShearUnit();
      m_pYFormat = new WBFL::Units::ShearTool(shearUnit);
      m_Graph.SetYAxisValueFormat(m_pYFormat);
      std::_tstring strYAxisTitle = _T("Shear (") + ((WBFL::Units::ShearTool*)m_pYFormat)->UnitTag() + _T(")");
      m_Graph.SetYAxisTitle(strYAxisTitle.c_str());
      break;
      }
   default:
      ASSERT(false); 
   }
}

void CXBRLiveLoadGraphBuilder::UpdateGraphTitle()
{
   ActionType actionType = m_GraphController.GetActionType();
   m_Graph.SetTitle(GetGraphTitle(actionType));
}

void CXBRLiveLoadGraphBuilder::UpdateGraphData()
{
   m_Graph.ClearData();

   auto pBroker = EAFGetBroker();

   GET_IFACE2_NOCHECK(pBroker,IXBRRatingSpecification,pRatingSpec);

   ActionType actionType = m_GraphController.GetActionType();

   PierIDType pierID = m_GraphController.GetPierID();

   pgsTypes::LoadRatingType ratingType = m_GraphController.GetLoadRatingType();
   VehicleIndexType vehicleIdx = m_GraphController.GetVehicleIndex();
   IndexType permitLaneIdx = m_GraphController.GetPermitLaneIndex();

   GET_IFACE2(pBroker,IXBRPier,pPier);
   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

   GET_IFACE2(pBroker,IXBRPointOfInterest,pPoi);
   std::vector<xbrPointOfInterest> vPoi = pPoi->GetXBeamPointsOfInterest(pierID,POI_GRID | POI_BRG | POI_FACEOFCOLUMN | POI_COLUMN_LEFT | POI_COLUMN_RIGHT | POI_MIDPOINT | POI_FOC_DV2 | POI_FOC_DV);
   std::vector<xbrPointOfInterest> vWLPoi = pPoi->GetXBeamPointsOfInterest(pierID,POI_WHEELLINE);
   if ( !vWLPoi.empty() && 0 <= vWLPoi.front().GetDistFromStart() )
   {
      vPoi.insert(vPoi.begin(),vWLPoi.front());
   }

   if ( !vWLPoi.empty() && vWLPoi.back().GetDistFromStart() <= Lxb )
   {
      vPoi.insert(vPoi.end(),vWLPoi.back());
   }
   std::sort(vPoi.begin(),vPoi.end());

   IndexType llConfigIdx = m_GraphController.GetSelectedLiveLoadConfiguration();

   if ( !pRatingSpec->IsWSDOTEmergencyRating(ratingType) && !pRatingSpec->IsWSDOTPermitRating(ratingType) )
   {
      // only draw envelopes if this is not a permit rating, or if it is a permit rating, we aren't using the WSDOT method.
      // envelope doesn't make sense with WSDOT method.

      // Controlling envelope for all trucks for this rating type
      CString strLiveLoadName = GetLiveLoadTypeName(ratingType);
      IndexType minGraphIdx = m_Graph.CreateDataSeries(strLiveLoadName,PS_SOLID,GRAPH_PEN_WEIGHT,GREEN);
      IndexType maxGraphIdx = m_Graph.CreateDataSeries(nullptr,PS_SOLID,GRAPH_PEN_WEIGHT,GREEN);
      BuildControllingLiveLoadGraph(pierID,vPoi,ratingType,actionType,minGraphIdx,maxGraphIdx);

      GET_IFACE2(pBroker,IXBRProject,pProject);
      std::_tstring strLiveLoad = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);
      if ( strLiveLoad != NO_LIVE_LOAD_DEFINED )
      {
         IndexType minVehGraphIdx = m_Graph.CreateDataSeries(strLiveLoad.c_str(),PS_SOLID,GRAPH_PEN_WEIGHT,BLUE);
         IndexType maxVehGraphIdx = m_Graph.CreateDataSeries(nullptr,PS_SOLID,GRAPH_PEN_WEIGHT,BLUE);
         BuildControllingVehicularLiveLoadGraph(pierID,vPoi,ratingType,vehicleIdx,actionType,minVehGraphIdx,maxVehGraphIdx);
      }
   }

   if ( llConfigIdx != INVALID_INDEX )
   {
      CString strName;
      strName.Format(_T("Live Load Configuration %d"),LABEL_INDEX(llConfigIdx));
      m_Graph.SetSubtitle(strName);

      IndexType graphIdx = m_Graph.CreateDataSeries(nullptr,PS_SOLID,GRAPH_PEN_WEIGHT,RED);

      GET_IFACE2(pBroker,IXBRProductForces,pProductForces);

      IndexType permitLaneIdx = m_GraphController.GetPermitLaneIndex();
      WheelLineConfiguration wheelLineConfig = pProductForces->GetLiveLoadConfiguration(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx);
      std::vector<WheelLinePlacement>::iterator iter(wheelLineConfig.begin());
      std::vector<WheelLinePlacement>::iterator end(wheelLineConfig.end());
      for ( ; iter != end; iter++ )
      {
         WheelLinePlacement& placement = *iter;
         xbrPointOfInterest poi(pPoi->GetNearestPointOfInterest(pierID,xbrPointOfInterest(INVALID_ID,placement.Xxb)));
         ATLASSERT(poi.HasAttribute(POI_WHEELLINE));
         vPoi.push_back(poi);
      }
      std::sort(vPoi.begin(),vPoi.end());

      if (pRatingSpec->IsWSDOTEmergencyRating(ratingType) || pRatingSpec->IsWSDOTPermitRating(ratingType))
      {
         m_Graph.SetDataLabel(graphIdx,_T("Legal"));

         CString strSeries;
         strSeries.Format(_T("%s"), pRatingSpec->IsWSDOTEmergencyRating(ratingType) ? _T("Emergency") : _T("Permit"));

         IndexType permitGraphIdx = m_Graph.CreateDataSeries(strSeries,PS_SOLID,GRAPH_PEN_WEIGHT,GREEN);
         BuildWSDOTPermitLiveLoadGraph(pierID,vPoi,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,actionType,permitGraphIdx,graphIdx);
      }
      else
      {
         BuildLiveLoadGraph(pierID,vPoi,ratingType,vehicleIdx,llConfigIdx,actionType,graphIdx);
      }
   }
}

void CXBRLiveLoadGraphBuilder::DrawGraphNow(CWnd* pGraphWnd,CDC* pDC)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   int save = pDC->SaveDC();

   // The graph is valided and there was not an error
   // updating data.... draw the graph
   CRect rect = GetView()->GetDrawingRect();

   m_Graph.SetOutputRect(rect);
   m_Graph.UpdateGraphMetrics(pDC->GetSafeHdc());
   m_Graph.DrawBackground(pDC->GetSafeHdc());

   IndexType llConfigIdx = m_GraphController.GetSelectedLiveLoadConfiguration();
   if ( llConfigIdx != INVALID_INDEX )
   {
      PierIDType pierID = m_GraphController.GetPierID();
      pgsTypes::LoadRatingType ratingType = m_GraphController.GetLoadRatingType();
      VehicleIndexType vehicleIdx = m_GraphController.GetVehicleIndex();
      DrawLiveLoadConfig(pGraphWnd,pDC,pierID,ratingType,vehicleIdx,llConfigIdx);
   }

   m_Graph.DrawDataSeries(pDC->GetSafeHdc());

   pDC->RestoreDC(save);
}

LPCTSTR CXBRLiveLoadGraphBuilder::GetGraphTitle(ActionType actionType)
{
   switch(actionType)
   {
   case actionMoment:
      return _T("Moment");

   case actionShear:
      return _T("Shear");
   }
   ATLASSERT(false);
   return _T("Unknown Graph Type");
}

void CXBRLiveLoadGraphBuilder::BuildControllingLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mmin, Mmax;
         pResults->GetMoment(pierID,ratingType,poi,&Mmin,&Mmax,nullptr,nullptr);
         Mmin = m_pYFormat->Convert(Mmin);
         Mmax = m_pYFormat->Convert(Mmax);
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Mmin));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Mmax));
      }
      else
      {
         WBFL::System::SectionValue FyMin, FyMax;
         pResults->GetShear(pierID,ratingType,poi,&FyMin,&FyMax,nullptr,nullptr,nullptr,nullptr);
         Float64 Vlmin = m_pYFormat->Convert(FyMin.Left());
         Float64 Vrmin = m_pYFormat->Convert(FyMin.Right());
         Float64 Vlmax = m_pYFormat->Convert(FyMax.Left());
         Float64 Vrmax = m_pYFormat->Convert(FyMax.Right());

         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vlmin));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vrmin));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vlmax));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vrmax));
      }
   }
}

void CXBRLiveLoadGraphBuilder::BuildControllingVehicularLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mmin, Mmax;
         pResults->GetMoment(pierID,ratingType,vehicleIdx,poi,&Mmin,&Mmax,nullptr,nullptr);
         Mmin = m_pYFormat->Convert(Mmin);
         Mmax = m_pYFormat->Convert(Mmax);
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Mmin));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Mmax));
      }
      else
      {
         WBFL::System::SectionValue FyMin, FyMax;
         pResults->GetShear(pierID,ratingType,vehicleIdx,poi,&FyMin,&FyMax,nullptr,nullptr);
         Float64 Vlmin = m_pYFormat->Convert(FyMin.Left());
         Float64 Vrmin = m_pYFormat->Convert(FyMin.Right());
         Float64 Vlmax = m_pYFormat->Convert(FyMax.Left());
         Float64 Vrmax = m_pYFormat->Convert(FyMax.Right());
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vlmin));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vrmin));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vlmax));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vrmax));
      }
   }
}

void CXBRLiveLoadGraphBuilder::BuildLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,ActionType actionType,IndexType graphIdx)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mz = pResults->GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
         Mz = m_pYFormat->Convert(Mz);
         m_Graph.AddPoint(graphIdx, WBFL::Graphing::Point(X,Mz));
      }
      else
      {
         WBFL::System::SectionValue Fy = pResults->GetShear(pierID,ratingType,vehicleIdx,llConfigIdx,poi);
         Float64 Vl = m_pYFormat->Convert(Fy.Left());
         Float64 Vr = m_pYFormat->Convert(Fy.Right());
         m_Graph.AddPoint(graphIdx,WBFL::Graphing::Point(X,Vl));
         m_Graph.AddPoint(graphIdx,WBFL::Graphing::Point(X,Vr));
      }
   }
}

void CXBRLiveLoadGraphBuilder::BuildWSDOTPermitLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,ActionType actionType,IndexType permitGraphIdx,IndexType legalGraphIdx)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   // make sure the permitLaneIdx is value... if multipe loads are selected, and the live load cases
   // have a different number of lanes, the permit lane index could be too big for some cases.
   GET_IFACE2(pBroker,IXBRProductForces,pProductForces);
   IndexType nLoadedLanes = pProductForces->GetLoadedLaneCount(pierID,llConfigIdx);
   permitLaneIdx = (nLoadedLanes <= permitLaneIdx ? nLoadedLanes-1 : permitLaneIdx);


   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mpermit, Mlegal;
         pResults->GetMoment(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,poi,&Mpermit,&Mlegal);
         Mpermit = m_pYFormat->Convert(Mpermit);
         Mlegal  = m_pYFormat->Convert(Mlegal);
         m_Graph.AddPoint(permitGraphIdx,WBFL::Graphing::Point(X,Mpermit));
         m_Graph.AddPoint(legalGraphIdx, WBFL::Graphing::Point(X,Mlegal));
      }
      else
      {
         WBFL::System::SectionValue Vpermit, Vlegal;
         pResults->GetShear(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx,poi,&Vpermit,&Vlegal);
         Vpermit.Left() = m_pYFormat->Convert(Vpermit.Left());
         Vpermit.Right() = m_pYFormat->Convert(Vpermit.Right());
         Vlegal.Left() = m_pYFormat->Convert(Vlegal.Left());
         Vlegal.Right() = m_pYFormat->Convert(Vlegal.Right());
         m_Graph.AddPoint(permitGraphIdx,WBFL::Graphing::Point(X,Vpermit.Left()));
         m_Graph.AddPoint(permitGraphIdx,WBFL::Graphing::Point(X,Vpermit.Right()));
         m_Graph.AddPoint(legalGraphIdx, WBFL::Graphing::Point(X,Vlegal.Left()));
         m_Graph.AddPoint(legalGraphIdx, WBFL::Graphing::Point(X,Vlegal.Right()));
      }
   }
}

void CXBRLiveLoadGraphBuilder::DrawLiveLoadConfig(CWnd* pGraphWnd,CDC* pDC,PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx)
{
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRProductForces,pProductForces);
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   IndexType permitLaneIdx = m_GraphController.GetPermitLaneIndex();
   WheelLineConfiguration wheelLineConfig = pProductForces->GetLiveLoadConfiguration(pierID,ratingType,vehicleIdx,llConfigIdx,permitLaneIdx);

   WBFL::Graphing::PointMapper mapper( m_Graph.GetClientAreaPointMapper(pDC->GetSafeHdc()) );
   LONG x,y;
   mapper.WPtoDP(0,0,&x,&y);

   // Get world extents and world origin
   WBFL::Graphing::Size wExt  = mapper.GetWorldExt();
   WBFL::Graphing::Point wOrg = mapper.GetWorldOrg();
   
   // get device extents and device origin
   LONG dx,dy;
   mapper.GetDeviceExt(&dx,&dy);

   LONG dox,doy;
   mapper.GetDeviceOrg(&dox,&doy);

   // compute a new device origin in the y direction
   doy = (LONG)Round(y - (0.0 - wOrg.Y())*((Float64)(dx)/-wExt.Dx()));

   // change the y scaling (use same scaling as X direction)
   mapper.SetWorldExt(wExt.Dx(),wExt.Dx());
   mapper.SetDeviceExt(dx,dx);

   // change the device origin
   mapper.SetDeviceOrg(dox,doy);

   CPen legal_pen(PS_SOLID,1,RED);
   CPen permit_pen(PS_SOLID,1,GREEN);
   CPen* pOldPen = pDC->SelectObject(&legal_pen);

   HFONT hFont = CreateRotatedFont(pDC->GetSafeHdc(),900,8,_T("Arial"));
   HGDIOBJ hOldFont = pDC->SelectObject(hFont);

   UINT oldTextAlign = pDC->GetTextAlign();
   int nBkMode = pDC->SetBkMode(TRANSPARENT);

   LONG dx_last, dy_last;
   std::vector<WheelLinePlacement>::iterator iter(wheelLineConfig.begin());
   std::vector<WheelLinePlacement>::iterator end(wheelLineConfig.end());
   for ( ; iter != end; iter++ )
   {
      IndexType idx = std::distance(wheelLineConfig.begin(),iter);
      IndexType laneIdx = idx/2;
      if ( laneIdx == permitLaneIdx )
      {
         pDC->SelectObject(&permit_pen);
      }
      else
      {
         pDC->SelectObject(&legal_pen);
      }

      WheelLinePlacement& placement = *iter;

      Float64 X = m_pXFormat->Convert(placement.Xxb);
      Float64 Y = m_pYFormat->Convert(0);
      LONG dx,dy;
      mapper.WPtoDP(X,Y,&dx,&dy);

      pDC->MoveTo(dx,dy);
      pDC->LineTo(dx,dy-30);

      // draw the crossbar connecting wheel lines
      if ( idx % 2 == 0 )
      {
         pDC->SetTextAlign(TA_LEFT | TA_TOP);
         dx_last = dx;
         dy_last = dy-30;
      }
      else
      {
         pDC->SetTextAlign(TA_LEFT | TA_BOTTOM);
         pDC->MoveTo(dx_last,dy_last);
         pDC->LineTo(dx,dy-30);
      }

      // arrow head
      pDC->MoveTo(dx,dy);
      pDC->LineTo(dx-10,dy-10);
      pDC->MoveTo(dx,dy);
      pDC->LineTo(dx+10,dy-10);

      CString str;
      str.Format(_T("%s"),::FormatDimension(placement.Xxb,pDisplayUnits->GetSpanLengthUnit()));
      pDC->TextOut(dx,dy-30,str);

   }

   pDC->SelectObject(pOldPen);
   pDC->SelectObject(hOldFont);
   pDC->SetTextAlign(oldTextAlign);
   pDC->SetBkMode(nBkMode);

   DeleteObject(hFont);
}

void CXBRLiveLoadGraphBuilder::ExportGraphData(LPCTSTR rstrDefaultFileName)
{
   CExportGraphXYTool::ExportGraphData(m_Graph,rstrDefaultFileName);
}