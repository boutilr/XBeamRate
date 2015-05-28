// XBeamRateView.cpp : implementation of the CXBeamRateView class
//

#include "stdafx.h"

#include "XBeamRateDoc.h"
#include "XBeamRateView.h"

#include <IFace\Project.h>
#include <IFace\AnalysisResults.h>
#include <IFace\LoadRating.h>
#include <IFace\Pier.h>
#include <MFCTools\Format.h>

#include <EAF\EAFDisplayUnits.h>
#include <MFCTools\WsdotCalculationSheet.h>



#include <Colors.h>
#define SELECTED_OBJECT_LINE_COLOR     RED4
#define SELECTED_OBJECT_FILL_COLOR     RED2
#define COLUMN_LINE_COLOR              GREY50
#define COLUMN_FILL_COLOR              GREY70
#define XBEAM_LINE_COLOR               GREY50
#define XBEAM_FILL_COLOR               GREY70
#define REBAR_COLOR                    GREEN

#define COLUMN_LINE_WEIGHT             1
//#define XBEAM_LINE_WEIGHT              3
#define REBAR_LINE_WEIGHT              1

#define BEARING_DISPLAY_LIST_ID        0
#define XBEAM_DISPLAY_LIST_ID          1
#define COLUMN_DISPLAY_LIST_ID         2
#define DIMENSIONS_DISPLAY_LIST_ID     3
#define REBAR_DISPLAY_LIST_ID          4

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView

IMPLEMENT_DYNCREATE(CXBeamRateView, CDisplayView)

BEGIN_MESSAGE_MAP(CXBeamRateView, CDisplayView)
	//{{AFX_MSG_MAP(CXBeamRateView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
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

BOOL CXBeamRateView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CXBeamRateView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
   // get paper size
   WsdotCalculationSheet border;

   CDocument* pdoc = GetDocument();
   CString path = pdoc->GetPathName();
   border.SetFileName(path);
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
	CDisplayView::AssertValid();
}

void CXBeamRateView::Dump(CDumpContext& dc) const
{
	CDisplayView::Dump(dc);
}

CXBeamRateDoc* CXBeamRateView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CXBeamRateDoc)));
	return (CXBeamRateDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXBeamRateView message handlers
void CXBeamRateView::OnInitialUpdate()
{
   EnableToolTips();

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);
   dispMgr->EnableLBtnSelect(TRUE);
   dispMgr->EnableRBtnSelect(TRUE);
   dispMgr->SetSelectionLineColor(SELECTED_OBJECT_LINE_COLOR);
   dispMgr->SetSelectionFillColor(SELECTED_OBJECT_FILL_COLOR);

   CDisplayView::SetMappingMode(DManip::Isotropic);

   // Setup display lists
   CComPtr<iDisplayList> displayList;
   ::CoCreateInstance(CLSID_DisplayList,NULL,CLSCTX_ALL,IID_iDisplayList,(void**)&displayList);
   displayList->SetID(DIMENSIONS_DISPLAY_LIST_ID);
   dispMgr->AddDisplayList(displayList);

   displayList.Release();
   ::CoCreateInstance(CLSID_DisplayList,NULL,CLSCTX_ALL,IID_iDisplayList,(void**)&displayList);
   displayList->SetID(REBAR_DISPLAY_LIST_ID);
   dispMgr->AddDisplayList(displayList);

   displayList.Release();
   ::CoCreateInstance(CLSID_DisplayList,NULL,CLSCTX_ALL,IID_iDisplayList,(void**)&displayList);
   displayList->SetID(BEARING_DISPLAY_LIST_ID);
   dispMgr->AddDisplayList(displayList);

   displayList.Release();
   ::CoCreateInstance(CLSID_DisplayList,NULL,CLSCTX_ALL,IID_iDisplayList,(void**)&displayList);
   displayList->SetID(XBEAM_DISPLAY_LIST_ID);
   dispMgr->AddDisplayList(displayList);

   displayList.Release();
   ::CoCreateInstance(CLSID_DisplayList,NULL,CLSCTX_ALL,IID_iDisplayList,(void**)&displayList);
   displayList->SetID(COLUMN_DISPLAY_LIST_ID);
   dispMgr->AddDisplayList(displayList);

   CDisplayView::OnInitialUpdate();
}

void CXBeamRateView::OnUpdate(CView* pSender,LPARAM lHint,CObject* pHint)
{
   CDisplayView::OnUpdate(pSender,lHint,pHint);
   UpdateDisplayObjects();
   ScaleToFit();
}

void CXBeamRateView::UpdateDisplayObjects()
{
   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);
   dispMgr->ClearDisplayObjects();

   UpdateXBeamDisplayObjects();
   UpdateColumnDisplayObjects();
   UpdateBearingDisplayObjects();
   UpdateRebarDisplayObjects();
   UpdateDimensionsDisplayObjects();
}

void CXBeamRateView::UpdateXBeamDisplayObjects()
{
   CWaitCursor wait;

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);

   CDManipClientDC dc(this);

   CComPtr<iDisplayList> displayList;
   dispMgr->FindDisplayList(XBEAM_DISPLAY_LIST_ID,&displayList);

   CXBeamRateDoc* pDoc = (CXBeamRateDoc*)GetDocument();
   CComPtr<IBroker> pBroker;
   pDoc->GetBroker(&pBroker);

   GET_IFACE2(pBroker,IXBRProject,pProject);
   GET_IFACE2(pBroker,IXBRSectionProperties,pSectProp);

   // Model a vertical line for the alignment
   // Let X = 0 be at the alignment and Y = the alignment elevation
   Float64 X = 0;
   Float64 Y = pProject->GetDeckElevation() + ::ConvertToSysUnits(1.0,unitMeasure::Feet);
   CComPtr<IPoint2d> pnt1;
   pnt1.CoCreateInstance(CLSID_Point2d);
   pnt1->Move(X,Y);
   CComPtr<iPointDisplayObject> doPnt1;
   doPnt1.CoCreateInstance(CLSID_PointDisplayObject);
   doPnt1->Visible(FALSE);
   doPnt1->SetPosition(pnt1,FALSE,FALSE);
   CComQIPtr<iConnectable> connectable1(doPnt1);
   CComPtr<iSocket> socket1;
   connectable1->AddSocket(0,pnt1,&socket1);

   GET_IFACE2(pBroker,IXBRPier,pPier);
   Y -= pPier->GetMaxColumnHeight();
   CComPtr<IPoint2d> pnt2;
   pnt2.CoCreateInstance(CLSID_Point2d);
   pnt2->Move(X,Y);
   CComPtr<iPointDisplayObject> doPnt2;
   doPnt2.CoCreateInstance(CLSID_PointDisplayObject);
   doPnt2->Visible(FALSE);
   doPnt2->SetPosition(pnt2,FALSE,FALSE);
   CComQIPtr<iConnectable> connectable2(doPnt2);
   CComPtr<iSocket> socket2;
   connectable2->AddSocket(0,pnt2,&socket2);

   CComPtr<iLineDisplayObject> doAlignment;
   doAlignment.CoCreateInstance(CLSID_LineDisplayObject);
   CComPtr<iDrawLineStrategy> drawStrategy;
   doAlignment->GetDrawLineStrategy(&drawStrategy);
   CComQIPtr<iSimpleDrawLineStrategy> drawAlignmentStrategy(drawStrategy);
   drawAlignmentStrategy->SetWidth(3);
   drawAlignmentStrategy->SetColor(RED);
   drawAlignmentStrategy->SetLineStyle(lsCenterline);


   CComQIPtr<iConnector> connector(doAlignment);
   CComPtr<iPlug> startPlug, endPlug;
   connector->GetStartPlug(&startPlug);
   connector->GetEndPlug(&endPlug);
   DWORD dwCookie;
   connectable1->Connect(0,atByID,startPlug,&dwCookie);
   connectable2->Connect(0,atByID,endPlug,  &dwCookie);

   displayList->AddDisplayObject(doAlignment);

   // Model Upper Cross Beam (Elevation)
   Float64 Z = pProject->GetXBeamLength();

   CComPtr<IPoint2d> point;
   point.CoCreateInstance(CLSID_Point2d);
   point->Move(0,0);

   CComPtr<iPointDisplayObject> doUpperXBeam;
   doUpperXBeam.CoCreateInstance(CLSID_PointDisplayObject);
   doUpperXBeam->SetID(m_DisplayObjectID++);
   doUpperXBeam->SetPosition(point,FALSE,FALSE);
   doUpperXBeam->SetSelectionType(stAll);

   CComPtr<IShape> upperXBeamShape;
   pPier->GetUpperXBeamProfile(&upperXBeamShape);

   CComPtr<iShapeDrawStrategy> upperXBeamDrawStrategy;
   upperXBeamDrawStrategy.CoCreateInstance(CLSID_ShapeDrawStrategy);
   upperXBeamDrawStrategy->SetShape(upperXBeamShape);
   upperXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   upperXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   upperXBeamDrawStrategy->DoFill(TRUE);

   doUpperXBeam->SetDrawingStrategy(upperXBeamDrawStrategy);

   CComPtr<iShapeGravityWellStrategy> upper_xbeam_gravity_well;
   upper_xbeam_gravity_well.CoCreateInstance(CLSID_ShapeGravityWellStrategy);
   upper_xbeam_gravity_well->SetShape(upperXBeamShape);
   doUpperXBeam->SetGravityWellStrategy(upper_xbeam_gravity_well);

   displayList->AddDisplayObject(doUpperXBeam);

   // Model Upper XBeam (End View)
   doUpperXBeam.Release();
   doUpperXBeam.CoCreateInstance(CLSID_PointDisplayObject);
   doUpperXBeam->SetID(m_DisplayObjectID++);
   doUpperXBeam->SetPosition(point,FALSE,FALSE);
   doUpperXBeam->SetSelectionType(stAll);

   upperXBeamShape.Release();
   pSectProp->GetUpperXBeamShape(xbrPointOfInterest(INVALID_ID,Z),&upperXBeamShape);
   CComQIPtr<IXYPosition> position(upperXBeamShape);
   position->Offset(1.2*Z,0);

   upperXBeamDrawStrategy.Release();
   upperXBeamDrawStrategy.CoCreateInstance(CLSID_ShapeDrawStrategy);
   upperXBeamDrawStrategy->SetShape(upperXBeamShape);
   upperXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   upperXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   upperXBeamDrawStrategy->DoFill(TRUE);

   doUpperXBeam->SetDrawingStrategy(upperXBeamDrawStrategy);

   upper_xbeam_gravity_well.Release();
   upper_xbeam_gravity_well.CoCreateInstance(CLSID_ShapeGravityWellStrategy);
   upper_xbeam_gravity_well->SetShape(upperXBeamShape);
   doUpperXBeam->SetGravityWellStrategy(upper_xbeam_gravity_well);

   displayList->AddDisplayObject(doUpperXBeam);

   // Model Lower Cross Beam (Elevation)
   CComPtr<iPointDisplayObject> doLowerXBeam;
   doLowerXBeam.CoCreateInstance(CLSID_PointDisplayObject);
   doLowerXBeam->SetID(m_DisplayObjectID++);
   doLowerXBeam->SetPosition(point,FALSE,FALSE);
   doLowerXBeam->SetSelectionType(stAll);

   CComPtr<IShape> lowerXBeamShape;
   pPier->GetLowerXBeamProfile(&lowerXBeamShape);

   CComPtr<iShapeDrawStrategy> lowerXBeamDrawStrategy;
   lowerXBeamDrawStrategy.CoCreateInstance(CLSID_ShapeDrawStrategy);
   lowerXBeamDrawStrategy->SetShape(lowerXBeamShape);
   lowerXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   lowerXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   lowerXBeamDrawStrategy->DoFill(TRUE);

   doLowerXBeam->SetDrawingStrategy(lowerXBeamDrawStrategy);

   CComPtr<iShapeGravityWellStrategy> lower_xbeam_gravity_well;
   lower_xbeam_gravity_well.CoCreateInstance(CLSID_ShapeGravityWellStrategy);
   lower_xbeam_gravity_well->SetShape(lowerXBeamShape);
   doLowerXBeam->SetGravityWellStrategy(lower_xbeam_gravity_well);

   displayList->AddDisplayObject(doLowerXBeam);

   // Model Lower Cross Beam (End)
   doLowerXBeam.Release();
   doLowerXBeam.CoCreateInstance(CLSID_PointDisplayObject);
   doLowerXBeam->SetID(m_DisplayObjectID++);
   doLowerXBeam->SetPosition(point,FALSE,FALSE);
   doLowerXBeam->SetSelectionType(stAll);

   lowerXBeamShape.Release();
   pSectProp->GetLowerXBeamShape(xbrPointOfInterest(INVALID_ID,Z),&lowerXBeamShape);
   position.Release();
   lowerXBeamShape->QueryInterface(&position);
   position->Offset(1.2*Z,0);

   lowerXBeamDrawStrategy.Release();
   lowerXBeamDrawStrategy.CoCreateInstance(CLSID_ShapeDrawStrategy);
   lowerXBeamDrawStrategy->SetShape(lowerXBeamShape);
   lowerXBeamDrawStrategy->SetSolidLineColor(XBEAM_LINE_COLOR);
   lowerXBeamDrawStrategy->SetSolidFillColor(XBEAM_FILL_COLOR);
   lowerXBeamDrawStrategy->DoFill(TRUE);

   doLowerXBeam->SetDrawingStrategy(lowerXBeamDrawStrategy);

   lower_xbeam_gravity_well.Release();
   lower_xbeam_gravity_well.CoCreateInstance(CLSID_ShapeGravityWellStrategy);
   lower_xbeam_gravity_well->SetShape(lowerXBeamShape);
   doLowerXBeam->SetGravityWellStrategy(lower_xbeam_gravity_well);

   displayList->AddDisplayObject(doLowerXBeam);
}

void CXBeamRateView::UpdateColumnDisplayObjects()
{
   CWaitCursor wait;

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);

   CDManipClientDC dc(this);

   CComPtr<iDisplayList> displayList;
   dispMgr->FindDisplayList(COLUMN_DISPLAY_LIST_ID,&displayList);

   CXBeamRateDoc* pDoc = (CXBeamRateDoc*)GetDocument();
   CComPtr<IBroker> pBroker;
   pDoc->GetBroker(&pBroker);

   GET_IFACE2(pBroker,IXBRPier,pPier);
   GET_IFACE2(pBroker,IXBRProject,pProject);

   CComPtr<IPoint2d> pntTL, pntTC, pntTR;
   CComPtr<IPoint2d> pntBL, pntBC, pntBR;
   pPier->GetUpperXBeamPoints(&pntTL,&pntTC,&pntTR,&pntBL,&pntBC,&pntBR);

   Float64 Xoffset;
   pntTL->get_X(&Xoffset);

   IndexType nColumns = pPier->GetColumnCount();
   for (IndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      Float64 X = pPier->GetColumnLocation(colIdx);
      Float64 Ytop = pPier->GetTopColumnElevation(colIdx);
      Float64 Ybot = pPier->GetBottomColumnElevation(colIdx);
      CColumnData::ColumnShapeType colShapeType;
      Float64 d1, d2;
      CColumnData::ColumnHeightMeasurementType columnHeightType;
      Float64 H;
      pProject->GetColumnProperties(&colShapeType,&d1,&d2,&columnHeightType,&H);

      CComPtr<IPoint2d> pntTop, pntBot;
      pntTop.CoCreateInstance(CLSID_Point2d);
      pntTop->Move(X+Xoffset,Ytop);

      pntBot.CoCreateInstance(CLSID_Point2d);
      pntBot->Move(X+Xoffset,Ybot);
   
      CComPtr<iPointDisplayObject> doTop;
      doTop.CoCreateInstance(CLSID_PointDisplayObject);
      doTop->Visible(FALSE);
      doTop->SetPosition(pntTop,FALSE,FALSE);
      CComQIPtr<iConnectable> connectable1(doTop);
      CComPtr<iSocket> socket1;
      connectable1->AddSocket(0,pntTop,&socket1);

      CComPtr<iPointDisplayObject> doBot;
      doBot.CoCreateInstance(CLSID_PointDisplayObject);
      doBot->Visible(FALSE);
      doBot->SetPosition(pntBot,FALSE,FALSE);
      CComQIPtr<iConnectable> connectable2(doBot);
      CComPtr<iSocket> socket2;
      connectable2->AddSocket(0,pntBot,&socket2);

      CComPtr<iLineDisplayObject> doColumn;
      doColumn.CoCreateInstance(CLSID_LineDisplayObject);
      doColumn->SetID(m_DisplayObjectID++);
      doColumn->SetSelectionType(stAll);

      CComPtr<iRectangleDrawLineStrategy> drawColumnStrategy;
      drawColumnStrategy.CoCreateInstance(CLSID_RectangleDrawLineStrategy);
      doColumn->SetDrawLineStrategy(drawColumnStrategy);

      drawColumnStrategy->SetWidth(d1);
      drawColumnStrategy->SetColor(COLUMN_LINE_COLOR);
      drawColumnStrategy->SetLineWidth(COLUMN_LINE_WEIGHT);
      drawColumnStrategy->SetFillColor(COLUMN_FILL_COLOR);
      drawColumnStrategy->SetDoFill(TRUE);

      drawColumnStrategy->PerimeterGravityWell(TRUE);
      CComQIPtr<iGravityWellStrategy> gravity_well(drawColumnStrategy);
      doColumn->SetGravityWellStrategy(gravity_well);


      CComQIPtr<iConnector> connector(doColumn);
      CComPtr<iPlug> startPlug, endPlug;
      connector->GetStartPlug(&startPlug);
      connector->GetEndPlug(&endPlug);
      DWORD dwCookie;
      connectable1->Connect(0,atByID,startPlug,&dwCookie);
      connectable2->Connect(0,atByID,endPlug,  &dwCookie);

      displayList->AddDisplayObject(doColumn);
   }
}

void CXBeamRateView::UpdateRebarDisplayObjects()
{
   CWaitCursor wait;

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);

   CDManipClientDC dc(this);

   CComPtr<iDisplayList> displayList;
   dispMgr->FindDisplayList(REBAR_DISPLAY_LIST_ID,&displayList);

   CXBeamRateDoc* pDoc = (CXBeamRateDoc*)GetDocument();
   CComPtr<IBroker> pBroker;
   pDoc->GetBroker(&pBroker);

   GET_IFACE2(pBroker,IXBRProject,pProject);
   Float64 X = pProject->GetXBeamLength();

   GET_IFACE2(pBroker,IXBRRebar,pRebar);
   IndexType nRebarRows = pRebar->GetRebarRowCount();
   for ( IndexType rowIdx = 0; rowIdx < nRebarRows; rowIdx++ )
   {
      CComPtr<IPoint2dCollection> points;
      pRebar->GetRebarProfile(rowIdx,&points);

      CComPtr<iPolyLineDisplayObject> doRebar;
      doRebar.CoCreateInstance(CLSID_PolyLineDisplayObject);
      doRebar->SetID(m_DisplayObjectID++);
      doRebar->AddPoints(points);
      doRebar->put_PointType(plpNone);
      doRebar->put_Color(REBAR_COLOR);
      doRebar->put_Width(REBAR_LINE_WEIGHT);
      doRebar->Commit();
      displayList->AddDisplayObject(doRebar);

      IndexType nBars = pRebar->GetRebarCount(rowIdx);
      for ( IndexType barIdx = 0; barIdx < nBars; barIdx++ )
      {
         CComPtr<IPoint2d> pntBar;
         pRebar->GetRebarLocation(X,rowIdx,barIdx,&pntBar);

         pntBar->Offset(1.2*X,0);

         CComPtr<iPointDisplayObject> doBar;
         doBar.CoCreateInstance(CLSID_PointDisplayObject);
         doBar->SetID(m_DisplayObjectID++);
         doBar->SetPosition(pntBar,FALSE,FALSE);
         

         displayList->AddDisplayObject(doBar);
      }
   }
}

void CXBeamRateView::UpdateBearingDisplayObjects()
{
   CWaitCursor wait;

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);

   CDManipClientDC dc(this);

   CComPtr<iDisplayList> displayList;
   dispMgr->FindDisplayList(BEARING_DISPLAY_LIST_ID,&displayList);

   CXBeamRateDoc* pDoc = (CXBeamRateDoc*)GetDocument();
   CComPtr<IBroker> pBroker;
   pDoc->GetBroker(&pBroker);

   GET_IFACE2(pBroker,IXBRProject,pProject);

   Float64 H,W;
   pProject->GetDiaphragmDimensions(&H,&W);

   GET_IFACE2(pBroker,IXBRPier,pPier);

   CComPtr<IPoint2d> pntTL, pntTC, pntTR;
   CComPtr<IPoint2d> pntBL, pntBC, pntBR;
   pPier->GetUpperXBeamPoints(&pntTL,&pntTC,&pntTR,&pntBL,&pntBC,&pntBR);

   Float64 Xoffset;
   pntTL->get_X(&Xoffset);

   IndexType nBearingLines = pPier->GetBearingLineCount();
   for ( IndexType brgLineIdx = 0; brgLineIdx < nBearingLines; brgLineIdx++ )
   {
      IndexType nBearings = pPier->GetBearingCount(brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         Float64 Xbrg = pPier->GetBearingLocation(brgLineIdx,brgIdx);
         Float64 Y    = pPier->GetElevation(Xbrg);

         CComPtr<IPoint2d> pnt;
         pnt.CoCreateInstance(CLSID_Point2d);
         pnt->Move(Xbrg+Xoffset,Y-H);

         CComPtr<iPointDisplayObject> doPnt;
         doPnt.CoCreateInstance(CLSID_PointDisplayObject);
         doPnt->SetID(m_DisplayObjectID++);
         doPnt->SetPosition(pnt,FALSE,FALSE);

         CComPtr<iDrawPointStrategy> strategy;
         doPnt->GetDrawingStrategy(&strategy);

         CComQIPtr<iSimpleDrawPointStrategy> theStrategy(strategy);
         theStrategy->SetPointType(ptSquare);
         theStrategy->SetColor(BLACK);

         displayList->AddDisplayObject(doPnt);
      }
   }
}

void CXBeamRateView::UpdateDimensionsDisplayObjects()
{
   CWaitCursor wait;

   CComPtr<iDisplayMgr> dispMgr;
   GetDisplayMgr(&dispMgr);

   CDManipClientDC dc(this);

   CComPtr<iDisplayList> displayList;
   dispMgr->FindDisplayList(DIMENSIONS_DISPLAY_LIST_ID,&displayList);

   CXBeamRateDoc* pDoc = (CXBeamRateDoc*)GetDocument();
   CComPtr<IBroker> pBroker;
   pDoc->GetBroker(&pBroker);


   GET_IFACE2(pBroker,IXBRPier,pPier);
   CComPtr<IPoint2d> uxbTL, uxbTC, uxbTR;
   CComPtr<IPoint2d> uxbBL, uxbBC, uxbBR;
   pPier->GetUpperXBeamPoints(&uxbTL,&uxbTC,&uxbTR,&uxbBL,&uxbBC,&uxbBR);

   CComPtr<IPoint2d> lxbTL, lxbTC, lxbTR;
   CComPtr<IPoint2d> lxbBL, lxbBL2, lxbBR2, lxbBR;
   pPier->GetLowerXBeamPoints(&lxbTL,&lxbTC,&lxbTR,&lxbBL,&lxbBL2,&lxbBR2,&lxbBR);

   CComPtr<IPoint2d> uxbTLC, uxbTRC;
   uxbTLC.CoCreateInstance(CLSID_Point2d);
   uxbTRC.CoCreateInstance(CLSID_Point2d);
   Float64 x1,y1;
   uxbTL->Location(&x1,&y1);
   Float64 x2,y2;
   uxbTR->Location(&x2,&y2);
   uxbTLC->Move(x1,Max(y1,y2));
   uxbTRC->Move(x2,Max(y1,y2));

   CComPtr<IPoint2d> lxbBLC, lxbBRC;
   lxbBLC.CoCreateInstance(CLSID_Point2d);
   lxbBRC.CoCreateInstance(CLSID_Point2d);
   Float64 x,y;
   lxbBL->get_X(&x);
   lxbBL2->get_Y(&y);
   lxbBLC->Move(x,y);

   lxbBR->get_X(&x);
   lxbBR2->get_Y(&y);
   lxbBRC->Move(x,y);

   // Horizontal Cross Beam Dimensions
   BuildDimensionLine(displayList,uxbTLC,uxbTRC);
   BuildDimensionLine(displayList,lxbBL2,lxbBLC);
   BuildDimensionLine(displayList,lxbBRC,lxbBR2);

   // Left Side of Cross Beam

   // Height of upper cross beam
   BuildDimensionLine(displayList,uxbBL,uxbTL);
   BuildDimensionLine(displayList,uxbTR,uxbBR);

   // Height of lower cross beam
   BuildDimensionLine(displayList,lxbBL,lxbTL);
   BuildDimensionLine(displayList,lxbTR,lxbBR);

   BuildDimensionLine(displayList,lxbBLC,lxbBL);
   BuildDimensionLine(displayList,lxbBR,lxbBRC);

   // Column Dimensions
   ColumnIndexType nColumns = pPier->GetColumnCount();
   lxbBRC->get_X(&x);
   Float64 Ycol = pPier->GetBottomColumnElevation(nColumns-1);

   CComPtr<IPoint2d> pntBC;
   pntBC.CoCreateInstance(CLSID_Point2d);
   pntBC->Move(x,Ycol);
   BuildDimensionLine(displayList,lxbBRC,pntBC);

   Float64 Xoffset;
   uxbTL->get_X(&Xoffset);
   CComPtr<IPoint2d> pntLeft;
   pntLeft.CoCreateInstance(CLSID_Point2d);
   pntLeft->Move(Xoffset,Ycol);
   for ( ColumnIndexType colIdx = 0; colIdx < nColumns; colIdx++ )
   {
      Float64 Xcol = pPier->GetColumnLocation(colIdx);
      Xcol += Xoffset;

      CComPtr<IPoint2d> pntRight;
      pntRight.CoCreateInstance(CLSID_Point2d);
      pntRight->Move(Xcol,Ycol);
     
      BuildDimensionLine(displayList,pntRight,pntLeft);

      pntLeft = pntRight;
   }

   uxbTR->get_X(&x);
   CComPtr<IPoint2d> pntRight;
   pntRight.CoCreateInstance(CLSID_Point2d);
   pntRight->Move(x,Ycol);
   BuildDimensionLine(displayList,pntRight,pntLeft);
}

void CXBeamRateView::BuildDimensionLine(iDisplayList* pDL, IPoint2d* fromPoint,IPoint2d* toPoint,iDimensionLine** ppDimLine)
{
   Float64 distance;
   toPoint->DistanceEx(fromPoint,&distance);
   if ( !IsZero(distance) )
   {
      BuildDimensionLine(pDL,fromPoint,toPoint,distance,ppDimLine);
   }
}

void CXBeamRateView::BuildDimensionLine(iDisplayList* pDL, IPoint2d* fromPoint,IPoint2d* toPoint,Float64 dimension,iDimensionLine** ppDimLine)
{
   // put points at locations and make them sockets
   CComPtr<iPointDisplayObject> from_rep;
   ::CoCreateInstance(CLSID_PointDisplayObject,NULL,CLSCTX_ALL,IID_iPointDisplayObject,(void**)&from_rep);
   from_rep->SetPosition(fromPoint,FALSE,FALSE);
   from_rep->SetID(m_DisplayObjectID++);
   CComQIPtr<iConnectable,&IID_iConnectable> from_connectable(from_rep);
   CComPtr<iSocket> from_socket;
   from_connectable->AddSocket(0,fromPoint,&from_socket);
   from_rep->Visible(FALSE);
   pDL->AddDisplayObject(from_rep);

   CComPtr<iPointDisplayObject> to_rep;
   ::CoCreateInstance(CLSID_PointDisplayObject,NULL,CLSCTX_ALL,IID_iPointDisplayObject,(void**)&to_rep);
   to_rep->SetPosition(toPoint,FALSE,FALSE);
   to_rep->SetID(m_DisplayObjectID++);
   CComQIPtr<iConnectable,&IID_iConnectable> to_connectable(to_rep);
   CComPtr<iSocket> to_socket;
   to_connectable->AddSocket(0,toPoint,&to_socket);
   to_rep->Visible(FALSE);
   pDL->AddDisplayObject(to_rep);

   // Create the dimension line object
   CComPtr<iDimensionLine> dimLine;
   ::CoCreateInstance(CLSID_DimensionLineDisplayObject,NULL,CLSCTX_ALL,IID_iDimensionLine,(void**)&dimLine);

   dimLine->SetArrowHeadStyle(DManip::ahsFilled);

   // Attach connector (the dimension line) to the sockets 
   CComPtr<iConnector> connector;
   dimLine->QueryInterface(IID_iConnector,(void**)&connector);
   CComPtr<iPlug> startPlug;
   CComPtr<iPlug> endPlug;
   connector->GetStartPlug(&startPlug);
   connector->GetEndPlug(&endPlug);

   DWORD dwCookie;
   from_socket->Connect(startPlug,&dwCookie);
   to_socket->Connect(endPlug,&dwCookie);

   // Create the text block and attach it to the dimension line
   CComPtr<iTextBlock> textBlock;
   ::CoCreateInstance(CLSID_TextBlock,NULL,CLSCTX_ALL,IID_iTextBlock,(void**)&textBlock);

   // Format the dimension text
   CComPtr<IBroker> pBroker;
   EAFGetBroker(&pBroker);
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);
   CString strDimension = FormatDimension(dimension,pDisplayUnits->GetSpanLengthUnit());

   textBlock->SetText(strDimension);
   textBlock->SetBkMode(TRANSPARENT);

   dimLine->SetTextBlock(textBlock);

   // Assign the span id to the dimension line (so they are the same)
   dimLine->SetID(m_DisplayObjectID++);

   pDL->AddDisplayObject(dimLine);

   if ( ppDimLine )
   {
      (*ppDimLine) = dimLine;
      (*ppDimLine)->AddRef();
   }
}

void CXBeamRateView::HandleLButtonDblClk(UINT nFlags, CPoint logPoint)
{
   CDisplayView::HandleLButtonDblClk(nFlags,logPoint);
   CComPtr<IBroker> pBroker;
   EAFGetBroker(&pBroker);

   GET_IFACE2(pBroker,IXBRProjectEdit,pProjectEdit);
   pProjectEdit->EditPier(0);
}
