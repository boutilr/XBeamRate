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
#include "AnalysisResultsGraphBuilder.h"

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
#include <Graphs\ExportGraphXYTool.h>

#include <algorithm>



// create a dummy unit conversion tool to pacify the graph constructor
static WBFL::Units::LengthData DUMMY(WBFL::Units::Measure::Meter);
static WBFL::Units::LengthTool DUMMY_TOOL(DUMMY);

BEGIN_MESSAGE_MAP(CXBRAnalysisResultsGraphBuilder, CEAFAutoCalcGraphBuilder)
   ON_BN_CLICKED(IDC_MOMENT, &CXBRAnalysisResultsGraphBuilder::OnGraphTypeChanged)
   ON_BN_CLICKED(IDC_SHEAR, &CXBRAnalysisResultsGraphBuilder::OnGraphTypeChanged)
   ON_LBN_SELCHANGE(IDC_LOADING,&CXBRAnalysisResultsGraphBuilder::OnLbnSelChanged)
   ON_CBN_SELCHANGE(IDC_PIERS,&CXBRAnalysisResultsGraphBuilder::OnPierChanged)
END_MESSAGE_MAP()


CXBRAnalysisResultsGraphBuilder::CXBRAnalysisResultsGraphBuilder() :
CEAFAutoCalcGraphBuilder(),
m_Graph(&DUMMY_TOOL,&DUMMY_TOOL),
m_pXFormat(nullptr),
m_pYFormat(nullptr)
{
   SetName(_T("Analysis Results"));
   InitGraph();
}

CXBRAnalysisResultsGraphBuilder::CXBRAnalysisResultsGraphBuilder(const CXBRAnalysisResultsGraphBuilder& other) :
CEAFAutoCalcGraphBuilder(other),
m_Graph(&DUMMY_TOOL, &DUMMY_TOOL),
m_pXFormat(nullptr),
m_pYFormat(nullptr)
{
   InitGraph();
}

CXBRAnalysisResultsGraphBuilder::~CXBRAnalysisResultsGraphBuilder()
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

CEAFGraphControlWindow* CXBRAnalysisResultsGraphBuilder::GetGraphControlWindow()
{
   return &m_GraphController;
}

std::unique_ptr<WBFL::Graphing::GraphBuilder> CXBRAnalysisResultsGraphBuilder::Clone() const
{
   // set the module state or the commands wont route to the
   // the graph control window
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return std::make_unique<CXBRAnalysisResultsGraphBuilder>(*this);
}

void CXBRAnalysisResultsGraphBuilder::CreateViewController(IEAFViewController** ppController)
{
   *ppController = nullptr;
}

const CXBRAnalysisResultsGraphDefinitions& CXBRAnalysisResultsGraphBuilder::GetGraphDefinitions()
{
   return m_GraphDefinitions;
}

BOOL CXBRAnalysisResultsGraphBuilder::CreateGraphController(CWnd* pParent,UINT nID)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   if ( !m_GraphController.Create(pParent,IDD_ANALYSIS_RESULTS_GRAPH_CONTROLLER, CBRS_LEFT, nID) )
   {
      TRACE0("Failed to create control bar\n");
      return FALSE; // failed to create
   }

   UpdateGraphDefinitions();

   return TRUE;
}

void CXBRAnalysisResultsGraphBuilder::OnGraphTypeChanged()
{
   InvalidateGraph();
   Update();
}

void CXBRAnalysisResultsGraphBuilder::OnLbnSelChanged()
{
   OnGraphTypeChanged();
}

void CXBRAnalysisResultsGraphBuilder::OnPierChanged()
{
   OnGraphTypeChanged();
}

bool CXBRAnalysisResultsGraphBuilder::UpdateNow()
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

void CXBRAnalysisResultsGraphBuilder::InitGraph()
{
   m_Graph.SetGridPenStyle(GRAPH_GRID_PEN_STYLE, GRAPH_GRID_PEN_WEIGHT, GRAPH_GRID_COLOR);
   m_Graph.SetClientAreaColor(GRAPH_BACKGROUND);

   auto pBroker = EAFGetBroker();
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   m_pXFormat = new WBFL::Units::LengthTool(pDisplayUnits->GetSpanLengthUnit());
   m_Graph.SetXAxisValueFormat(m_pXFormat);
   m_Graph.SetXAxisTitle(std::_tstring(_T("Location (") + ((WBFL::Units::LengthTool*)m_pXFormat)->UnitTag() + _T(")")).c_str());
}

void CXBRAnalysisResultsGraphBuilder::UpdateYAxisUnits()
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
      ASSERT(0); 
   }
}

void CXBRAnalysisResultsGraphBuilder::UpdateGraphTitle()
{
   ActionType actionType = m_GraphController.GetActionType();
   m_Graph.SetTitle(GetGraphTitle(actionType));
}

void CXBRAnalysisResultsGraphBuilder::UpdateGraphData()
{
   m_Graph.ClearData();

   ActionType actionType = m_GraphController.GetActionType();

   PierIDType pierID = m_GraphController.GetPierID();

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRPier,pPier);
   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

   GET_IFACE2(pBroker,IXBRPointOfInterest,pPoi);
   std::vector<xbrPointOfInterest> vPoi = pPoi->GetXBeamPointsOfInterest(pierID,POI_GRID | POI_BRG | POI_FACEOFCOLUMN | POI_COLUMN_LEFT | POI_COLUMN_RIGHT | POI_MIDPOINT | POI_FOC_DV2 | POI_FOC_DV);
   std::vector<xbrPointOfInterest> vWLPoi = pPoi->GetXBeamPointsOfInterest(pierID,POI_WHEELLINE);
   if ( !vWLPoi.empty() && 0 <= vWLPoi.front().GetDistFromStart() )
   {
      vPoi.insert(vPoi.begin(),vWLPoi.front());
   }

   if (!vWLPoi.empty() && vWLPoi.back().GetDistFromStart() <= Lxb )
   {
      vPoi.insert(vPoi.end(),vWLPoi.back());
   }
   std::sort(vPoi.begin(),vPoi.end());

   CXBRAnalysisResultsGraphDefinitions graphDefs = m_GraphController.GetSelectedGraphDefinitions();
   IndexType nGraphs = graphDefs.GetGraphDefinitionCount();
   for ( IndexType idx = 0; idx < nGraphs; idx++ )
   {
      CXBRAnalysisResultsGraphDefinition& graphDef = graphDefs.GetGraphDefinition(idx);

      IndexType selectedGraphIdx = m_GraphDefinitions.GetGraphIndex(graphDef.m_ID);
      COLORREF color = m_GraphColor.GetColor(selectedGraphIdx);

      IndexType graphIdx, maxGraphIdx, minGraphIdx;
      if ( graphDef.m_GraphType == graphCapacity ||
           graphDef.m_GraphType == graphVehicularLiveLoad ||
           graphDef.m_GraphType == graphLiveLoad ||
           graphDef.m_GraphType == graphLimitState
         )
      {
         maxGraphIdx = m_Graph.CreateDataSeries(graphDef.m_Name.c_str(),PS_SOLID,2,color);
         minGraphIdx = m_Graph.CreateDataSeries(_T(""),PS_SOLID,2,color);
      }
      else
      {
         graphIdx = m_Graph.CreateDataSeries(graphDef.m_Name.c_str(),PS_SOLID,2,color);
      }


      if ( graphDef.m_GraphType == graphProduct )
      {
         BuildProductForceGraph(pierID,vPoi,graphDef,actionType,graphIdx);
      }
      else if ( graphDef.m_GraphType == graphCombined )
      {
         BuildCombinedForceGraph(pierID,vPoi,graphDef,actionType,graphIdx);
      }
      else if ( graphDef.m_GraphType == graphVehicularLiveLoad )
      {
         BuildVehicularLiveLoadGraph(pierID,vPoi,graphDef,actionType,minGraphIdx,maxGraphIdx);
      }
      else if ( graphDef.m_GraphType == graphLiveLoad )
      {
         BuildLiveLoadGraph(pierID,vPoi,graphDef,actionType,minGraphIdx,maxGraphIdx);
      }
      else if ( graphDef.m_GraphType == graphLimitState )
      {
         BuildLimitStateGraph(pierID,vPoi,graphDef,actionType,minGraphIdx,maxGraphIdx);
      }
      else if ( graphDef.m_GraphType == graphCapacity )
      {
         BuildCapacityGraph(pierID,vPoi,actionType,minGraphIdx,maxGraphIdx);
      }
      else
      {
         ATLASSERT(false);
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::UpdateGraphDefinitions()
{
   m_GraphDefinitions.Clear();

   PierIDType pierID = m_GraphController.GetPierID();
   if ( !IsStandAlone() && pierID == INVALID_ID )
   {
      // nothing to graph
      return;
   }

   auto pBroker = EAFGetBroker();

   // include time dependent load cases if
   // 1) we are running as a stand alone application
   // 2) we are running as a PGS extension and the losses are computed with the time-step method
   bool bIncludeTimeDependentLoads = true;

   if ( IsPGSExtension() )
   {
      GET_IFACE2( pBroker, ILossParameters, pLossParams);
      if ( pLossParams->GetLossMethod() != PrestressLossCriteria::LossMethodType::TIME_STEP )
      {
         // PGS extension and not time-step analysis... don't include time dependent loads
         bIncludeTimeDependentLoads = false;
      }
   }

   IDType graphID = 0;
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Lower Cross Beam Dead Load"),        xbrTypes::pftLowerXBeam));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure Diaphragm Dead Load"),xbrTypes::pftUpperXBeam));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure DC Reactions"),       xbrTypes::pftDCReactions));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure DW Reactions"),       xbrTypes::pftDWReactions));
   if ( bIncludeTimeDependentLoads )
   {
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure CR Reactions"),xbrTypes::pftCRReactions));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure SH Reactions"),xbrTypes::pftSHReactions));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure RE Reactions"),xbrTypes::pftREReactions));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Superstructure PS Reactions"),xbrTypes::pftPSReactions));
   }

   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("DC"),xbrTypes::lcDC));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("DW"),xbrTypes::lcDW));
   if ( bIncludeTimeDependentLoads )
   {
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("CR"),xbrTypes::lcCR));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("SH"),xbrTypes::lcSH));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("RE"),xbrTypes::lcRE));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("PS"),xbrTypes::lcPS));
   }

   GET_IFACE2(pBroker,IXBRProject,pProject);
   for ( int i = 1; i < 6; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;
      IndexType nVehicles = pProject->GetLiveLoadReactionCount(pierID,ratingType);
      for ( VehicleIndexType vehicleIdx = 0; vehicleIdx < nVehicles; vehicleIdx++ )
      {
         std::_tstring strName = pProject->GetLiveLoadName(pierID,ratingType,vehicleIdx);
         if ( strName != NO_LIVE_LOAD_DEFINED )
         {
            m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,strName.c_str(),ratingType,vehicleIdx));
         }
      }
   }

   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("LLIM (Inventory/Operating)"),pgsTypes::lrDesign_Inventory));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("LLIM (Legal Routine)"),      pgsTypes::lrLegal_Routine));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++, _T("LLIM (Legal Special)"), pgsTypes::lrLegal_Special));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++, _T("LLIM (Legal Emergency)"), pgsTypes::lrLegal_Emergency));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("LLIM (Permit Routine)"),     pgsTypes::lrPermit_Routine));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("LLIM (Permit Special)"),     pgsTypes::lrPermit_Special));

   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Strength I (Inventory)"),      pgsTypes::StrengthI_Inventory));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Strength I (Operating)"),      pgsTypes::StrengthI_Operating));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Strength I (Legal Routine)"),  pgsTypes::StrengthI_LegalRoutine));
   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++, _T("Strength I (Legal Special)"), pgsTypes::StrengthI_LegalSpecial));
   GET_IFACE2(pBroker, IXBRRatingSpecification, pRatingSpec);
   if (pRatingSpec->GetEmergencyRatingMethod() != xbrTypes::prmWSDOT)
   {
      // NOTE: Strength I limit state doesn't make sense for WSDOT Emergency vehicle method. The graphs show the limit state based on a controlling envelope. For the WSDOT method
      // we don't have a controlling envelope per se. WSDOT method minimizes RF based on a combination of Legal and Emergency loads
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++, _T("Strength I (Legal Emergency)"), pgsTypes::StrengthI_LegalEmergency));
   }

   if ( pRatingSpec->GetPermitRatingMethod() != xbrTypes::prmWSDOT )
   {
      // NOTE: Strength II limit state doesn't make sense for WSDOT method. The graphs show the limit state based on a controlling envelope. For the WSDOT method
      // we don't have a controlling envelope per se. WSDOT method minimizes RF based on a combination of Legal and Permit loads
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Strength II (Permit Routine)"),pgsTypes::StrengthII_PermitRoutine));
      m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Strength II (Permit Special)"),pgsTypes::StrengthII_PermitSpecial));
   }

   m_GraphDefinitions.AddGraphDefinition(CXBRAnalysisResultsGraphDefinition(graphID++,_T("Capacity")));

   m_GraphController.FillLoadingList();
}

void CXBRAnalysisResultsGraphBuilder::DrawGraphNow(CWnd* pGraphWnd,CDC* pDC)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   int save = pDC->SaveDC();

   // The graph is valided and there was not an error
   // updating data.... draw the graph
   CRect rect = GetView()->GetDrawingRect();

   m_Graph.SetOutputRect(rect);
   m_Graph.UpdateGraphMetrics(pDC->GetSafeHdc());
   m_Graph.Draw(pDC->GetSafeHdc());

   pDC->RestoreDC(save);
}

LPCTSTR CXBRAnalysisResultsGraphBuilder::GetGraphTitle(ActionType actionType)
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

void CXBRAnalysisResultsGraphBuilder::BuildProductForceGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType graphIdx)
{
   xbrTypes::ProductForceType pfType = graphDef.m_LoadType.ProductLoadType;

   auto pBroker = EAFGetBroker();
   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mz = pResults->GetMoment(pierID,pfType,poi);
         Mz = m_pYFormat->Convert(Mz);
         WBFL::Graphing::Point point(X,Mz);
         m_Graph.AddPoint(graphIdx,point);
      }
      else
      {
         WBFL::System::SectionValue V = pResults->GetShear(pierID,pfType,poi);
         Float64 Vl = m_pYFormat->Convert(V.Left());
         Float64 Vr = m_pYFormat->Convert(V.Right());
         m_Graph.AddPoint(graphIdx, WBFL::Graphing::Point(X,Vl));
         m_Graph.AddPoint(graphIdx, WBFL::Graphing::Point(X,Vr));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::BuildCombinedForceGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType graphIdx)
{
   xbrTypes::CombinedForceType comboType = graphDef.m_LoadType.CombinedLoadType;

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mz = pResults->GetMoment(pierID,comboType,poi);
         Mz = m_pYFormat->Convert(Mz);
         WBFL::Graphing::Point point(X,Mz);
         m_Graph.AddPoint(graphIdx,point);
      }
      else
      {
         WBFL::System::SectionValue V = pResults->GetShear(pierID,comboType,poi);
         Float64 Vl = m_pYFormat->Convert(V.Left());
         Float64 Vr = m_pYFormat->Convert(V.Right());
         m_Graph.AddPoint(graphIdx, WBFL::Graphing::Point(X,Vl));
         m_Graph.AddPoint(graphIdx, WBFL::Graphing::Point(X,Vr));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::BuildVehicularLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   pgsTypes::LoadRatingType ratingType = graphDef.m_LoadType.LiveLoadType;

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mmin, Mmax;
         pResults->GetMoment(pierID,ratingType,graphDef.m_VehicleIndex,poi,&Mmin,&Mmax,nullptr,nullptr);
         Mmin = m_pYFormat->Convert(Mmin);
         Mmax = m_pYFormat->Convert(Mmax);
         m_Graph.AddPoint(maxGraphIdx, WBFL::Graphing::Point(X,Mmax));
         m_Graph.AddPoint(minGraphIdx, WBFL::Graphing::Point(X,Mmin));
      }
      else
      {
         WBFL::System::SectionValue Vmin, Vmax;
         pResults->GetShear(pierID,ratingType,graphDef.m_VehicleIndex,poi,&Vmin,&Vmax,nullptr,nullptr);
         Float64 Vlmax = m_pYFormat->Convert(Vmax.Left());
         Float64 Vrmax = m_pYFormat->Convert(Vmax.Right());
         Float64 Vlmin = m_pYFormat->Convert(Vmin.Left());
         Float64 Vrmin = m_pYFormat->Convert(Vmin.Right());
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vlmax));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vrmax));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vlmin));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vrmin));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::BuildLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   pgsTypes::LoadRatingType ratingType = graphDef.m_LoadType.LiveLoadType;

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
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Mmax));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Mmin));
      }
      else
      {
         WBFL::System::SectionValue Vmin, Vmax;
         pResults->GetShear(pierID,ratingType,poi,&Vmin,&Vmax,nullptr,nullptr,nullptr,nullptr);
         Float64 Vlmax = m_pYFormat->Convert(Vmax.Left());
         Float64 Vrmax = m_pYFormat->Convert(Vmax.Right());
         Float64 Vlmin = m_pYFormat->Convert(Vmin.Left());
         Float64 Vrmin = m_pYFormat->Convert(Vmin.Right());
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vlmax));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vrmax));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vlmin));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vrmin));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::BuildLimitStateGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   pgsTypes::LimitState limitState = graphDef.m_LoadType.LimitStateType;

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRAnalysisResults,pResults);

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mmin, Mmax;
         pResults->GetMoment(pierID,limitState,poi,&Mmin,&Mmax);
         Mmin = m_pYFormat->Convert(Mmin);
         Mmax = m_pYFormat->Convert(Mmax);
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Mmax));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Mmin));
      }
      else
      {
         WBFL::System::SectionValue Vmin, Vmax;
         pResults->GetShear(pierID,limitState,poi,&Vmin,&Vmax);
         Float64 Vlmax = m_pYFormat->Convert(Vmax.Left());
         Float64 Vrmax = m_pYFormat->Convert(Vmax.Right());
         Float64 Vlmin = m_pYFormat->Convert(Vmin.Left());
         Float64 Vrmin = m_pYFormat->Convert(Vmin.Right());
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vlmax));
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,Vrmax));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vlmin));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,Vrmin));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::BuildCapacityGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx)
{
   auto pBroker = EAFGetBroker();
   GET_IFACE2_NOCHECK(pBroker,IXBRMomentCapacity,pMomentCapacity);
   GET_IFACE2_NOCHECK(pBroker,IXBRShearCapacity,pShearCapacity);

   xbrTypes::Stage stage = xbrTypes::Stage2;

   for (const auto& poi : vPoi)
   {
      Float64 X = poi.GetDistFromStart();
      X  = m_pXFormat->Convert(X);

      if ( actionType == actionMoment )
      {
         Float64 Mz = pMomentCapacity->GetMomentCapacity(pierID,stage,poi,true);
         Mz = m_pYFormat->Convert(Mz);
         WBFL::Graphing::Point point(X,Mz);
         m_Graph.AddPoint(maxGraphIdx,point);

         Mz = pMomentCapacity->GetMomentCapacity(pierID,stage,poi,false);
         Mz = m_pYFormat->Convert(Mz);
         point.Y() = Mz;
         m_Graph.AddPoint(minGraphIdx,point);
      }
      else
      {
         Float64 V = pShearCapacity->GetShearCapacity(pierID,stage,poi);
         V = m_pYFormat->Convert(V);
         m_Graph.AddPoint(maxGraphIdx,WBFL::Graphing::Point(X,V));
         m_Graph.AddPoint(minGraphIdx,WBFL::Graphing::Point(X,-V));
      }
   }
}

void CXBRAnalysisResultsGraphBuilder::ExportGraphData(LPCTSTR rstrDefaultFileName)
{
   CExportGraphXYTool::ExportGraphData(m_Graph,rstrDefaultFileName);
}