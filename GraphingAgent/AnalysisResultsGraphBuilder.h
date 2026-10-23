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

#include <EAF\EAFAutoCalcGraphBuilder.h>
#include "AnalysisResultsGraphController.h"
#include "AnalysisResultsGraphDefinition.h"

class CEAFGraphChildFrame;

namespace WBFL
{
   namespace Units
   {
      class PhysicalConverter;
   };
};

class CXBRAnalysisResultsGraphBuilder : public CEAFAutoCalcGraphBuilder
{
public:
   CXBRAnalysisResultsGraphBuilder();
   CXBRAnalysisResultsGraphBuilder(const CXBRAnalysisResultsGraphBuilder& other);
   virtual ~CXBRAnalysisResultsGraphBuilder();

   virtual BOOL CreateGraphController(CWnd* pParent,UINT nID) override;
   virtual void DrawGraphNow(CWnd* pGraphWnd,CDC* pDC) override;
   virtual std::unique_ptr<WBFL::Graphing::GraphBuilder> Clone() const;
   const CXBRAnalysisResultsGraphDefinitions& GetGraphDefinitions();

   void UpdateGraphDefinitions();

   virtual void CreateViewController(IEAFViewController** ppController) override;
   void ExportGraphData(LPCTSTR rstrDefaultFileName);

protected:
   virtual CEAFGraphControlWindow* GetGraphControlWindow() override;

   afx_msg void OnGraphTypeChanged();
   afx_msg void OnLbnSelChanged();
   afx_msg void OnPierChanged();

   virtual bool UpdateNow() override;
   void InitGraph();
   void UpdateYAxisUnits();
   void UpdateGraphTitle();
   void UpdateGraphData();

   void BuildProductForceGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType graphIdx);
   void BuildCombinedForceGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType graphIdx);
   void BuildVehicularLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx);
   void BuildLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx);
   void BuildLimitStateGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,const CXBRAnalysisResultsGraphDefinition& graphDef,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx);
   void BuildCapacityGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx);

   LPCTSTR GetGraphTitle(ActionType actionType);

   WBFL::Graphing::GraphColor m_GraphColor;
   WBFL::Graphing::GraphXY m_Graph;
   WBFL::Units::PhysicalConverter* m_pXFormat;
   WBFL::Units::PhysicalConverter* m_pYFormat;

   DECLARE_MESSAGE_MAP()

private:
   CXBRAnalysisResultsGraphDefinitions m_GraphDefinitions;
   CXBRAnalysisResultsGraphController m_GraphController;
};
