///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright � 1999-2015  Washington State Department of Transportation
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

#include <EAF\EAFGraphBuilderBase.h>
#include "LiveLoadGraphController.h"
#include "LiveLoadGraphDefinition.h"

class CEAFGraphChildFrame;
class grGraphXY;
class arvPhysicalConverter;

class CXBRLiveLoadGraphBuilder : public CEAFGraphBuilderBase
{
public:
   CXBRLiveLoadGraphBuilder();
   CXBRLiveLoadGraphBuilder(const CXBRLiveLoadGraphBuilder& other);
   virtual BOOL CreateGraphController(CWnd* pParent,UINT nID);
   virtual void DrawGraphNow(CWnd* pGraphWnd,CDC* pDC);
   virtual CGraphBuilder* Clone();

protected:
   virtual CEAFGraphControlWindow* GetGraphControlWindow();

   afx_msg void OnGraphTypeChanged();
   afx_msg void OnLbnSelChanged();
   afx_msg void OnSelectAll();
   afx_msg void OnPierChanged();
   afx_msg void OnNext();
   afx_msg void OnPrev();
   afx_msg void OnRatingTypeChanged();
   afx_msg void OnVehicleTypeChanged();

   virtual bool UpdateNow();

   void BuildControllingLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx,grGraphXY& graph,arvPhysicalConverter* pHorizontalAxisFormat,arvPhysicalConverter* pVerticalAxisFormat);
   void BuildControllingVehicularLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,ActionType actionType,IndexType minGraphIdx,IndexType maxGraphIdx,grGraphXY& graph,arvPhysicalConverter* pHorizontalAxisFormat,arvPhysicalConverter* pVerticalAxisFormat);
   void BuildLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,ActionType actionType,IndexType graphIdx,grGraphXY& graph,arvPhysicalConverter* pHorizontalAxisFormat,arvPhysicalConverter* pVerticalAxisFormat);
   void BuildWSDOTPermitLiveLoadGraph(PierIDType pierID,const std::vector<xbrPointOfInterest>& vPoi,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,IndexType permitLaneIdx,ActionType actionType,IndexType permitGraphIdx,IndexType legalGraphIdx,grGraphXY& graph,arvPhysicalConverter* pHorizontalAxisFormat,arvPhysicalConverter* pVerticalAxisFormat);

   void DrawLiveLoadConfig(CWnd* pGraphWnd,CDC* pDC,grGraphXY& graph,PierIDType pierID,pgsTypes::LoadRatingType ratingType,VehicleIndexType vehicleIdx,IndexType llConfigIdx,arvPhysicalConverter* pHorizontalAxisFormat,arvPhysicalConverter* pVerticalAxisFormat);

   LPCTSTR GetGraphTitle(ActionType actionType);

   DECLARE_MESSAGE_MAP()

private:
   CXBRLiveLoadGraphController m_GraphController;
};