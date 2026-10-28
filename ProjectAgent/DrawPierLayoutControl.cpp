///////////////////////////////////////////////////////////////////////
// PGSuper - Prestressed Girder SUPERstructure Design and Analysis
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
// DrawPierLayoutControl.cpp : implementation file
//

#include "stdafx.h"
#include <PointDefinedArc.h>
#include "ProjectAgent.h"
#include <PGSuperColors.h>
#include "DrawPierLayoutControl.h"

#include <WBFLGenericBridge.h>

#include <WBFLGeometry/GeomHelpers.h>

#include <EAF\EAFDisplayUnits.h>
#include <Math/Math.h>

#define XBEAM_LINE_COLOR               GREY50
#define XBEAM_FILL_COLOR               GREY70

#define ROADWAY_DISPLAY_LIST_ID        0
#define CROSSBEAM_DISPLAY_LIST_ID      1
#define COLUMN_DISPLAY_LIST_ID         2
#define DIMENSIONS_DISPLAY_LIST_ID     3

// The End/Section view of the pier is offset from the Elevation
// view by this amount.
const Float64 EndOffset = WBFL::Units::ConvertToSysUnits(10, WBFL::Units::Measure::Feet);

const WBFL::DManip::SelectionType g_selectionType = WBFL::DManip::SelectionType::None;

UINT WM_PIER_LAYOUT_POPOUT_CLOSED = ::RegisterWindowMessage(_T("XBRate.PierLayoutPopoutClosed"));

IMPLEMENT_DYNAMIC(CDrawPierLayoutControl, CDisplayWnd)

CDrawPierLayoutControl::CDrawPierLayoutControl()
{
    m_DisplayObjectID = 0;
}

CDrawPierLayoutControl::~CDrawPierLayoutControl()
{
}

BEGIN_MESSAGE_MAP(CDrawPierLayoutControl, CDisplayWnd)
    ON_WM_PAINT()
    ON_WM_DESTROY()
    ON_WM_SIZE()
END_MESSAGE_MAP()

void CDrawPierLayoutControl::OnDraw(CDC* pDC)
{
    CDisplayWnd::OnDraw(pDC);
}

void CDrawPierLayoutControl::CustomInit(const xbrPierData& pierData)
{
    CDisplayWnd::CustomInit();

    m_PierData = pierData;
    if (!m_pDispMgr->FindDisplayList(ROADWAY_DISPLAY_LIST_ID))
        m_pDispMgr->CreateDisplayList(ROADWAY_DISPLAY_LIST_ID);
    if (!m_pDispMgr->FindDisplayList(DIMENSIONS_DISPLAY_LIST_ID))
        m_pDispMgr->CreateDisplayList(DIMENSIONS_DISPLAY_LIST_ID);
    if (!m_pDispMgr->FindDisplayList(CROSSBEAM_DISPLAY_LIST_ID))
        m_pDispMgr->CreateDisplayList(CROSSBEAM_DISPLAY_LIST_ID);
    if (!m_pDispMgr->FindDisplayList(COLUMN_DISPLAY_LIST_ID))
        m_pDispMgr->CreateDisplayList(COLUMN_DISPLAY_LIST_ID);
    Refresh(pierData, true);
}

void CDrawPierLayoutControl::Refresh(const xbrPierData& pierData, bool fit)
{
    m_PierData = pierData;
    CComPtr<IPier> pier;
    m_PierData.CreatePierModel(&pier);
    m_PierModel = pier;
    m_CrossBeam.Release();
    m_PierModel->get_CrossBeam(&m_CrossBeam);
    m_Columns.Release();
    m_PierModel->get_ColumnLayout(&m_Columns);
    UpdateDisplayObjects();
    if (fit)
        ScaleToFit();
    Invalidate();
}

void CDrawPierLayoutControl::UpdateDisplayObjects()
{
    CDManipClientDC dc2(this);
    SetMappingMode(WBFL::DManip::MapMode::Isotropic, false);

    CWaitCursor wait;

    // Capture the current selection before blasting all the 
    // display objects
    auto vCurSel = m_pDispMgr->GetSelectedObjects();
    ATLASSERT(vCurSel.size() < 2);
    IDType curSel = INVALID_ID;
    IDType listID = INVALID_ID;
    if (vCurSel.size() == 1)
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
    UpdateDimensionsDisplayObjects();

    // Re-instate the current selection
    if (curSel != INVALID_ID)
    {
        auto doSel = m_pDispMgr->FindDisplayObject(curSel, listID, WBFL::DManip::AccessType::ByID);
        if (doSel)
            m_pDispMgr->SelectObject(doSel, TRUE);
    }
}

std::shared_ptr<WBFL::DManip::iLineDisplayObject> CDrawPierLayoutControl::CreateLineDisplayObject(const WBFL::Geometry::Point2d& pntStart, const WBFL::Geometry::Point2d& pntEnd)
{
    auto doPntStart = WBFL::DManip::PointDisplayObject::Create();
    doPntStart->Visible(false);
    doPntStart->SetPosition(pntStart, false, false);
    auto connectable1 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doPntStart);
    auto socket1 = connectable1->AddSocket(0, pntStart);

    auto doPntEnd = WBFL::DManip::PointDisplayObject::Create();
    doPntEnd->Visible(false);
    doPntEnd->SetPosition(pntEnd, false, false);
    auto connectable2 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doPntEnd);
    auto socket2 = connectable2->AddSocket(0, pntEnd);

    auto doLine = WBFL::DManip::LineDisplayObject::Create();

    auto connector = std::dynamic_pointer_cast<WBFL::DManip::iConnector>(doLine);
    auto startPlug = connector->GetStartPlug();
    auto endPlug = connector->GetEndPlug();

    connectable1->Connect(0, WBFL::DManip::AccessType::ByID, startPlug);
    connectable2->Connect(0, WBFL::DManip::AccessType::ByID, endPlug);

    return doLine;
}

void CDrawPierLayoutControl::UpdateRoadwayDisplayObjects()
{
    const xbrPierData* pPier = &m_PierData;
    auto displayList = m_pDispMgr->FindDisplayList(ROADWAY_DISPLAY_LIST_ID);
    const Float64 Ytop = GetElevation(ConvertPierToCurbLineCoordinate(0.0))
        + WBFL::Units::ConvertToSysUnits(1.0, WBFL::Units::Measure::Feet);
    const Float64 Ybottom = Ytop - GetMaxColumnHeight();
    auto alignment = CreateLineDisplayObject({0.0, Ytop}, {0.0, Ybottom});
    auto strategy = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawLineStrategy>(alignment->GetDrawLineStrategy());
    strategy->SetColor(ALIGNMENT_COLOR);
    strategy->SetWidth(ALIGNMENT_LINE_WEIGHT);
    strategy->SetLineStyle(WBFL::DManip::LineStyleType::Centerline);
    displayList->AddDisplayObject(alignment);

    Float64 skew;
    CComPtr<IAngle> angle;
    angle.CoCreateInstance(CLSID_Angle);
    angle->FromString(CComBSTR(pPier->GetSkew()));
    angle->get_Value(&skew);
    const Float64 Xbridge = pPier->GetBridgeLineOffset() / cos(skew);
    if (!IsZero(Xbridge))
    {
        auto bridge = CreateLineDisplayObject({Xbridge, Ytop}, {Xbridge, Ybottom});
        auto draw = std::dynamic_pointer_cast<WBFL::DManip::SimpleDrawLineStrategy>(bridge->GetDrawLineStrategy());
        draw->SetColor(BRIDGE_COLOR);
        draw->SetWidth(BRIDGELINE_LINE_WEIGHT);
        draw->SetLineStyle(WBFL::DManip::LineStyleType::Centerline);
        displayList->AddDisplayObject(bridge);
    }

    CComPtr<IPoint2dCollection> profile;
    m_PierModel->get_DeckProfile(&profile);
    auto deck = WBFL::DManip::PolyLineDisplayObject::Create(m_DisplayObjectID++);
    IndexType count;
    profile->get_Count(&count);
    for (IndexType idx = 0; idx < count; ++idx)
    {
        CComPtr<IPoint2d> point;
        profile->get_Item(idx, &point);
        deck->AddPoint(geomUtil::GetPoint(point));
    }
    deck->SetPointType(WBFL::DManip::PointType::None);
    deck->SetColor(PROFILE_COLOR);
    deck->SetWidth(PROFILE_LINE_WEIGHT);
    deck->SetSelectionType(g_selectionType);
    displayList->AddDisplayObject(deck);
}

void CDrawPierLayoutControl::UpdateXBeamDisplayObjects()
{

    const xbrPierData* pPier = &m_PierData;

    auto displayList = m_pDispMgr->FindDisplayList(CROSSBEAM_DISPLAY_LIST_ID);

    auto pBroker = EAFGetBroker();

    // Model Upper Cross Beam (Elevation)
    WBFL::Geometry::Point2d point(0, 0);

    if (pPier->GetPierType() != pgsTypes::pctExpansion)
    {
        auto doUpperXBeam = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
        doUpperXBeam->SetPosition(point, false, false);
        doUpperXBeam->SetSelectionType(g_selectionType);

        CComPtr<IShape> upperXBeamShape;
        GetXBeamProfile(pgsTypes::Stage::Stage2, &upperXBeamShape);

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
    doLowerXBeam->SetPosition(point, false, false);
    doLowerXBeam->SetSelectionType(g_selectionType);

    CComPtr<IShape> pLowerXBeamShape;
    GetXBeamProfile(pgsTypes::Stage::Stage1, &pLowerXBeamShape);

    auto lowerXBeamDrawStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
    lowerXBeamDrawStrategy->SetShape(geomUtil::ConvertShape(pLowerXBeamShape));
    lowerXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
    lowerXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
    lowerXBeamDrawStrategy->Fill(true);

    doLowerXBeam->SetDrawingStrategy(lowerXBeamDrawStrategy);

    auto lower_xbeam_gravity_well = WBFL::DManip::ShapeGravityWellStrategy::Create();
    lower_xbeam_gravity_well->SetShape(geomUtil::ConvertShape(pLowerXBeamShape));
    doLowerXBeam->SetGravityWellStrategy(lower_xbeam_gravity_well);

    displayList->AddDisplayObject(doLowerXBeam);

    // Section Cut
    Float64 Lxb = pPier->GetXBeamLength();
    Lxb = ConvertCrossBeamToPierCoordinate(Lxb);

    const Float64 XxbCut = pPier->GetXBeamLength() / 2.0;

    auto doXBeamSection = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
    doXBeamSection->SetPosition(point, false, false);
    doXBeamSection->SetSelectionType(g_selectionType);

    CComPtr<IShape> xbeamShape;
    GetXBeamShape(pgsTypes::Stage2, XxbCut, &xbeamShape);
    CComQIPtr<IXYPosition> position(xbeamShape);
    position->Offset(EndOffset + Lxb, 0);

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
}

void CDrawPierLayoutControl::UpdateColumnDisplayObjects()
{
    auto displayList = m_pDispMgr->FindDisplayList(COLUMN_DISPLAY_LIST_ID);

    auto pBroker = EAFGetBroker();

   // Create a function that represents the bottom of the cross beam
   // We will use it to make the top of the column match the bottom of the
   // cross beam.
    WBFL::Math::PiecewiseFunction fn;
    CComPtr<IPoint2dCollection> points;

    const xbrPierData* pPier = &m_PierData;

    Float64 H1L, H2L, H1R, H2R;
    Float64 X2L, X1L, X2R, X1R;
    Float64 width;
    std::vector<CPierPointData> pierPoints;
    pPier->GetLowerXBeamDimensions(&H1L, &H1R, &H2L, &H2R, &X1L, &X1R, &X2L, &X2R, &width, &pierPoints);

    m_CrossBeam->get_BottomSurface(0, &points);

    CComPtr<IEnumPoint2d> enumPoints;
    points->get__Enum(&enumPoints);
    CComPtr<IPoint2d> pnt;
    while (enumPoints->Next(1, &pnt, nullptr) != S_FALSE)
    {
        Float64 x, y;
        pnt->Location(&x, &y);
        fn.AddPoint(x, y);
        pnt.Release();
    }

    IndexType nColumns = pPier->GetColumnCount();
    for (IndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        const CColumnData& columnData = pPier->GetColumnData(colIdx);
		Float64 XxbCol = GetColumnLocation(colIdx);
        Float64 XpCol = ConvertCrossBeamToPierCoordinate(XxbCol);
        Float64 Ytop = GetTopColumnElevation(colIdx);
        Float64 Ybot = GetBottomColumnElevation(colIdx);

        CColumnData::ColumnShapeType colShapeType = columnData.GetColumnShape();
        Float64 d1, d2;
        columnData.GetColumnDimensions(&d1, &d2);
        CColumnData::ColumnHeightMeasurementType columnHeightType = columnData.GetColumnHeightMeasurementType();
        Float64 H = columnData.GetColumnHeight();

        WBFL::Geometry::Point2d pntTop(XpCol, Ytop);
        WBFL::Geometry::Point2d pntBot(XpCol, Ybot);

        auto doTop = WBFL::DManip::PointDisplayObject::Create();
        doTop->Visible(false);
        doTop->SetPosition(pntTop, false, false);
        auto connectable1 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doTop);
        auto socket1 = connectable1->AddSocket(0, pntTop);

        auto doBot = WBFL::DManip::PointDisplayObject::Create();
        doBot->Visible(false);
        doBot->SetPosition(pntBot, false, false);
        auto connectable2 = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(doBot);
        auto socket2 = connectable2->AddSocket(0, pntBot);

        // Create the shape of the column
        auto columnShape = std::make_shared<WBFL::Geometry::Polygon>();
        Float64 X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11;

        X6 = pntTop.X();
        X1 = X6 - d1 * 0.5;
        X2 = X6 - d1 * 0.4;
        X3 = X6 - d1 * 0.3;
        X4 = X6 - d1 * 0.2;
        X5 = X6 - d1 * 0.1;
        X7 = X6 + d1 * 0.1;
        X8 = X6 + d1 * 0.2;
        X9 = X6 + d1 * 0.3;
        X10 = X6 + d1 * 0.4;
        X11 = X6 + d1 * 0.5;

        pgsTypes::OffsetMeasurementType refColMeasure;
        ColumnIndexType refColIdx;
        Float64 refColOffset;
        pPier->GetRefColumnLocation(&refColMeasure, &refColIdx, &refColOffset);

        Float64 Y1 = fn.Evaluate(X1);
        Float64 Y2 = fn.Evaluate(X2);
        Float64 Y3 = fn.Evaluate(X3);
        Float64 Y4 = fn.Evaluate(X4);
        Float64 Y5 = fn.Evaluate(X5);
        Float64 Y6 = fn.Evaluate(X6);
        Float64 Y7 = fn.Evaluate(X7);
        Float64 Y8 = fn.Evaluate(X8);
        Float64 Y9 = fn.Evaluate(X9);
        Float64 Y10 = fn.Evaluate(X10);
        Float64 Y11 = fn.Evaluate(X11);

        columnShape->AddPoint(X1, Y1);
        columnShape->AddPoint(X2, Y2);
        columnShape->AddPoint(X3, Y3);
        columnShape->AddPoint(X4, Y4);
        columnShape->AddPoint(X5, Y5);
        columnShape->AddPoint(X6, Y6);
        columnShape->AddPoint(X7, Y7);
        columnShape->AddPoint(X8, Y8);
        columnShape->AddPoint(X9, Y9);
        columnShape->AddPoint(X10, Y10);
        columnShape->AddPoint(X11, Y11);
        columnShape->AddPoint(X11, Ybot);
        columnShape->AddPoint(X1, Ybot);

        auto doColumn = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
        doColumn->SetPosition(pntTop, false, false);
        doColumn->SetSelectionType(g_selectionType);

        auto drawColumnStrategy = WBFL::DManip::ShapeDrawStrategy::Create();
        doColumn->SetDrawingStrategy(drawColumnStrategy);

        drawColumnStrategy->SetShape(columnShape);
        drawColumnStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
        drawColumnStrategy->SetSolidFillColor(XBEAM_LINE_COLOR);
        drawColumnStrategy->Fill(true);

        displayList->AddDisplayObject(doColumn);
    }
}

void CDrawPierLayoutControl::UpdateDimensionsDisplayObjects()
{
    auto displayList = m_pDispMgr->FindDisplayList(DIMENSIONS_DISPLAY_LIST_ID);

    auto pBroker = EAFGetBroker();

    const xbrPierData* pPier = &m_PierData;
    pgsTypes::PierType pierType = pPier->GetPierType();

    Float64 H1L, H2L, H1R, H2R;
    Float64 X2L, X1L, X2R, X1R;
    Float64 width;
    std::vector<CPierPointData> pierPoints;
    pPier->GetLowerXBeamDimensions(&H1L, &H1R, &H2L, &H2R, &X1L, &X1R, &X2L, &X2R, &width, &pierPoints);

    Float64 Dd, Hu;
    pPier->GetDiaphragmDimensions(&Hu, &Dd);

    CComPtr<IPoint2dCollection> topUpperXBeamProfile, topLowerXBeamProfile, bottomXBeamProfile;
    m_CrossBeam->get_TopSurface(1, &topUpperXBeamProfile);
    m_CrossBeam->get_TopSurface(0, &topLowerXBeamProfile);
    m_CrossBeam->get_BottomSurface(0, &bottomXBeamProfile);

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
    Float64 ublc;
    pnt->get_X(&ublc);
    WBFL::Geometry::Point2d uxbBL(geomUtil::GetPoint(pnt));
    WBFL::Geometry::Point2d origin(geomUtil::GetPoint(pnt));
    uxbBL.X() = Xl;

    // Lower Cross Beam - Bottom Left
    pnt.Release();
    bottomXBeamProfile->get_Item(0, &pnt);
    Float64 lblc;
    pnt->get_X(&lblc);
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
    Float64 ubrc;
    pnt->get_X(&ubrc);
    WBFL::Geometry::Point2d uxbBR(geomUtil::GetPoint(pnt));
    uxbBR.X() = Xr;

    // Lower Cross Beam - Bottom Right
    pnt.Release();
    bottomXBeamProfile->get_Count(&nPoints);
    bottomXBeamProfile->get_Item(nPoints - 1, &pnt);
    Float64 lbrc;
    pnt->get_X(&lbrc);
    WBFL::Geometry::Point2d lxbBR(geomUtil::GetPoint(pnt));
    lxbBR.X() = Xr;

    // Height of upper cross beam
    if (pierType != pgsTypes::pctExpansion)
    {
        BuildDimensionLine(displayList, uxbBL, uxbTL);
        BuildDimensionLine(displayList, uxbTR, uxbBR);
    }

    // Height of lower cross beam
    BuildDimensionLine(displayList, lxbBL, uxbBL); // H1L Dimension
    BuildDimensionLine(displayList, uxbBR, lxbBR); // H1R Dimension

    Float64 Ybot = lxbBL.Y();
    WBFL::Geometry::Point2d blc(lblc - X1L, Ybot);
    WBFL::Geometry::Point2d x1l(lblc, Ybot);
    BuildDimensionLine(displayList, x1l, blc);
    WBFL::Geometry::Point2d brc(lbrc + X1R, Ybot);
    WBFL::Geometry::Point2d x1r(lbrc, Ybot);
    BuildDimensionLine(displayList, brc, x1r);

    // Get the center points for the arcs at column centers
    ColumnIndexType nColumns = pPier->GetColumnCount();

    const auto& pierLayoutType = pPier->GetPierLayoutType();

	if (pierLayoutType == pgsTypes::pltCommon)
    {
        // Lower cross beam bottom taper, vertical dimensions
        WBFL::Geometry::Point2d lxbBLC, lxbBRC;
        lxbBLC.Move(lxbBL);
        lxbBLC.Offset(0, -H2L);
        lxbBRC.Move(lxbBR);
        lxbBRC.Offset(0, -H2R);

        BuildDimensionLine(displayList, lxbBLC, lxbBL); // H2L Dimension
        BuildDimensionLine(displayList, lxbBR, lxbBRC); // H2R Dimension

        // Lower cross beam bottom taper, horizontal dimensions
        pnt.Release();
        topLowerXBeamProfile->get_Item(0, &pnt);
        WBFL::Geometry::Point2d lxbBL1, lxbBR1;
        lxbBL1.Move(geomUtil::GetPoint(pnt));
        lxbBL1.Offset(0, -H1L - H2L);

        pnt.Release();
        topLowerXBeamProfile->get_Count(&nPoints);
        topLowerXBeamProfile->get_Item(nPoints - 1, &pnt);
        lxbBR1.Move(geomUtil::GetPoint(pnt));
        lxbBR1.Offset(0, -H1R - H2R);

        WBFL::Geometry::Point2d lxbBL2, lxbBR2;
        Float64 y;
        std::tie(Xl, y) = lxbBL1.GetLocation(); // TRICKY: changing Xl to now be the x-location of left dimensions for the columns
        lxbBL2.Move(Xl + X2L, y);

        std::tie(Xr, y) = lxbBR1.GetLocation();
        lxbBR2.Move(Xr - X2R, y); // TRICKY: changing Xr to now be the x-location of right dimensions for the columns

        // Horizontal Cross Beam Dimensions
        BuildDimensionLine(displayList, lxbBL2, lxbBL1); // X2L Dimension
        BuildDimensionLine(displayList, lxbBR1, lxbBR2); // X2R Dimension
    }
    else if (pierLayoutType == pgsTypes::pltScalloped)
    {
        //Radius and depth of bottom scallop
        // R = radius of bottom scallop arcs (same for both left and right)
        // D = depth of lower cross beam at column center

        for (ColumnIndexType colIdx = 0;
            colIdx + 1 < nColumns; colIdx++)
        {
            Float64 X1 = GetColumnLocation(colIdx);
            Float64 X2 = GetColumnLocation(colIdx + 1);

            Float64 Xmid = (X1 + X2) / 2.0;

            Float64 XpMid =
                ConvertCrossBeamToPierCoordinate(Xmid);

            Float64 XpCol =
                ConvertCrossBeamToPierCoordinate(X1);

            CComPtr<IShape> shape1;
            GetXBeamShape(pgsTypes::Stage1,
                Xmid,
                &shape1);

            CComQIPtr<IXYPosition> position1(shape1);

            CComPtr<IShape> shape2;
            GetXBeamShape(pgsTypes::Stage1,
                X1,
                &shape2);

            CComQIPtr<IXYPosition> position2(shape2);

            CComPtr<IPoint2d> pntTop1;
            CComPtr<IPoint2d> pntBot1;

            position1->get_LocatorPoint(lpTopCenter, &pntTop1);
            position1->get_LocatorPoint(lpBottomCenter, &pntBot1);

            Float64 Ytop1, Ybot1;
            pntTop1->get_Y(&Ytop1);
            pntBot1->get_Y(&Ybot1);

            WBFL::Geometry::Point2d top1(XpMid, Ytop1);
            WBFL::Geometry::Point2d bottom1(XpMid, Ybot1);

            BuildDimensionLine(displayList, top1, bottom1);

            CComPtr<IPoint2d> pntTop2;
            CComPtr<IPoint2d> pntBot2;

            position2->get_LocatorPoint(lpTopCenter, &pntTop2);
            position2->get_LocatorPoint(lpBottomCenter, &pntBot2);

            Float64 Ytop2, Ybot2;
            pntTop2->get_Y(&Ytop2);
            pntBot2->get_Y(&Ybot2);

            WBFL::Geometry::Point2d top2(XpCol, Ytop2);
            WBFL::Geometry::Point2d bottom2(XpCol, Ybot2);

            BuildDimensionLine(displayList, top2, bottom2);
        }

        // Depth dimension at the last column 
        Float64 Xlast = GetColumnLocation(nColumns - 1);

        Float64 XpLast =
            ConvertCrossBeamToPierCoordinate(Xlast);

        CComPtr<IShape> shapeLast;
        GetXBeamShape(pgsTypes::Stage1,
            Xlast,
            &shapeLast);

        CComQIPtr<IXYPosition> positionLast(shapeLast);

        CComPtr<IPoint2d> pntTopLast;
        CComPtr<IPoint2d> pntBotLast;

        positionLast->get_LocatorPoint(lpTopCenter, &pntTopLast);
        positionLast->get_LocatorPoint(lpBottomCenter, &pntBotLast);

        Float64 YtopLast, YbotLast;
        pntTopLast->get_Y(&YtopLast);
        pntBotLast->get_Y(&YbotLast);

        WBFL::Geometry::Point2d topLast(XpLast, YtopLast);
        WBFL::Geometry::Point2d bottomLast(XpLast, YbotLast);

        BuildDimensionLine(displayList, topLast, bottomLast);

        GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

        for (ColumnIndexType arcIdx = 0; arcIdx <= nColumns; ++arcIdx)
        {
            Float64 Xleft, Xright;

            if (arcIdx == 0)
            {
                // Left overhang
                Xleft = ConvertPierToCrossBeamCoordinate(lblc);

                Xright = GetColumnLocation(0);
            }
            else if (arcIdx == nColumns)
            {
                // Right overhang
                Xleft = GetColumnLocation(nColumns - 1);

                Xright = ConvertPierToCrossBeamCoordinate(lbrc);
            }
            else
            {
                // Between adjacent columns
                Xleft = GetColumnLocation(arcIdx - 1);
                Xright = GetColumnLocation(arcIdx);
            }

            const Float64 Xmid = (Xleft + Xright) / 2.0;

            const Float64 Yleft = GetBottomY(Xleft);
            const Float64 Yright = GetBottomY(Xright);

            // Midpoint of the chord, accounting for inclination.
            const Float64 Ychord = (Yleft + Yright) / 2.0;

            const Float64 XpMid =
                ConvertCrossBeamToPierCoordinate(Xmid);

            // Small offset below the chord.
            WBFL::Geometry::Point2d ppRad(XpMid, Ychord - WBFL::Units::ConvertToSysUnits(0.5, WBFL::Units::Measure::Feet));

            const Float64 radius = arcIdx == 0
                ? pPier->GetXBeamOverhangRadius(pgsTypes::stLeft)
                : arcIdx == nColumns
                    ? pPier->GetXBeamOverhangRadius(pgsTypes::stRight)
                    : pPier->GetColumnScallopRadius(arcIdx - 1);
            const CString strRadius = FormatDimension(radius, pDisplayUnits->GetSpanLengthUnit());

            CString strRadiusLabel;
            if (arcIdx == 0)
                strRadiusLabel.Format(_T("RL = %s"), strRadius.GetString());
            else if (arcIdx == nColumns)
                strRadiusLabel.Format(_T("RR = %s"), strRadius.GetString());
            else
                strRadiusLabel.Format(_T("R%d = %s"), arcIdx, strRadius.GetString());

            auto doRadiusLabel = WBFL::DManip::TextBlock::Create();
            doRadiusLabel->SetText(strRadiusLabel);
            doRadiusLabel->SetBkMode(TRANSPARENT);
            doRadiusLabel->SetTextAlign(TA_TOP | TA_CENTER);
            doRadiusLabel->SetPosition(ppRad);

            displayList->AddDisplayObject(doRadiusLabel);
        }
    }
    else if (pierLayoutType == pgsTypes::pltUserDefined)
    {
        // Function representing the top of the lower cross beam
        WBFL::Math::PiecewiseFunction fnTop;

        CComPtr<IEnumPoint2d> enumPoints;
        topLowerXBeamProfile->get__Enum(&enumPoints);

        CComPtr<IPoint2d> pntTop;
        while (enumPoints->Next(1, &pntTop, nullptr) != S_FALSE)
        {
            Float64 x, y;
            pntTop->Location(&x, &y);
            fnTop.AddPoint(x, y);
            pntTop.Release();
        }

        Float64 origX = ConvertCrossBeamToPierCoordinate(0.0);
        Float64 origY = fnTop.Evaluate(origX) - H1L;
        WBFL::Geometry::Point2d origin(origX, origY);

        auto doOrigin = WBFL::DManip::PointDisplayObject::Create();
        doOrigin->SetPosition(origin, false, false);
        displayList->AddDisplayObject(doOrigin);

        CString strBlcLabel = _T("(0, 0)");

        auto doBlcLabel = WBFL::DManip::TextBlock::Create();
        doBlcLabel->SetText(strBlcLabel);
        doBlcLabel->SetBkMode(TRANSPARENT);
        doBlcLabel->SetTextAlign(TA_TOP | TA_CENTER);
        doBlcLabel->SetPosition(origin);
        displayList->AddDisplayObject(doBlcLabel);

        for (IndexType pointIdx = 0; pointIdx < pPier->GetPierPointData().size(); ++pointIdx)
        {
            const auto& pnt = pPier->GetPierPointData()[pointIdx];
            Float64 Xxb = pnt.Get_X();

            Float64 Xp = ConvertCrossBeamToPierCoordinate(Xxb);

            const Float64 Ytop = fnTop.Evaluate(Xp);

            const Float64 Ypoint = Ytop - pnt.Get_Y();

            WBFL::Geometry::Point2d pp(Xp, Ypoint);

            GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

            CString strPntLabel;
            strPntLabel.Format(_T("(%s,%s)"), ::FormatDimension(Xxb, pDisplayUnits->GetSpanLengthUnit()), ::FormatDimension(pnt.Get_Y(), pDisplayUnits->GetSpanLengthUnit()));

            auto doPntLabel = WBFL::DManip::TextBlock::Create();
            doPntLabel->SetText(strPntLabel);
            doPntLabel->SetBkMode(TRANSPARENT);
            doPntLabel->SetTextAlign(TA_TOP | TA_CENTER);
            doPntLabel->SetPosition(pp);
            displayList->AddDisplayObject(doPntLabel);

            auto doPoint = WBFL::DManip::PointDisplayObject::Create();
            doPoint->SetPosition(pp, false, false);
            displayList->AddDisplayObject(doPoint);
            if (pointIdx + 1 < pPier->GetPierPointData().size() && pnt.GetTransitionType() != CPierPointData::ttStraight)
            {
                const auto& next = pPier->GetPierPointData()[pointIdx+1];
                const Float64 nx = ConvertCrossBeamToPierCoordinate(next.Get_X());
                const Float64 ny = fnTop.Evaluate(nx) - next.Get_Y();
                WBFL::Geometry::PointDefinedArc arc;
                if (arc.Initialize(Xp,Ypoint,nx,ny,pnt.GetRadius(),pnt.GetTransitionType() == CPierPointData::ttArcUpward))
                {
                    Float64 mx,my;
                    arc.PointAtFraction(0.5,mx,my);
                    CString text;
                    const CString radiusText = FormatDimension(pnt.GetRadius(),pDisplayUnits->GetSpanLengthUnit());
                    text.Format(_T("R%d = %s"),pointIdx+1,radiusText.GetString());
                    auto label = WBFL::DManip::TextBlock::Create();
                    label->SetText(text);
                    label->SetBkMode(TRANSPARENT);
                    label->SetTextAlign(TA_TOP | TA_CENTER);
                    label->SetPosition(WBFL::Geometry::Point2d(mx,my - WBFL::Units::ConvertToSysUnits(0.5,WBFL::Units::Measure::Feet)));
                    displayList->AddDisplayObject(label);
                }
            }
        }

    }

    // Column Dimensions

    // Column Height
    Float64 YbotColMin = DBL_MAX;
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        Float64 XxbCol = GetColumnLocation(colIdx);
        Float64 XpCol = ConvertCrossBeamToPierCoordinate(XxbCol);
        Float64 YtopCol = GetTopColumnElevation(colIdx);
        Float64 YbotCol = GetBottomColumnElevation(colIdx);

        WBFL::Geometry::Point2d pntTop(XpCol, YtopCol);
        WBFL::Geometry::Point2d pntBot(XpCol, YbotCol);
        BuildDimensionLine(displayList, pntTop, pntBot); // Column Height

        YbotColMin = Min(YbotColMin, YbotCol);
    }

    // Column Spacing (starts with left cross beam cantilever and
    // than proceeds with the spacing between columns at their base)
    // create the dimension line with rightpt,leftpt so the text comes
    // out on the correct side
    WBFL::Geometry::Point2d pntLeft(ublc, YbotColMin);
    for (ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++)
    {
        Float64 XxbCol = GetColumnLocation(colIdx);
        Float64 XpCol = ConvertCrossBeamToPierCoordinate(XxbCol);

        WBFL::Geometry::Point2d pntRight(XpCol, YbotColMin);

        BuildDimensionLine(displayList, pntRight, pntLeft); // first time this is OHL, then S

        pntLeft = pntRight;
    }

    // Right cross beam cantilever
    WBFL::Geometry::Point2d pntRight(ubrc, YbotColMin);
    BuildDimensionLine(displayList, pntRight, pntLeft); // OHR Dimension

    // Reference column to alignment/bridge line dimension 

    pgsTypes::OffsetMeasurementType refColMeasure;
    ColumnIndexType refColIdx;
    Float64 refColOffset;
    pPier->GetRefColumnLocation(&refColMeasure, &refColIdx, &refColOffset);

    Float64 XxbRefCol = GetColumnLocation(refColIdx);
    Float64 XpRefCol =
    ConvertCrossBeamToPierCoordinate(XxbRefCol);

    // Alignment/bridge line is at X = 0 in pier coordinates
    Float64 XpAlignment = 0.0;
    if (refColMeasure == pgsTypes::omtBridge)
    {
        CComPtr<IAngle> skew;
        skew.CoCreateInstance(CLSID_Angle);
        skew->FromString(CComBSTR(pPier->GetSkew()));
        Float64 angle;
        skew->get_Value(&angle);
        XpAlignment = pPier->GetBridgeLineOffset() / cos(angle);
    }

    // Match the lower end of the alignment/bridge-line marker drawn in UpdateRoadwayDisplayObjects.
    const Float64 alignmentXcl = ConvertPierToCurbLineCoordinate(0.0);
    const Float64 alignmentBottom = GetElevation(alignmentXcl)
        + WBFL::Units::ConvertToSysUnits(1.0, WBFL::Units::Measure::Feet)
        - GetMaxColumnHeight();
    // Place the reference-column dimension six inches below that marker.
    const Float64 Ydim = alignmentBottom
        - WBFL::Units::ConvertToSysUnits(0.5, WBFL::Units::Measure::Feet);

    WBFL::Geometry::Point2d pntRefCol(XpRefCol, Ydim);
    WBFL::Geometry::Point2d pntAlignment(XpAlignment, Ydim);

    BuildDimensionLine(displayList, pntRefCol, pntAlignment);

    //
    // Cross section dimensions
    //

	Float64 Lxb = pPier->GetXBeamLength();
    Lxb = ConvertCrossBeamToPierCoordinate(Lxb);

    const Float64 Z = pPier->GetXBeamLength() / 2.0;

    if (pPier->GetPierType() != pgsTypes::pctExpansion)
    {
        // Upper Cross Beam (End View)
        CComPtr<IShape> upperXBeamShape;
		GetXBeamShape(pgsTypes::Stage2, Z, &upperXBeamShape);

        CComQIPtr<ICompositeShape> composite(upperXBeamShape);
        CComPtr<IShape> shape;
        if (composite)
        {
            CComPtr<ICompositeShapeItem> upperShapeItem;
            composite->get_Item(1, &upperShapeItem);

            upperShapeItem->get_Shape(&shape);
        }
        else
        {
            shape = upperXBeamShape;
        }

        CComQIPtr<IXYPosition> position(shape);
        CComPtr<IPoint2d> pnt_left, pnt_right;
        position->get_LocatorPoint(lpTopLeft, &pnt_left);
        position->get_LocatorPoint(lpTopRight, &pnt_right);
        pnt_left->Offset(EndOffset + Lxb, 0);
        pnt_right->Offset(EndOffset + Lxb, 0);
        BuildDimensionLine(displayList, geomUtil::GetPoint(pnt_left), geomUtil::GetPoint(pnt_right));
    }

    // Lower Cross Beam (End View)
    CComPtr<IShape> lowerXBeamShape;
    GetXBeamShape(pgsTypes::Stage1, Z, &lowerXBeamShape);

    CComQIPtr<IXYPosition> position(lowerXBeamShape);
    CComPtr<IPoint2d> pnt_left, pnt_right;
    position->get_LocatorPoint(lpBottomLeft, &pnt_left);
    position->get_LocatorPoint(lpBottomRight, &pnt_right);
    pnt_left->Offset(EndOffset + Lxb, 0);
    pnt_right->Offset(EndOffset + Lxb, 0);
    BuildDimensionLine(displayList, geomUtil::GetPoint(pnt_right), geomUtil::GetPoint(pnt_left));

    CComPtr<IShape> section;
    GetXBeamShape(pgsTypes::Stage2, Z, &section);
    CComQIPtr<IXYPosition> sectionPosition(section);
    CComPtr<IPoint2d> sectionTop, sectionBottom;
    sectionPosition->get_LocatorPoint(lpTopRight, &sectionTop);
    sectionPosition->get_LocatorPoint(lpBottomRight, &sectionBottom);
    sectionTop->Offset(EndOffset + Lxb, 0);
    sectionBottom->Offset(EndOffset + Lxb, 0);
    BuildDimensionLine(displayList, geomUtil::GetPoint(sectionTop), geomUtil::GetPoint(sectionBottom));

    // Curb-to-curb width

    // This is a basic dimension generated from the curb line offset. It does not take
    // into account geometric effects of the roadway curvature
	    CComPtr<IAngle> angle;
    angle.CoCreateInstance(CLSID_Angle);
    angle->FromString(CComBSTR(pPier->GetSkew()));
    Float64 skew;
    angle->get_Value(&skew);
    Float64 LCO, RCO;
    pPier->GetCurbLineOffset(&LCO, &RCO);
    if (pPier->GetCurbLineDatum() == pgsTypes::omtBridge)
    {
        LCO += pPier->GetBridgeLineOffset();
        RCO += pPier->GetBridgeLineOffset();
    }
    Float64 Ylc = GetElevation(0.0);
    Float64 Yrc = GetElevation((RCO - LCO) / cos(skew));
    Float64 Y = Max(Ylc, Yrc);

    LCO /= cos(skew); // skew adjust
    WBFL::Geometry::Point2d pntLC(LCO, Y);

    RCO /= cos(skew); // skew adjust
    WBFL::Geometry::Point2d pntRC(RCO, Y);

    BuildDimensionLine(displayList, pntLC, pntRC, false /*don't omit if zero distance*/);
}

void CDrawPierLayoutControl::BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint, bool bOmitForZeroDistance)
{
    Float64 distance = toPoint.Distance(fromPoint);
    if (!IsZero(distance) || !bOmitForZeroDistance)
    {
        BuildDimensionLine(pDL, fromPoint, toPoint, distance);
    }
}

void CDrawPierLayoutControl::BuildDimensionLine(std::shared_ptr<WBFL::DManip::iDisplayList> pDL, const WBFL::Geometry::Point2d& fromPoint, const WBFL::Geometry::Point2d& toPoint, Float64 dimension)
{
    // put points at locations and make them sockets
    auto from_rep = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
    from_rep->SetPosition(fromPoint, false, false);
    auto from_connectable = std::shared_ptr<WBFL::DManip::iConnectable>(from_rep);
    auto from_socket = from_connectable->AddSocket(0, fromPoint);
    from_rep->Visible(false);
    pDL->AddDisplayObject(from_rep);

    auto to_rep = WBFL::DManip::PointDisplayObject::Create(m_DisplayObjectID++);
    to_rep->SetPosition(toPoint, false, false);
    auto to_connectable = std::dynamic_pointer_cast<WBFL::DManip::iConnectable>(to_rep);
    auto to_socket = to_connectable->AddSocket(0, toPoint);
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

    GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);
    CString strDimension = FormatDimension(dimension, pDisplayUnits->GetSpanLengthUnit());

    textBlock->SetText(strDimension);
    textBlock->SetBkMode(TRANSPARENT);

    dimLine->SetTextBlock(textBlock);

    pDL->AddDisplayObject(dimLine);
}

BOOL CDrawPierLayoutControl::CreatePopout(const xbrPierData& pierData, CWnd* pOwner)
{
    CString className = AfxRegisterWndClass(
        CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(nullptr, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        nullptr);

    // Use WS_EX_APPWINDOW so this is treated as a normal app window (not a tool window).
    DWORD dwExStyle = WS_EX_APPWINDOW;
    // Use WS_OVERLAPPEDWINDOW (or WS_POPUPWINDOW|WS_CAPTION|WS_SYSMENU) for a regular top-level window
    DWORD dwStyle = WS_OVERLAPPEDWINDOW/* | WS_VISIBLE*/;

    HWND hOwner = (pOwner != nullptr) ? pOwner->GetSafeHwnd() : nullptr;
    m_hPopoutOwner = hOwner;

    BOOL ok = CreateEx(
        dwExStyle,
        className,
        _T("Pier Layout"),
        dwStyle,
        CRect(100, 100, 1000, 800),
        CWnd::FromHandle(hOwner), // supply the owner window if provided
        0);

    if (!ok)
        return FALSE;

    // Initialize control after creation
    CustomInit(pierData);

    // Optionally keep the popout above the owner without making it topmost for all apps:
    // if (hOwner) SetWindowPos(hOwner, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);

    ShowWindow(SW_SHOW);
    UpdateWindow();

    return TRUE;
}

void CDrawPierLayoutControl::OnPaint()
{

	CDisplayWnd::OnPaint();

}

void CDrawPierLayoutControl::OnDestroy()
{
    // The native owner can be the property sheet, rather than the layout page.
    if (::IsWindow(m_hPopoutOwner))
        ::SendMessage(m_hPopoutOwner, WM_PIER_LAYOUT_POPOUT_CLOSED, 0, 0);
    m_hPopoutOwner = nullptr;
    CDisplayWnd::OnDestroy();
}

void CDrawPierLayoutControl::GetXBeamProfile(pgsTypes::Stage stage, IShape** shape) const
{
    m_CrossBeam->get_Profile(stage == pgsTypes::Stage1 ? 0 : 1, shape);
}

void CDrawPierLayoutControl::GetXBeamShape(pgsTypes::Stage stage, Float64 Xxb, IShape** shape) const
{
    m_CrossBeam->get_Shape(stage == pgsTypes::Stage1 ? 0 : 1, Xxb, shape);
}

Float64 CDrawPierLayoutControl::GetBottomY(Float64 Xxb) const
{
    CComPtr<IShape> shape;
    GetXBeamShape(pgsTypes::Stage1, Xxb, &shape);
    CComQIPtr<IXYPosition> position(shape);
    CComPtr<IPoint2d> point;
    position->get_LocatorPoint(lpBottomCenter, &point);
    Float64 value;
    point->get_Y(&value);
    return value;
}

Float64 CDrawPierLayoutControl::GetMaxColumnHeight() const
{
    Float64 height = 0.0;
    for (IndexType idx = 0; idx < m_PierData.GetColumnCount(); ++idx)
        height = Max(height, GetTopColumnElevation(idx) - GetBottomColumnElevation(idx));
    return height;
}

Float64 CDrawPierLayoutControl::ConvertCrossBeamToPierCoordinate(Float64 coordinate) const
{
    Float64 value;
    m_PierModel->ConvertCrossBeamToPierCoordinate(coordinate, &value);
    return value;
}

Float64 CDrawPierLayoutControl::ConvertPierToCrossBeamCoordinate(Float64 coordinate) const
{
    Float64 value;
    m_PierModel->ConvertPierToCrossBeamCoordinate(coordinate, &value);
    return value;
}

Float64 CDrawPierLayoutControl::ConvertPierToCurbLineCoordinate(Float64 coordinate) const
{
    Float64 value;
    m_PierModel->ConvertPierToCurbLineCoordinate(coordinate, &value);
    return value;
}

Float64 CDrawPierLayoutControl::GetElevation(Float64 coordinate) const
{
    Float64 value;
    m_PierModel->get_Elevation(coordinate, &value);
    return value;
}

Float64 CDrawPierLayoutControl::GetColumnLocation(IndexType index) const
{
    Float64 value;
    m_Columns->get_ColumnLocation(index, &value);
    return value;
}

Float64 CDrawPierLayoutControl::GetTopColumnElevation(IndexType index) const
{
    Float64 value;
    m_Columns->get_TopColumnElevation(index, &value);
    return value;
}

Float64 CDrawPierLayoutControl::GetBottomColumnElevation(IndexType index) const
{
    Float64 value;
    m_Columns->get_BottomColumnElevation(index, &value);
    return value;
}

void CDrawPierLayoutControl::OnSize(UINT nType, int cx, int cy)
{
    CDisplayWnd::OnSize(nType, cx, cy);
    if (cx > 0 && cy > 0 && m_CrossBeam)
    {
        CDisplayWnd::CustomInit();
        ScaleToFit();
    }
}
