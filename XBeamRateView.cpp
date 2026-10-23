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

// XBeamRateView.cpp : implementation of the CXBeamRateView class
//

#include "stdafx.h"
#include "resource.h"

#include "XBeamRateDoc.h"
#include "XBeamRateView.h"
#include "XBeamRateChildFrame.h"
#include "SectionCut.h"
#include "DisplayObjectFactory.h"

#include <AgentTools.h>
#include <IFace\XBeamRateAgent.h>
#include <IFace\Project.h>

#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>
#include <IFace\Pier.h>
#include <IFace\EditByUI.h>
#include <..\..\PGSuper\Include\IFace\Project.h>
#include <..\..\PGSuper\Include\IFace\Bridge.h>
#include <..\..\PGSuper\Include\IFace\PointOfInterest.h>
#include <..\..\PGSuper\Include\IFace\Intervals.h>
#include <..\..\PGSuper\Include\Hints.h>
#include <IFace\Alignment.h>
#include <MFCTools\Format.h>

#include <EAF\EAFDisplayUnits.h>
#include <EAF\Menu.h>

#include "XBRateCalculationSheet.h"

#include <PsgLib\BridgeDescription2.h>

#include <XBeamRateExt\XBeamRateUtilities.h>

#include <WBFLGenericBridge.h>
#include <WBFLGeometry/GeomHelpers.h>

#include <PGSuperColors.h>
#define COLUMN_LINE_COLOR              GREY50
#define COLUMN_FILL_COLOR              GREY70
#define XBEAM_LINE_COLOR               GREY50
#define XBEAM_FILL_COLOR               GREY70

#define COLUMN_LINE_WEIGHT             1
//#define XBEAM_LINE_WEIGHT              3
#define REBAR_LINE_WEIGHT              1

#define ROADWAY_DISPLAY_LIST_ID        0
#define GIRDER_DISPLAY_LIST_ID         1
#define BEARING_DISPLAY_LIST_ID        2
#define XBEAM_DISPLAY_LIST_ID          3
#define COLUMN_DISPLAY_LIST_ID         4
#define DIMENSIONS_DISPLAY_LIST_ID     5
#define REBAR_DISPLAY_LIST_ID          6
#define STIRRUP_DISPLAY_LIST_ID        7
#define SECTION_CUT_DISPLAY_LIST_ID    8

#define STAGE1_STIRRUP_COLOR    STIRRUP_COLOR
#define STAGE2_STIRRUP_COLOR    OLIVEDRAB


#define SECTION_CUT_ID                500

// The End/Section view of the pier is offset from the Elevation
// view by this amount.
const Float64 EndOffset = WBFL::Units::ConvertToSysUnits(10,WBFL::Units::Measure::Feet);

const WBFL::DManip::SelectionType g_selectionType = WBFL::DManip::SelectionType::None; // nothing is selectable, except for the section cut object

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView

IMPLEMENT_DYNCREATE(CXBeamRateView, CDisplayView)

BEGIN_MESSAGE_MAP(CXBeamRateView, CDisplayView)
	//{{AFX_MSG_MAP(CXBeamRateView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
   ON_COMMAND(IDM_EXPORT_PIER, OnExportPier)
   ON_COMMAND(IDM_EDIT_PIER, OnEditPier)
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView construction/destruction

CXBeamRateView::CXBeamRateView()
{
	// TODO: add construction code here
   m_DisplayObjectID = 0;
}

CXBeamRateView::~CXBeamRateView()
{
}

BOOL CXBeamRateView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CDisplayView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView printing
void CXBeamRateView::OnDraw(CDC* pDC)
{
   if ( m_bIsIdealized )
   {
      pDC->TextOut(0,0,_T("Pier is idealized"));
   }
   else
   {
      CDisplayView::OnDraw(pDC);
   }
}

BOOL CXBeamRateView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CXBeamRateView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
   // get paper size
   auto pBroker = EAFGetBroker();

   XBRateCalculationSheet border;

   CRect rcPrint = border.Print(pDC, 1);

   // want to offset picture away from borders - get device units for 10mm
   int oldmode = pDC->SetMapMode(MM_LOMETRIC);
   POINT offset[2] = { {0,0}, {100,-100}};
   pDC->LPtoDP(offset,2);
   int offsetx = offset[1].x - offset[0].x;
   int offsety = offset[1].y - offset[0].y;
   rcPrint.DeflateRect(offsetx,offsety);
   pDC->SetMapMode(oldmode);

   if (rcPrint.IsRectEmpty())
   {
      CHECKX(0,_T("Can't print border - page too small?"));
      rcPrint = pInfo->m_rectDraw;
   }

   CDisplayView::OnBeginPrinting(pDC, pInfo, rcPrint);
   OnPrepareDC(pDC);
   ScaleToFit();
   OnDraw(pDC);
   OnEndPrinting(pDC, pInfo);
}

void CXBeamRateView::OnSize(UINT nType, int cx, int cy) 
{
	CDisplayView::OnSize(nType, cx, cy);

   CRect rect;
   GetClientRect(&rect);
   rect.DeflateRect(15,15,15,15);

   CSize size = rect.Size();
   size.cx = Max(0L,size.cx);
   size.cy = Max(0L,size.cy);

   SetLogicalViewRect(MM_TEXT,rect);

   SetScrollSizes(MM_TEXT,size,CScrollView::sizeDefault,CScrollView::sizeDefault);

   ScaleToFit();
}

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView diagnostics

#ifdef _DEBUG
void CXBeamRateView::AssertValid() const
{
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//CDisplayView::AssertValid();
}

void CXBeamRateView::Dump(CDumpContext& dc) const
{
	CDisplayView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView message handlers
int CXBeamRateView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if ( CDisplayView::OnCreate(lpCreateStruct) == -1 )
   {
      return -1;
   }

   EnableToolTips();


   auto doFactory = std::make_shared<CDisplayObjectFactory>();
   m_pDispMgr->AddDisplayObjectFactory(doFactory);

   m_pDispMgr->EnableLBtnSelect(TRUE);
   m_pDispMgr->EnableRBtnSelect(TRUE);
   m_pDispMgr->SetSelectionLineColor(SELECTED_OBJECT_LINE_COLOR);
   m_pDispMgr->SetSelectionFillColor(SELECTED_OBJECT_FILL_COLOR);

   CDisplayView::SetMappingMode(WBFL::DManip::MapMode::Isotropic);

   // Setup display lists
   m_pDispMgr->CreateDisplayList(SECTION_CUT_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(DIMENSIONS_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(ROADWAY_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(REBAR_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(STIRRUP_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(BEARING_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(GIRDER_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(XBEAM_DISPLAY_LIST_ID);
   m_pDispMgr->CreateDisplayList(COLUMN_DISPLAY_LIST_ID);

   m_pFrame = (CXBeamRateChildFrame*)GetParent();
   ASSERT( m_pFrame != nullptr );
   ASSERT( m_pFrame->IsKindOf( RUNTIME_CLASS( CXBeamRateChildFrame ) ) );

   return 0;
}

void CXBeamRateView::OnInitialUpdate()
{
   CDisplayView::OnInitialUpdate();
}

PierIDType CXBeamRateView::GetPierID()
{
   AFX_MANAGE_STATE(AfxGetAppModuleState());
   CXBeamRateChildFrame* pFrame = (CXBeamRateChildFrame*)GetParentFrame();
   return pFrame->GetPierID();
}

PierIndexType CXBeamRateView::GetPierIndex()
{
   AFX_MANAGE_STATE(AfxGetAppModuleState());
   CXBeamRateChildFrame* pFrame = (CXBeamRateChildFrame*)GetParentFrame();
   return pFrame->GetPierIndex();
}

xbrPointOfInterest CXBeamRateView::GetCutLocation()
{
   auto pDL = m_pDispMgr->FindDisplayList(SECTION_CUT_DISPLAY_LIST_ID);
   ATLASSERT(pDL);

   auto dispObj = pDL->FindDisplayObject(SECTION_CUT_ID);

   if ( dispObj == nullptr )
   {
      return xbrPointOfInterest();
   }

   auto sink = dispObj->GetEventSink();
   auto sectionCutStrategy = std::dynamic_pointer_cast<iSectionCutDrawStrategy>(sink);

   return sectionCutStrategy->GetCutPOI(m_pFrame->GetCurrentCutLocation());
}

void CXBeamRateView::OnUpdate(CView* pSender,LPARAM lHint,CObject* pHint)
{
   if (IsPGSExtension())
   {
      if (lHint == HINT_SELECTIONCHANGED)
      {
         // nothing in this view is related to the current selection
         return;
      }

      if (lHint == HINT_BRIDGECHANGED)
      {
         AFX_MANAGE_STATE(AfxGetAppModuleState());
         CXBeamRateChildFrame* pFrame = (CXBeamRateChildFrame*)GetParentFrame();
         pFrame->UpdatePierList();
      }
   }

   PierIDType pierID = GetPierID();
   if (pierID != INVALID_ID)
   {
      auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker, IBridgeDescription, pIBridgeDesc);
      const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
      const CPierData2* pPier = pBridgeDesc->FindPier(pierID);
      if (pPier == nullptr)
      {
         CDC* pDC = GetDC();
         pDC->TextOut(0, 0, _T("Invalid Pier"));
         return;
      }
   }

   m_pFrame->UpdateSectionCutExtents();

   CDisplayView::OnUpdate(pSender,lHint,pHint);

   CDManipClientDC dc2(this);
   UpdateDisplayObjects();
   ScaleToFit();
   Invalidate();
}

void CXBeamRateView::UpdateDisplayObjects()
{
   CWaitCursor wait;

   m_bIsIdealized = false;
   PierIndexType pierIdx = GetPierIndex();
   if ( pierIdx != INVALID_INDEX )
   {
      auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);
      const CPierData2* pPier = pIBridgeDesc->GetPier(pierIdx);
      if ( pPier->GetPierModelType() == pgsTypes::pmtIdealized )
      {
         // there is nothing to draw when the pier is idealized
         m_bIsIdealized = true;
         return;
      }
   }


   // Capture the current selection before blasting all the 
   // display objects
   auto vCurSel = m_pDispMgr->GetSelectedObjects();
   ATLASSERT(vCurSel.size() < 2);
   IDType curSel = INVALID_ID;
   IDType listID = INVALID_ID;
   if ( vCurSel.size() == 1 )
   {
      auto pDO = vCurSel[0];
      curSel = pDO->GetID();
      auto pDL = pDO->GetDisplayList();
      listID = pDL->GetID();
   }

   m_pDispMgr->ClearDisplayObjects();
   m_DisplayObjectID = 0;

   UpdateRoadwayDisplayObjects();
   UpdateXBeamDisplayObjects();
   UpdateColumnDisplayObjects();
   UpdateBearingDisplayObjects();
   UpdateGirderDisplayObjects();
   UpdateRebarDisplayObjects();
   UpdateStirrupDisplayObjects();
   UpdateDimensionsDisplayObjects();
   UpdateSectionCutDisplayObjects();

   // Re-instate the current selection
   if ( curSel != INVALID_ID )
   {
      auto doSel = m_pDispMgr->FindDisplayObject(curSel,listID,WBFL::DManip::AccessType::ByID);
      m_pDispMgr->SelectObject(doSel,TRUE);
   }
}

void CXBeamRateView::UpdateRoadwayDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(ROADWAY_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRPier,pPier);
   Float64 skew = pPier->GetSkewAngle(pierID);

   Float64 cos_skew = cos(skew);

   GET_IFACE2(pBroker,IXBRProject,pProject);

   // Model a vertical line for the alignment
   // The alignment is at X = 0 in Pier coordinates
   Float64 X = 0;
   Float64 Xcl = pPier->ConvertPierToCurbLineCoordinate(pierID,X);
   Float64 Ydeck = pPier->GetElevation(pierID,Xcl); // deck elevation at alignment
   Float64 Yt = Ydeck + WBFL::Units::ConvertToSysUnits(1.0,WBFL::Units::Measure::Feet); // add a little so it projects over the roadway surface
   WBFL::Geometry::Point2d pnt1(X, Yt);

   Float64 Yb = Yt - pPier->GetMaxColumnHeight(pierID);
   WBFL::Geometry::Point2d pnt2(X, Yb);

   auto doAlignment = CreateLineDisplayObject(pnt1,pnt2);
   auto drawStrategy = doAlignment->GetDrawLineStrategy();
   auto drawAlignmentStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawLineStrategy>(drawStrategy);
   drawAlignmentStrategy->SetWidth(ALIGNMENT_LINE_WEIGHT);
   drawAlignmentStrategy->SetColor(ALIGNMENT_COLOR);
   drawAlignmentStrategy->SetLineStyle(WBFL::DManip::LineStyleType::Centerline);

   // Don't add the object to the display list here... do it at the end
   // We want it to be drawn on top so it has to go into the display list last
   //displayList->AddDisplayObject(doAlignment);

   // Draw the bridge line if different then the alignment
   std::shared_ptr<WBFL::DManip::iLineDisplayObject> doBridgeLine;
   Float64 BLO = pProject->GetBridgeLineOffset(pierID);
   if ( !IsZero(BLO) )
   {
      // Model a vertical line for the bridge line
      // Let X = BLO be at the alignment and Y = the alignment elevation
      Float64 X = BLO/cos_skew;
      pnt1.Move(X,Yt);
      pnt2.Move(X,Yb);

      doBridgeLine = CreateLineDisplayObject(pnt1,pnt2);
      auto drawStrategy = doBridgeLine->GetDrawLineStrategy();
      auto drawBridgeLineStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawLineStrategy>(drawStrategy);
      drawBridgeLineStrategy->SetWidth(BRIDGELINE_LINE_WEIGHT);
      drawBridgeLineStrategy->SetColor(BRIDGE_COLOR);
      drawBridgeLineStrategy->SetLineStyle(WBFL::DManip::LineStyleType::Centerline);

      //displayList->AddDisplayObject(doBridgeLine); // do this at the end
   }

   // Draw Roadway Surface
   if ( IsStandAlone() )
   {
      // Stand-alone... we don't have an actual deck modeled, so
      // just show the curb-to-curb extents of the roadway surface
      auto doDeck = WBFL::DManip::PolyLineDisplayObject::Create(m_DisplayObjectID++);

      Float64 LCO, RCO;
      pProject->GetCurbLineOffset(pierID,&LCO,&RCO);
      if (pProject->GetCurbLineDatum(pierID) == pgsTypes::omtBridge)
      {
         LCO += BLO;
         RCO += BLO;
      }

      Float64 Ylc = pPier->GetElevation(pierID,0);
      LCO /= cos_skew; // skew adjust
      pnt1.Move(LCO,Ylc);
      doDeck->AddPoint(pnt1);

      Float64 CPO = pProject->GetCrownPointOffset(pierID);
      if (LCO <= CPO && CPO <= RCO)
      {
         // add crown point if it is between the left and right curblines
         pnt2.Move(CPO, Ydeck);
         doDeck->AddPoint(pnt2);
      }

      Float64 Yrc = pPier->GetElevation(pierID, (RCO - LCO) / cos_skew);
      RCO /= cos_skew; // skew adjust
      WBFL::Geometry::Point2d pnt3(RCO,Yrc);
      doDeck->AddPoint(pnt3);

      doDeck->SetPointType(WBFL::DManip::PointType::None);
      doDeck->SetColor(PROFILE_COLOR);
      doDeck->SetWidth(PROFILE_LINE_WEIGHT);

      displayList->AddDisplayObject(doDeck);
   }
   else
   {
      // Part of PGSuper/PGSplice... we can get the actual superstructure shapes

      // Deck
      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);
      const CBridgeDescription2* pBridgeDesc = pIBridgeDesc->GetBridgeDescription();
      const CDeckDescription2* pDeck = pBridgeDesc->GetDeckDescription();

      PierIndexType pierIdx = GetPierIndex();
      const CPierData2* pPier = pBridgeDesc->GetPier(pierIdx);
      Float64 pierStation = pPier->GetStation();

      GET_IFACE2(pBroker,IShapes,pShapes);
      GET_IFACE2(pBroker,IBridge,pBridge);

      CComPtr<IDirection> pierDirection;
      pBridge->GetPierDirection(pierIdx,&pierDirection);

      pgsTypes::SupportedDeckType deckType = pDeck->GetDeckType();
      if ( deckType != pgsTypes::sdtNone )
      {
         auto dispObj = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);

         CComPtr<IShape> shape;
         pShapes->GetSlabShape(pierStation,pierDirection,true/*include haunch*/,&shape);

         auto strategy = WBFL::DManip::ShapeDrawStrategy::Create();

         strategy->SetShape(geomUtil::ConvertShape(shape));
         strategy->SetSolidLineColor(IsStructuralDeck(deckType) ? DECK_BORDER_COLOR : NONSTRUCTURAL_DECK_BORDER_COLOR);
         strategy->SetSolidFillColor(IsStructuralDeck(deckType) ? DECK_FILL_COLOR : NONSTRUCTURAL_DECK_FILL_COLOR);
         strategy->SetVoidLineColor(VOID_BORDER_COLOR);
         strategy->SetVoidFillColor(GetSysColor(COLOR_WINDOW));
         strategy->Fill(true);

         dispObj->SetDrawingStrategy(strategy);

         auto gravity_well = WBFL::DManip::ShapeGravityWellStrategy::Create();
         gravity_well->SetShape(geomUtil::ConvertShape(shape));

         dispObj->SetGravityWellStrategy(gravity_well);

         dispObj->SetSelectionType(g_selectionType);

         displayList->AddDisplayObject(dispObj);
      }

      // Left Hand Barrier
      auto left_dispObj = WBFL::DManip::PointDisplayObject::Create();

      Float64 left_curb_offset  = pBridge->GetLeftCurbOffset(pierIdx);
      Float64 right_curb_offset = pBridge->GetRightCurbOffset(pierIdx);

      CComPtr<IShape> left_shape;
      pShapes->GetLeftTrafficBarrierShape(pierStation,pierDirection,&left_shape);

      if ( left_shape )
      {
         auto strategy = WBFL::DManip::ShapeDrawStrategy::Create();
         strategy->SetShape(geomUtil::ConvertShape(left_shape));
         strategy->SetSolidLineColor(BARRIER_BORDER_COLOR);
         strategy->SetSolidFillColor(BARRIER_FILL_COLOR);
         strategy->SetVoidLineColor(VOID_BORDER_COLOR);
         strategy->SetVoidFillColor(GetSysColor(COLOR_WINDOW));
         strategy->Fill(true);
         strategy->HasBoundingShape(false);

         left_dispObj->SetDrawingStrategy(strategy);

         displayList->AddDisplayObject(left_dispObj);
      }

      // Right Hand Barrier
      auto right_dispObj = WBFL::DManip::PointDisplayObject::Create();

      CComPtr<IShape> right_shape;
      pShapes->GetRightTrafficBarrierShape(pierStation,pierDirection,&right_shape);

      if ( right_shape )
      {
         auto strategy = WBFL::DManip::ShapeDrawStrategy::Create();
         strategy->SetShape(geomUtil::ConvertShape(right_shape));
         strategy->SetSolidLineColor(BARRIER_BORDER_COLOR);
         strategy->SetSolidFillColor(BARRIER_FILL_COLOR);
         strategy->SetVoidLineColor(VOID_BORDER_COLOR);
         strategy->SetVoidFillColor(GetSysColor(COLOR_WINDOW));
         strategy->Fill(true);
         strategy->HasBoundingShape(false);

         right_dispObj->SetDrawingStrategy(strategy);

         displayList->AddDisplayObject(right_dispObj);
      }
   }
   
   displayList->AddDisplayObject(doAlignment);
   if ( doBridgeLine )
   {
      displayList->AddDisplayObject(doBridgeLine);
   }
}

void CXBeamRateView::UpdateXBeamDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(XBEAM_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRProject,pProject);
   GET_IFACE2(pBroker,IXBRPier,pPier);
   GET_IFACE2(pBroker,IXBRSectionProperties,pSectProp);

   // Model Upper Cross Beam (Elevation)
   WBFL::Geometry::Point2d point(0, 0);

   if ( pProject->GetPierType(pierID) != xbrTypes::pctExpansion )
   {
      auto doUpperXBeam = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
      doUpperXBeam->SetPosition(point,false,false);
      doUpperXBeam->SetSelectionType(g_selectionType);

      CComPtr<IShape> upperXBeamShape;
      pPier->GetUpperXBeamProfile(pierID,&upperXBeamShape);

      auto upperXBeamDrawStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
      upperXBeamDrawStrategy->SetShape(geomUtil::ConvertShape(upperXBeamShape));
      upperXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
      upperXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
      upperXBeamDrawStrategy->Fill(true);

      doUpperXBeam->SetDrawingStrategy(upperXBeamDrawStrategy);

      auto upper_xbeam_gravity_well = WBFL::DManip::ShapeGravityWellStrategy::Create();
      upper_xbeam_gravity_well->SetShape(geomUtil::ConvertShape(upperXBeamShape));
      doUpperXBeam->SetGravityWellStrategy(upper_xbeam_gravity_well);

      displayList->AddDisplayObject(doUpperXBeam);
   }

   // Model Lower Cross Beam (Elevation)
   auto doLowerXBeam = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
   doLowerXBeam->SetPosition(point,false,false);
   doLowerXBeam->SetSelectionType(g_selectionType);

   CComPtr<IShape> lowerXBeamShape;
   pPier->GetLowerXBeamProfile(pierID,&lowerXBeamShape);

   auto lowerXBeamDrawStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
   lowerXBeamDrawStrategy->SetShape(geomUtil::ConvertShape(lowerXBeamShape));
   lowerXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   lowerXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   lowerXBeamDrawStrategy->Fill(true);

   doLowerXBeam->SetDrawingStrategy(lowerXBeamDrawStrategy);

   auto lower_xbeam_gravity_well = WBFL::DManip::ShapeGravityWellStrategy::Create();
   lower_xbeam_gravity_well->SetShape(geomUtil::ConvertShape(lowerXBeamShape));
   doLowerXBeam->SetGravityWellStrategy(lower_xbeam_gravity_well);

   displayList->AddDisplayObject(doLowerXBeam);

   // Section Cut
   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblTopLowerXBeam, pierID);
   Lxb = pPier->ConvertCrossBeamToPierCoordinate(pierID,Lxb);

   Float64 XxbCut = pPier->ConvertPierToCrossBeamCoordinate(pierID,m_pFrame->GetCurrentCutLocation());

   auto doXBeamSection = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
   doXBeamSection->SetPosition(point,false,false);
   doXBeamSection->SetSelectionType(g_selectionType);

   CComPtr<IShape> xbeamShape;
   pSectProp->GetXBeamShape(pierID,xbrPointOfInterest(INVALID_ID,XxbCut),&xbeamShape);
   CComQIPtr<IXYPosition> position(xbeamShape);
   position->Offset(EndOffset+Lxb,0);

   auto xbeamDrawStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
   xbeamDrawStrategy->SetShape(geomUtil::ConvertShape(xbeamShape));
   xbeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   xbeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   xbeamDrawStrategy->Fill(true);

   doXBeamSection->SetDrawingStrategy(xbeamDrawStrategy);

   auto xbeam_section_gravity_well = WBFL::DManip::ShapeGravityWellStrategy::Create();
   xbeam_section_gravity_well->SetShape(geomUtil::ConvertShape(xbeamShape));
   doXBeamSection->SetGravityWellStrategy(xbeam_section_gravity_well);

   displayList->AddDisplayObject(doXBeamSection);

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   CString strSectionCutLabel;
   strSectionCutLabel.Format(_T("Section @ %s"),::FormatDimension(XxbCut,pDisplayUnits->GetSpanLengthUnit()));

   CComPtr<IPoint2d> pntBC;
   position->get_LocatorPoint(lpBottomCenter,&pntBC);
   pntBC->Offset(0,-WBFL::Units::ConvertToSysUnits(3.0,WBFL::Units::Measure::Feet));

   auto doLabel = WBFL::DManip::TextBlock::Create();
   doLabel->SetText(strSectionCutLabel);
   doLabel->SetBkMode(TRANSPARENT);
   doLabel->SetTextAlign(TA_TOP | TA_CENTER);
   doLabel->SetPosition(geomUtil::GetPoint(pntBC));
   displayList->AddDisplayObject(doLabel);
}

void CXBeamRateView::UpdateColumnDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(COLUMN_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBRPier,pPier);
   GET_IFACE2(pBroker,IXBRProject,pProject);

   PierIDType pierID = GetPierID();

   // Create a function that represents the bottom of the cross beam
   // We will use it to make the top of the column match the bottom of the
   // cross beam.
   WBFL::Math::PiecewiseFunction fn;
   CComPtr<IPoint2dCollection> points;
   pPier->GetBottomSurface(pierID,xbrTypes::Stage1,&points);
   CComPtr<IEnumPoint2d> enumPoints;
   points->get__Enum(&enumPoints);
   CComPtr<IPoint2d> pnt;
   while ( enumPoints->Next(1,&pnt,nullptr) != S_FALSE )
   {
      Float64 x,y;
      pnt->Location(&x,&y);
      fn.AddPoint(x,y);
      pnt.Release();
   }

   IndexType nColumns = pPier->GetColumnCount(pierID);
   for (IndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      Float64 XxbCol = pPier->GetColumnLocation(pierID,colIdx);
      Float64 XpCol  = pPier->ConvertCrossBeamToPierCoordinate(pierID,XxbCol);
      Float64 Ytop = pPier->GetTopColumnElevation(pierID,colIdx);
      Float64 Ybot = pPier->GetBottomColumnElevation(pierID,colIdx);
      CColumnData::ColumnShapeType colShapeType;
      Float64 d1, d2;
      CColumnData::ColumnHeightMeasurementType columnHeightType;
      Float64 H;
      pProject->GetColumnProperties(pierID,colIdx,&colShapeType,&d1,&d2,&columnHeightType,&H);

      WBFL::Geometry::Point2d pntTop(XpCol,Ytop);
      WBFL::Geometry::Point2d pntBot(XpCol,Ybot);
   
      auto doTop = WBFL::DManip::PointDisplayObject::Create();
      doTop->Visible(false);
      doTop->SetPosition(pntTop,false,false);
      auto connectable1 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doTop);
      auto socket1 = connectable1->AddSocket(0,pntTop);

      auto doBot = WBFL::DManip::PointDisplayObject::Create();
      doBot->Visible(false);
      doBot->SetPosition(pntBot,false,false);
      auto connectable2 =  std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doBot);
      auto socket2 = connectable2->AddSocket(0,pntBot);

      // Create the shape of the column
      auto columnShape = std::make_shared<WBFL::Geometry::Polygon>();
      Float64 X1,X2,X3;
      X2 = pntTop.X();
      X1 = X2-d1/2;
      X3 = X2+d1/2;
      Float64 Y1 = fn.Evaluate(X1);
      Float64 Y2 = fn.Evaluate(X2);
      Float64 Y3 = fn.Evaluate(X3);

      columnShape->AddPoint(X1,Y1);
      columnShape->AddPoint(X2,Y2);
      columnShape->AddPoint(X3,Y3);
      columnShape->AddPoint(X3,Ybot);
      columnShape->AddPoint(X1,Ybot);

      auto doColumn = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
      doColumn->SetPosition(pntTop,false,false);
      doColumn->SetSelectionType(g_selectionType);

      auto drawColumnStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
      doColumn->SetDrawingStrategy(drawColumnStrategy);

      drawColumnStrategy->SetShape(columnShape);
      drawColumnStrategy->SetSolidLineColor(COLUMN_LINE_COLOR);
      drawColumnStrategy->SetSolidLineWidth(COLUMN_LINE_WEIGHT);
      drawColumnStrategy->SetSolidFillColor(COLUMN_FILL_COLOR);
      drawColumnStrategy->Fill(true);

      displayList->AddDisplayObject(doColumn);
   }
}

void CXBeamRateView::UpdateRebarDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(REBAR_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBRPier,pPier);

   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRRebar,pRebar);

   // Elevation View
   IndexType nRebarRows = pRebar->GetRebarRowCount(pierID);
   for ( IndexType rowIdx = 0; rowIdx < nRebarRows; rowIdx++ )
   {
      CComPtr<IPoint2dCollection> points;
      pRebar->GetRebarProfile(pierID,rowIdx,&points); // in Cross Beam Coordinates

      auto doRebar = WBFL::DManip::PolyLineDisplayObject::Create(m_DisplayObjectID++);
      doRebar->AddPoints(geomUtil::CreatePointCollection(points));
      doRebar->SetPointType(WBFL::DManip::PointType::None);
      doRebar->SetColor(REBAR_COLOR);
      doRebar->SetWidth(REBAR_LINE_WEIGHT);
      displayList->AddDisplayObject(doRebar);
   }

   // Section View
   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblTopLowerXBeam, pierID);
   Lxb = pPier->ConvertCrossBeamToPierCoordinate(pierID,Lxb);

   Float64 XxbCut = pPier->ConvertPierToCrossBeamCoordinate(pierID,m_pFrame->GetCurrentCutLocation());

   CComPtr<IRebarSection> rebarSection;
   pRebar->GetRebarSection(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,XxbCut),&rebarSection);

   CComPtr<IEnumRebarSectionItem> enumSectionItems;
   rebarSection->get__EnumRebarSectionItem(&enumSectionItems);
   CComPtr<IRebarSectionItem> sectionItem;
   while ( enumSectionItems->Next(1,&sectionItem,nullptr) != S_FALSE )
   {
      CComPtr<IPoint2d> pntBar;
      sectionItem->get_Location(&pntBar);

      pntBar->Offset(EndOffset+Lxb,0);

      auto doBar = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
      doBar->SetPosition(geomUtil::GetPoint(pntBar),false,false);

      displayList->AddDisplayObject(doBar);

      sectionItem.Release();
   }
}

void CXBeamRateView::UpdateStirrupDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(STIRRUP_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   GET_IFACE2_NOCHECK(pBroker,IXBRPier,pPier);
   GET_IFACE2_NOCHECK(pBroker,IXBRSectionProperties,pSectProps);

   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRProject,pProject);
   Float64 H,W;
   pProject->GetDiaphragmDimensions(pierID,&H,&W);

   Float64 tDeck = pProject->GetDeckThickness(pierID);

   xbrTypes::PierType pierType = pProject->GetPierType(pierID);

   GET_IFACE2(pBroker,IXBRStirrups,pStirrups);
   // work backwards when drawing stirrups so lower XBeam stirrups are 
   // drawn second and not covered up by the full depth stirrups
   for ( int i = 1; 0 <= i; i-- ) 
   {
      xbrTypes::Stage stage = (xbrTypes::Stage)i;
      COLORREF color = (stage == xbrTypes::Stage1 ? STAGE1_STIRRUP_COLOR : STAGE2_STIRRUP_COLOR);
      IndexType nStirrupZones = pStirrups->GetStirrupZoneCount(pierID,stage);
      for ( IndexType zoneIdx = 0; zoneIdx < nStirrupZones; zoneIdx++ )
      {
         Float64 Xstart, Xend;
         pStirrups->GetStirrupZoneBoundary(pierID,stage,zoneIdx,&Xstart,&Xend);
         
         Float64 nLegs = pStirrups->GetStirrupLegCount(pierID,stage,zoneIdx);

         if ( nLegs == 0 )
         {
            continue;
         }

         IndexType nStirrups = pStirrups->GetStirrupCount(pierID,stage,zoneIdx);
         Float64 Lzone = pStirrups->GetStirrupZoneLength(pierID, stage, zoneIdx);
         Float64 S = pStirrups->GetStirrupZoneSpacing(pierID, stage, zoneIdx);
         if (1 < nStirrups)
         {
            S = Lzone / (nStirrups - 1); // the zones and the spacing don't usually line up... compute a nominal spacing for display purposes
         }

         for ( IndexType stirrupIdx = 0; stirrupIdx < nStirrups; stirrupIdx++ )
         {
            Float64 Xxb = Xstart + stirrupIdx*S;
            Float64 Xcl = pPier->ConvertCrossBeamToCurbLineCoordinate(pierID,Xxb);

            Float64 Ytop = pPier->GetElevation(pierID,Xcl);

            Ytop -= tDeck;

            Float64 D = pSectProps->GetDepth(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,Xxb));
            Float64 Ybot = Ytop - D;

            if ( pierType != xbrTypes::pctIntegral )
            {
               // for non-integral, D is just the depth of the lower portion
               Ybot -= H;
            }

            if ( stage == xbrTypes::Stage1 || pierType != xbrTypes::pctIntegral )
            {
               Ytop -= H;
            }

            Float64 X = pPier->ConvertCrossBeamToPierCoordinate(pierID,Xxb);
            WBFL::Geometry::Point2d pntTop(X,Ytop);

            auto doTopPnt = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
            doTopPnt->SetPosition(pntTop,false,false);

            auto startConnectable = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doTopPnt);
            auto startSocket = startConnectable->AddSocket(0,pntTop);

            auto draw_point_strategy = doTopPnt->GetDrawingStrategy();
            auto thePointDrawStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawPointStrategy>(draw_point_strategy);
            thePointDrawStrategy->SetColor(color);
            thePointDrawStrategy->SetPointType(WBFL::DManip::PointType::None);

            WBFL::Geometry::Point2d pntBottom(X,Ybot);

            auto doBottomPnt = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
            doBottomPnt->SetPosition(pntBottom,false,false);

            auto endConnectable = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doBottomPnt);
            auto endSocket = endConnectable->AddSocket(0,pntBottom);

            draw_point_strategy = doBottomPnt->GetDrawingStrategy();
            thePointDrawStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawPointStrategy>(draw_point_strategy);
            thePointDrawStrategy->SetColor(STIRRUP_COLOR);
            thePointDrawStrategy->SetPointType(WBFL::DManip::PointType::None);

            auto doLine = WBFL::DManip::LineDisplayObject::Create(m_DisplayObjectID++);
            auto connector = std::dynamic_pointer_cast<WBFL::DManip::iConnector>(doLine);
            auto startPlug = connector->GetStartPlug();
            auto endPlug = connector->GetEndPlug();
            startPlug->SetSocket(startSocket);
            endPlug->SetSocket(endSocket);

            auto strategy = doLine->GetDrawLineStrategy();
            auto theStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawLineStrategy>(strategy);
            theStrategy->SetColor(color);
            theStrategy->SetEndType(WBFL::DManip::PointType::None);
            theStrategy->SetLineStyle(WBFL::DManip::LineStyleType::Solid);
            theStrategy->SetWidth((UINT)nLegs/2); // the more legs, the thicker the stirrup line

            displayList->AddDisplayObject(doTopPnt);
            displayList->AddDisplayObject(doBottomPnt);
            displayList->AddDisplayObject(doLine);
         }
      }
   }
}

void CXBeamRateView::UpdateBearingDisplayObjects()
{
   if ( !IsStandAlone() )
   {
      // don't show "dots" for bearing location if the girders are drawn
      return;
   }

   auto displayList = m_pDispMgr->FindDisplayList(BEARING_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRPier,pPier);

   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);

   IndexType nBearingLines = pPier->GetBearingLineCount(pierID);
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      IndexType nBearings = pPier->GetBearingCount(pierID,brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         Float64 Xbrg = pPier->GetBearingLocation(pierID,brgLineIdx,brgIdx);
         Float64 Xp   = pPier->ConvertCrossBeamToPierCoordinate(pierID,Xbrg);
         Float64 Y    = pPier->GetBearingElevation(pierID,brgLineIdx,brgIdx);

         WBFL::Geometry::Point2d pnt(Xp,Y);

         auto doPnt = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
         doPnt->SetPosition(pnt,false,false);

         auto strategy = doPnt->GetDrawingStrategy();

         auto theStrategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawPointStrategy>(strategy);
         theStrategy->SetPointType(WBFL::DManip::PointType::Square);
         theStrategy->SetColor(Xbrg < 0 || Lxb < Xbrg ? RED : BLACK); // use red if bearing is off the cross beam

         displayList->AddDisplayObject(doPnt);
      }
   }
}

void CXBeamRateView::UpdateGirderDisplayObjects()
{
   if ( IsStandAlone() )
   {
      // not in PGSuper/PGSplice so we don't know anything about the shape of the girder
      return;
   }

   auto displayList = m_pDispMgr->FindDisplayList(GIRDER_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   PierIndexType pierIdx = GetPierIndex();
   GET_IFACE2(pBroker,IBridge,pBridge);
   GroupIndexType backGroupIdx,aheadGroupIdx;
   pBridge->GetGirderGroupIndex(pierIdx,&backGroupIdx,&aheadGroupIdx);

   // deal with first and last pier
   PierIndexType nPiers = pBridge->GetPierCount();
   if ( pierIdx == 0 )
   {
      backGroupIdx = aheadGroupIdx;
   }
   else if ( pierIdx == nPiers-1 )
   {
      aheadGroupIdx = backGroupIdx;
   }

   GET_IFACE2(pBroker,IShapes,pShapes);
   GET_IFACE2(pBroker,IPointOfInterest,pPoi);
   GET_IFACE2(pBroker,IIntervals,pIntervals);
   GET_IFACE2(pBroker,IGirder,pGirder);

   CComPtr<IDirection> pierDirection;
   pBridge->GetPierDirection(pierIdx,&pierDirection);

   CComPtr<IDirection> pierNormal;
   pierDirection->Increment(CComVariant(-PI_OVER_2),&pierNormal);

   Float64 dirPierNormal;
   pierNormal->get_Value(&dirPierNormal);

   for ( GroupIndexType grpIdx = aheadGroupIdx; backGroupIdx <= grpIdx && grpIdx != INVALID_INDEX; grpIdx-- ) // draw in reverse order so back side girders cover ahead side girders
   {
      GirderIndexType nGirders = pBridge->GetGirderCount(grpIdx);
      for ( GirderIndexType gdrIdx = 0; gdrIdx < nGirders; gdrIdx++ )
      {
         CGirderKey girderKey(grpIdx,gdrIdx);
         pgsPointOfInterest poi = pPoi->GetPierPointOfInterest(girderKey,pierIdx);
         const CSegmentKey& segmentKey = poi.GetSegmentKey();
         IntervalIndexType intervalIdx = pIntervals->GetErectSegmentInterval(segmentKey);

         pgsPointOfInterest gdrPoi;

         if (pBridge->IsInteriorPier(pierIdx) && IsSegmentContinuousOverPier(pBridge->GetPierSegmentConnectionType(pierIdx)))
         {
            gdrPoi = poi;
         }
         else
         {
            if (grpIdx == backGroupIdx && pierIdx != 0)
            {
               PoiList vPoi;
               pPoi->GetPointsOfInterest(segmentKey, POI_END_FACE, &vPoi);
               ATLASSERT(vPoi.size() == 1);
               gdrPoi = vPoi.front();
            }
            else
            {
               PoiList vPoi;
               pPoi->GetPointsOfInterest(segmentKey, POI_START_FACE, &vPoi);
               ATLASSERT(vPoi.size() == 1);
               gdrPoi = vPoi.front();
            }
         }

         CComPtr<IShape> shape;
         pShapes->GetSegmentShape(intervalIdx, gdrPoi,true,pgsTypes::scBridge,&shape); // this is the shape normal to the girder... it needs to be projected onto the viewing plane

         // compute skew angle of segment with respect to the viewing direction
         // (viewing direction is normal to pier)
         CComPtr<IDirection> segDirection;
         pGirder->GetSegmentDirection(segmentKey,&segDirection);

         Float64 dirSegment;
         segDirection->get_Value(&dirSegment);

         Float64 skew = dirPierNormal - dirSegment;
         ATLASSERT(-PI_OVER_2 <= skew && skew <= PI_OVER_2);

         // compute shear factor
         Float64 wTop = pGirder->GetTopWidth(gdrPoi);
         wTop /= cos(skew); // top width of the girder, measured normal to the viewing direction

         // Compute the shear factor
         Float64 shear = 0;

         // Adjust the shape of the girder for skew and shear
         CComPtr<IShape> skewedShape;
         SkewGirderShape(skew,shear,shape,&skewedShape);

         auto dispObj = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);

         CComQIPtr<IXYPosition> position(skewedShape);
         CComPtr<IPoint2d> topCenter;
         position->get_LocatorPoint(lpTopCenter,&topCenter);

         dispObj->SetPosition(geomUtil::GetPoint(topCenter),false,false);

         auto strategy = WBFL::DManip::ShapeDrawStrategy::Create();
         strategy->SetShape(geomUtil::ConvertShape(skewedShape));
         strategy->SetSolidLineColor(SEGMENT_BORDER_COLOR);
         strategy->SetVoidLineColor(VOID_BORDER_COLOR);
         strategy->SetVoidFillColor(GetSysColor(COLOR_WINDOW));
         if ( grpIdx == backGroupIdx )
         {
            strategy->SetSolidFillColor(SEGMENT_FILL_COLOR);
            strategy->SetSolidLineStyle(WBFL::DManip::LineStyleType::Solid);
            strategy->SetVoidLineStyle(WBFL::DManip::LineStyleType::Solid);
         }
         else
         {
            strategy->SetSolidFillColor(SEGMENT_FILL_GHOST_COLOR);
            strategy->SetSolidLineStyle(WBFL::DManip::LineStyleType::Dash);
            strategy->SetVoidLineStyle(WBFL::DManip::LineStyleType::Dash);
         }
         strategy->Fill(true);

         dispObj->SetDrawingStrategy(strategy);

         dispObj->SetSelectionType(g_selectionType);

         displayList->AddDisplayObject(dispObj);
      }
   }
}

void CXBeamRateView::UpdateSectionCutDisplayObjects()
{
   auto pBroker = EAFGetBroker();


   auto display_list = m_pDispMgr->FindDisplayList(SECTION_CUT_DISPLAY_LIST_ID);

   auto factory = m_pDispMgr->GetDisplayObjectFactory(0);

   auto disp_obj = factory->Create(CSectionCutDisplayImpl::ms_Format,nullptr);

   auto sink = disp_obj->GetEventSink();

   disp_obj->SetSelectionType(WBFL::DManip::SelectionType::All);

   auto point_disp = std::dynamic_pointer_cast<WBFL::DManip::iPointDisplayObject>(disp_obj);
   point_disp->SetMaxTipWidth(TOOLTIP_WIDTH);
   point_disp->SetToolTipText(_T("Drag me to move section cut.\r\nDouble click to enter the cut location\r\nPress CTRL + -> to move ahead\r\nPress CTRL + <- to move back"));
   point_disp->SetTipDisplayTime(TOOLTIP_DURATION);

   auto section_cut_strategy = std::dynamic_pointer_cast<iSectionCutDrawStrategy>(sink);
   section_cut_strategy->Init(m_pFrame,point_disp, m_pFrame);
   section_cut_strategy->SetColor(CUT_COLOR);

   point_disp->SetID(SECTION_CUT_ID);

   display_list->Clear();
   display_list->AddDisplayObject(disp_obj);
}

void CXBeamRateView::UpdateDimensionsDisplayObjects()
{
   auto displayList = m_pDispMgr->FindDisplayList(DIMENSIONS_DISPLAY_LIST_ID);

   auto pBroker = EAFGetBroker();


   PierIDType pierID = GetPierID();

   GET_IFACE2(pBroker,IXBRPier,pPier);
   GET_IFACE2(pBroker,IXBRSectionProperties,pSectProp);
   GET_IFACE2(pBroker,IXBRProject,pProject);

   Float64 tDeck = pProject->GetDeckThickness(pierID);

   xbrTypes::PierType pierType = pProject->GetPierType(pierID);

   Float64 CPO = pPier->GetCrownPointOffset(pierID);
   Float64 Xcrown = pPier->GetCrownPointLocation(pierID);
   Xcrown = pPier->ConvertCurbLineToCrossBeamCoordinate(pierID,Xcrown);
   Float64 Xoffset = CPO - Xcrown;

   Float64 H1, H2, H3, H4, X1, X2, X3, X4, W;
   pProject->GetLowerXBeamDimensions(pierID,&H1,&H2,&H3,&H4,&X1,&X2,&X3,&X4,&W);

   Float64 D, Hu;
   pProject->GetDiaphragmDimensions(pierID,&Hu,&D);

   CComPtr<IPoint2dCollection> topUpperXBeamProfile, topLowerXBeamProfile, bottomXBeamProfile;
   pPier->GetTopSurface(pierID, xbrTypes::Stage1, &topLowerXBeamProfile);
   pPier->GetTopSurface(pierID, xbrTypes::Stage2, &topUpperXBeamProfile);
   pPier->GetBottomSurface(pierID, xbrTypes::Stage1, &bottomXBeamProfile);

   // Upper Cross Beam - Top Left
   CComPtr<IPoint2d> pnt;
   topUpperXBeamProfile->get_Item(0, &pnt);
   WBFL::Geometry::Point2d uxbTL(geomUtil::GetPoint(pnt));

   // we want all vertical dimensions on the left side to be at this x-location
   Float64 Xl;
   pnt->get_X(&Xl);

   // Upper Cross Beam - Bottom Left (Lower Cross Beam - Top Left)
   pnt.Release();
   topLowerXBeamProfile->get_Item(0, &pnt);
   WBFL::Geometry::Point2d uxbBL(geomUtil::GetPoint(pnt));
   uxbBL.X() = Xl;

   // Lower Cross Beam - Bottom Left
   pnt.Release();
   bottomXBeamProfile->get_Item(0, &pnt);
   WBFL::Geometry::Point2d lxbBL(geomUtil::GetPoint(pnt));
   lxbBL.X() = Xl;

   // Upper Cross Beam - Top Right
   pnt.Release();
   IndexType nPoints;
   topUpperXBeamProfile->get_Count(&nPoints);
   topUpperXBeamProfile->get_Item(nPoints - 1, &pnt);
   WBFL::Geometry::Point2d uxbTR(geomUtil::GetPoint(pnt));

   // we want all vertical dimensions on the right side to be at this x-location
   Float64 Xr;
   pnt->get_X(&Xr);
   
   // Upper Cross Beam - Bottom Right (Lower Cross Beam - Top Right)
   pnt.Release();
   topLowerXBeamProfile->get_Count(&nPoints);
   topLowerXBeamProfile->get_Item(nPoints - 1, &pnt);
   WBFL::Geometry::Point2d uxbBR(geomUtil::GetPoint(pnt));
   uxbBR.X() = Xr;

   // Lower Cross Beam - Bottom Right
   pnt.Release();
   bottomXBeamProfile->get_Count(&nPoints);
   bottomXBeamProfile->get_Item(nPoints - 1, &pnt);
   WBFL::Geometry::Point2d lxbBR(geomUtil::GetPoint(pnt));
   lxbBR.X() = Xr;

   // Height of upper cross beam
   if (pierType != xbrTypes::pctExpansion)
   {
      BuildDimensionLine(displayList, uxbBL, uxbTL);
      BuildDimensionLine(displayList, uxbTR, uxbBR);
   }

   // Height of lower cross beam
   BuildDimensionLine(displayList, lxbBL, uxbBL); // H1 Dimension
   BuildDimensionLine(displayList, uxbBR, lxbBR); // H3 Dimension

   // Lower cross beam bottom taper, vertical dimensions
   WBFL::Geometry::Point2d lxbBLC, lxbBRC;
   lxbBLC.Move(lxbBL);
   lxbBLC.Offset(0, -H2);
   lxbBRC.Move(lxbBR);
   lxbBRC.Offset(0, -H4);

   BuildDimensionLine(displayList, lxbBLC, lxbBL); // H2 Dimension
   BuildDimensionLine(displayList, lxbBR, lxbBRC); // H4 Dimension

   // Lower cross beam bottom taper, horizontal dimensions
   pnt.Release();
   topLowerXBeamProfile->get_Item(0, &pnt);
   WBFL::Geometry::Point2d lxbBL1, lxbBR1;
   lxbBL1.Move(geomUtil::GetPoint(pnt));
   lxbBL1.Offset(0, -H1 - H2);

   pnt.Release();
   topLowerXBeamProfile->get_Count(&nPoints);
   topLowerXBeamProfile->get_Item(nPoints - 1, &pnt);
   lxbBR1.Move(geomUtil::GetPoint(pnt));
   lxbBR1.Offset(0, -H3 - H4);

   WBFL::Geometry::Point2d lxbBL2, lxbBR2;
   Float64 y;
   std::tie(Xl,y) = lxbBL1.GetLocation(); // TRICKY: changing Xl to now be the x-location of left dimensions for the columns
   lxbBL2.Move(Xl+X1,y);

   std::tie(Xr,y) = lxbBR1.GetLocation();
   lxbBR2.Move(Xr-X3,y); // TRICKY: changing Xr to now be the x-location of right dimensions for the columns

   // Horizontal Cross Beam Dimensions
   BuildDimensionLine(displayList,lxbBL2,lxbBL1); // X1 Dimension
   BuildDimensionLine(displayList,lxbBR1,lxbBR2); // X3 Dimension


   // Column Dimensions

   // Column Height
   Float64 YbotColMin = DBL_MAX;
   ColumnIndexType nColumns = pPier->GetColumnCount(pierID);
   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      Float64 XxbCol = pPier->GetColumnLocation(pierID,colIdx);
      Float64 XpCol = pPier->ConvertCrossBeamToPierCoordinate(pierID,XxbCol);
      Float64 YtopCol = pPier->GetTopColumnElevation(pierID,colIdx);
      Float64 YbotCol = pPier->GetBottomColumnElevation(pierID,colIdx);

      WBFL::Geometry::Point2d pntTop(XpCol, YtopCol);
      WBFL::Geometry::Point2d pntBot(XpCol, YbotCol);
      BuildDimensionLine(displayList,pntTop,pntBot); // Column Height

      YbotColMin = Min(YbotColMin,YbotCol);
   }

   // Column Spacing (starts with left cross beam cantilever and
   // than proceeds with the spacing between columns at their base)
   // create the dimension line with rightpt,leftpt so the text comes
   // out on the correct side
   WBFL::Geometry::Point2d pntLeft(Xl, YbotColMin);
   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      Float64 XxbCol = pPier->GetColumnLocation(pierID,colIdx);
      Float64 XpCol = pPier->ConvertCrossBeamToPierCoordinate(pierID,XxbCol);

      WBFL::Geometry::Point2d pntRight(XpCol,YbotColMin);
     
      BuildDimensionLine(displayList,pntRight,pntLeft); // first time this is X5, then S

      pntLeft = pntRight;
   }

   // Right cross beam cantilever
   WBFL::Geometry::Point2d pntRight(Xr,YbotColMin);
   BuildDimensionLine(displayList,pntRight,pntLeft); // X6 Dimension

   //
   // Cross section dimensions
   //

   Float64 Lxb = pPier->GetXBeamLength(xbrTypes::xblTopLowerXBeam, pierID);
   Lxb = pPier->ConvertCrossBeamToPierCoordinate(pierID, Lxb);

   Float64 Z = pPier->ConvertPierToCrossBeamCoordinate(pierID,m_pFrame->GetCurrentCutLocation());

   if ( pProject->GetPierType(pierID) != xbrTypes::pctExpansion )
   {
      // Upper Cross Beam (End View)
      CComPtr<IShape> upperXBeamShape;
      pSectProp->GetXBeamShape(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,Z),&upperXBeamShape);

      CComQIPtr<ICompositeShape> composite(upperXBeamShape);
      CComPtr<IShape> shape;
      if ( composite )
      {
         CComPtr<ICompositeShapeItem> upperShapeItem;
         composite->get_Item(1,&upperShapeItem);

         upperShapeItem->get_Shape(&shape);
      }
      else
      {
         shape = upperXBeamShape;
      }

      CComQIPtr<IXYPosition> position(shape);
      CComPtr<IPoint2d> pnt_left, pnt_right;
      position->get_LocatorPoint(lpTopLeft,&pnt_left);
      position->get_LocatorPoint(lpTopRight,&pnt_right);
      pnt_left->Offset(EndOffset+Lxb,0);
      pnt_right->Offset(EndOffset+Lxb,0);
      BuildDimensionLine(displayList,geomUtil::GetPoint(pnt_left),geomUtil::GetPoint(pnt_right));
   }

   // Lower Cross Beam (End View)
   CComPtr<IShape> lowerXBeamShape;
   pSectProp->GetXBeamShape(pierID,xbrTypes::Stage1,xbrPointOfInterest(INVALID_ID,Z),&lowerXBeamShape);

   CComQIPtr<IXYPosition> position(lowerXBeamShape);
   CComPtr<IPoint2d> pnt_left, pnt_right;
   position->get_LocatorPoint(lpBottomLeft,&pnt_left);
   position->get_LocatorPoint(lpBottomRight,&pnt_right);
   pnt_left->Offset(EndOffset+Lxb,0);
   pnt_right->Offset(EndOffset+Lxb,0);
   BuildDimensionLine(displayList,geomUtil::GetPoint(pnt_right),geomUtil::GetPoint(pnt_left));

   // End View Height
   CComPtr<IShape> xbeamShape;
   pSectProp->GetXBeamShape(pierID,xbrTypes::Stage2,xbrPointOfInterest(INVALID_ID,Z),&xbeamShape);

   position.Release();
   xbeamShape->QueryInterface(&position);
   CComPtr<IPoint2d> pntTop;
   CComPtr<IPoint2d> pntBot;
   position->get_LocatorPoint(lpTopRight,&pntTop);
   position->get_LocatorPoint(lpBottomRight,&pntBot);
   pntTop->Offset(EndOffset+Lxb,0);
   pntBot->Offset(EndOffset+Lxb,0);
   BuildDimensionLine(displayList,geomUtil::GetPoint(pntTop),geomUtil::GetPoint(pntBot));

   // Curb-to-curb width

   // This is a basic dimension generated from the curb line offset. It does not take
   // into account geometric effects of the roadway curvature
   Float64 skew = pPier->GetSkewAngle(pierID);

   Float64 LCO, RCO;
   pProject->GetCurbLineOffset(pierID, &LCO, &RCO);
   if (pProject->GetCurbLineDatum(pierID) == pgsTypes::omtBridge)
   {
      Float64 BLO = pProject->GetBridgeLineOffset(pierID);
      LCO += BLO;
      RCO += BLO;
   }

   Float64 Ylc = pPier->GetElevation(pierID, 0);
   Float64 Yrc = pPier->GetElevation(pierID, RCO - LCO);
   Float64 Y = Max(Ylc, Yrc);

   LCO /= cos(skew); // skew adjust
   WBFL::Geometry::Point2d pntLC(LCO, Y);

   RCO /= cos(skew); // skew adjust
   WBFL::Geometry::Point2d pntRC(RCO, Y);

   BuildDimensionLine(displayList, pntLC, pntRC, false /*don't omit if zero distance*/);
}

void CXBeamRateView::BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint, bool bOmitForZeroDistance)
{
   Float64 distance = toPoint.Distance(fromPoint);
   if ( !IsZero(distance) || !bOmitForZeroDistance)
   {
      BuildDimensionLine(pDL,fromPoint,toPoint,distance);
   }
}

void CXBeamRateView::BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint,Float64 dimension)
{
   // put points at locations and make them sockets
   auto from_rep = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
   from_rep->SetPosition(fromPoint,false,false);
   auto from_connectable = std::shared_ptr<WBFL::DManip::iConnectable>(from_rep);
   auto from_socket = from_connectable->AddSocket(0,fromPoint);
   from_rep->Visible(false);
   pDL->AddDisplayObject(from_rep);

   auto to_rep = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
   to_rep->SetPosition(toPoint,false,false);
   auto to_connectable = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(to_rep);
   auto to_socket = to_connectable->AddSocket(0,toPoint);
   to_rep->Visible(false);
   pDL->AddDisplayObject(to_rep);

   // Create the dimension line object
   auto dimLine = WBFL::DManip::DimensionLine::Create(m_DisplayObjectID++);

   dimLine->SetArrowHeadStyle(WBFL::DManip::ArrowHeadStyleType::Filled);

   // Attach connector (the dimension line) to the sockets 
   auto connector = std::dynamic_pointer_cast<WBFL::DManip::iConnector>(dimLine);
   auto startPlug = connector->GetStartPlug();
   auto endPlug = connector->GetEndPlug();

   from_socket->Connect(startPlug);
   to_socket->Connect(endPlug);

   // Create the text block and attach it to the dimension line
   auto textBlock = WBFL::DManip::TextBlock::Create();

   // Format the dimension text
   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   CString strDimension = FormatDimension(dimension,pDisplayUnits->GetSpanLengthUnit());

   textBlock->SetText(strDimension);
   textBlock->SetBkMode(TRANSPARENT);

   dimLine->SetTextBlock(textBlock);


   pDL->AddDisplayObject(dimLine);
}

void CXBeamRateView::HandleLButtonDblClk(UINT nFlags, CPoint logPoint)
{
   CDisplayView::HandleLButtonDblClk(nFlags,logPoint);
   auto pBroker = EAFGetBroker();


   if ( IsStandAlone() )
   {
      GET_IFACE2(pBroker,IXBRProjectEdit,pProjectEdit);
      pProjectEdit->EditPier(INVALID_ID);
   }
   else
   {
      GET_IFACE2(pBroker,IEditByUI,pEditByUI);
      pEditByUI->EditPierDescription(GetPierIndex(),0/*pageIdx*/);
   }
}

std::shared_ptr<WBFL::DManip::iLineDisplayObject> CXBeamRateView::CreateLineDisplayObject(const WBFL::Geometry::Point2d& pntStart, const WBFL::Geometry::Point2d& pntEnd)
{
   auto doPntStart = WBFL::DManip::PointDisplayObject::Create();
   doPntStart->Visible(false);
   doPntStart->SetPosition(pntStart,false,false);
   auto connectable1 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doPntStart);
   auto socket1 = connectable1->AddSocket(0,pntStart);

   auto doPntEnd = WBFL::DManip::PointDisplayObject::Create();
   doPntEnd->Visible(false);
   doPntEnd->SetPosition(pntEnd,false,false);
   auto connectable2 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doPntEnd);
   auto socket2 = connectable2->AddSocket(0,pntEnd);

   auto doLine = WBFL::DManip::LineDisplayObject::Create();

   auto connector = std::dynamic_pointer_cast<WBFL::DManip::iConnector>(doLine);
   auto startPlug = connector->GetStartPlug();
   auto endPlug = connector->GetEndPlug();

   connectable1->Connect(0,WBFL::DManip::AccessType::ByID,startPlug);
   connectable2->Connect(0,WBFL::DManip::AccessType::ByID,endPlug);

   return doLine;
}

DROPEFFECT CXBeamRateView::CanDrop(COleDataObject* pDataObject,DWORD dwKeyState,const WBFL::Geometry::Point2d& point)
{
   // This override has to determine if the thing being dragged over it can
   // be dropped. In order to do that, it must unpackage the OleDataObject.
   //
   // The stuff in the data object is just from the display object. The display
   // objects need to be updated so that the client can attach an object to it
   // that knows how to package up domain specific information. At the same
   // time, this view needs to be able to get some domain specific hint 
   // as to the type of data that is going to be dropped.

   if ( pDataObject->IsDataAvailable(CSectionCutDisplayImpl::ms_Format) )
   {
      // need to peek at our object first and make sure it's coming from the local process
      // this is ugly because it breaks encapsulation of CBridgeSectionCutDisplayImpl
      auto source = WBFL::DManip::DragDataSource::Create();
      source->SetDataObject(pDataObject);
      source->PrepareFormat(CSectionCutDisplayImpl::ms_Format);

      CWinThread* thread = ::AfxGetThread( );
      DWORD threadid = thread->m_nThreadID;

      DWORD threadl;
      // know (by voodoo) that the first member of this data source is the thread id
      source->Read(CSectionCutDisplayImpl::ms_Format,&threadl,sizeof(DWORD));

      if (threadl == threadid)
      {
        return DROPEFFECT_MOVE;
      }
   }

   return DROPEFFECT_NONE;
}

void CXBeamRateView::OnDropped(COleDataObject* pDataObject,DROPEFFECT dropEffect, const WBFL::Geometry::Point2d& point)
{
   AfxMessageBox(_T("CBridgePlanView::OnDropped"));
}

void CXBeamRateView::SkewGirderShape(Float64 skew,Float64 shear,IShape* pShape,IShape** ppSkewedShape)
{
   if ( IsZero(skew) )
   {
      // Not skewed... nothing to do
      (*ppSkewedShape) = pShape;
      (*ppSkewedShape)->AddRef();
      return;
   }

   CComPtr<IRect2d> bbox;
   pShape->get_BoundingBox(&bbox);
   CComPtr<IPoint2d> pntTC;
   bbox->get_TopCenter(&pntTC);
   Float64 xcl;
   pntTC->get_X(&xcl);

   CComPtr<IPoint2dCollection> points;
   pShape->get_PolyPoints(&points);
   CComPtr<IPoint2d> pnt;
   CComPtr<IEnumPoint2d> enumPoints;
   points->get__Enum(&enumPoints);
   while ( enumPoints->Next(1,&pnt,nullptr) != S_FALSE )
   {
      Float64 x,y;
      pnt->Location(&x,&y);

      y += shear*(x-xcl);
      x /= cos(skew);

      pnt->Move(x,y);

      pnt.Release();
   }

   CComPtr<IPolyShape> polyShape;
   polyShape.CoCreateInstance(CLSID_PolyShape);
   polyShape->AddPoints(points);

   CComQIPtr<IShape> s(polyShape);
   s.CopyTo(ppSkewedShape);
}

void CXBeamRateView::HandleContextMenu(CWnd* pWnd,CPoint logPoint)
{
   CDisplayView::HandleContextMenu(pWnd,logPoint);

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CEAFBrokerDocument* pDoc = (CEAFBrokerDocument*)GetDocument();
   std::shared_ptr<WBFL::EAF::Menu> pContextMenu;

   PierIndexType pierIdx = GetPierIndex();
   if ( IsPGSExtension() )
   {
      auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker,IBridgeDescription,pIBridgeDesc);
      const CPierData2* pPierData = pIBridgeDesc->GetPier(pierIdx);
      if ( pPierData->GetPierModelType() == pgsTypes::pmtIdealized )
      {
         return;
      }
      else
      {
         pContextMenu = WBFL::EAF::Menu::CreateContextMenu(pDoc->GetPluginCommandManager());
         pContextMenu->LoadMenu(IDR_PGS_PIER_VIEW_CTX,nullptr);
      }
   }
   else
   {
      pContextMenu = WBFL::EAF::Menu::CreateContextMenu(pDoc->GetPluginCommandManager());
      pContextMenu->LoadMenu(IDR_XBR_PIER_VIEW_CTX,nullptr);
      pDoc->BuildReportMenu(pContextMenu,true);
   }

   if ( logPoint.x < 0 || logPoint.y < 0 )
   {
      // the context menu key or Shift+F10 was pressed
      // need some real coordinates (how about the center of the client area)
      CRect rClient;
      GetClientRect(&rClient);
      CPoint center = rClient.TopLeft();
      ClientToScreen(&center);
      logPoint = center;
   }

   pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, logPoint.x, logPoint.y, this);
}

void CXBeamRateView::OnExportPier()
{
   PierIndexType pierIdx = GetPierIndex();

   auto pBroker = EAFGetBroker();

   GET_IFACE2(pBroker,IXBRExport,pExport);
   pExport->Export(pierIdx);
}

void CXBeamRateView::OnEditPier()
{
   auto pBroker = EAFGetBroker();


   if ( IsStandAlone() )
   {
      GET_IFACE2(pBroker,IXBRProjectEdit,pProjectEdit);
      pProjectEdit->EditPier(INVALID_ID);
   }
   else
   {
      GET_IFACE2(pBroker,IEditByUI,pEditByUI);
      pEditByUI->EditPierDescription(GetPierIndex(),0/*pageIdx*/);
   }
}