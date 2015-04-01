// PierLayoutPat.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ProjectAgent.h"
#include "PierLayoutPage.h"
#include "PierDlg.h"

#include <EAF\EAFDisplayUnits.h>
#include <MFCTools\CustomDDX.h>

#include <PgsExt\GirderLabel.h>

// CPierLayoutPage dialog


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage property page

IMPLEMENT_DYNCREATE(CPierLayoutPage, CPropertyPage)

CPierLayoutPage::CPierLayoutPage() : CPropertyPage(CPierLayoutPage::IDD)
{
	//{{AFX_DATA_INIT(CPierLayoutPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPierLayoutPage::~CPierLayoutPage()
{
}

//void CPierLayoutPage::Init(CPierData2* pPier)
//{
//   m_pPier = pPier;
//
//   m_PierIdx = pPier->GetIndex();
//}

void CPierLayoutPage::DoDataExchange(CDataExchange* pDX)
{
  
   CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPierConnectionsPage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

   CComPtr<IBroker> pBroker;
   EAFGetBroker(&pBroker);
   GET_IFACE2(pBroker,IEAFDisplayUnits,pDisplayUnits);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_MetaFileStatic(pDX, IDC_PIER_LAYOUT, m_LayoutPicture,_T("PIERLAYOUT"), _T("Metafile") );
   DDX_Control(pDX, IDC_S, m_SpacingControl);

   // Transverse location of the pier
   DDX_CBIndex(pDX,IDC_REFCOLUMN,pParent->m_PierData.m_RefColumnIdx);
   DDX_OffsetAndTag(pDX,IDC_X5,IDC_X5_UNIT,pParent->m_PierData.m_TransverseOffset, pDisplayUnits->GetSpanLengthUnit() );
   DDX_CBItemData(pDX,IDC_X5_MEASUREMENT,pParent->m_PierData.m_TransverseOffsetMeasurement);

   DDX_UnitValueAndTag(pDX,IDC_H1,IDC_H1_UNIT,pParent->m_PierData.m_XBeamHeight[pgsTypes::pstLeft],pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_H2,IDC_H2_UNIT,pParent->m_PierData.m_XBeamTaperHeight[pgsTypes::pstLeft],pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X1,IDC_X1_UNIT,pParent->m_PierData.m_XBeamTaperLength[pgsTypes::pstLeft],pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_H3,IDC_H3_UNIT,pParent->m_PierData.m_XBeamHeight[pgsTypes::pstRight],pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_H4,IDC_H4_UNIT,pParent->m_PierData.m_XBeamTaperHeight[pgsTypes::pstRight],pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X2,IDC_X2_UNIT,pParent->m_PierData.m_XBeamTaperLength[pgsTypes::pstRight],pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_W,IDC_W_UNIT,pParent->m_PierData.m_XBeamWidth,pDisplayUnits->GetSpanLengthUnit() );

   DDX_UnitValueAndTag(pDX,IDC_X3,IDC_X3_UNIT,pParent->m_PierData.m_XBeamOverhang[pgsTypes::pstLeft], pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_X4,IDC_X4_UNIT,pParent->m_PierData.m_XBeamOverhang[pgsTypes::pstRight],pDisplayUnits->GetSpanLengthUnit() );

   DDX_Text(pDX,IDC_COLUMN_COUNT,pParent->m_PierData.m_nColumns);
   DDX_UnitValueAndTag(pDX,IDC_S,IDC_S_UNIT,pParent->m_PierData.m_ColumnSpacing,pDisplayUnits->GetSpanLengthUnit());
   DDX_UnitValueAndTag(pDX,IDC_H,IDC_H_UNIT,pParent->m_PierData.m_ColumnHeight,pDisplayUnits->GetSpanLengthUnit());
   DDX_CBItemData(pDX,IDC_HEIGHT_MEASURE,pParent->m_PierData.m_ColumnHeightMeasurementType);

   DDX_CBItemData(pDX,IDC_COLUMN_SHAPE,pParent->m_PierData.m_ColumnShape);
   DDX_UnitValueAndTag(pDX,IDC_B,IDC_B_UNIT,pParent->m_PierData.m_B,pDisplayUnits->GetSpanLengthUnit() );
   DDX_UnitValueAndTag(pDX,IDC_D,IDC_D_UNIT,pParent->m_PierData.m_D,pDisplayUnits->GetSpanLengthUnit() );

   if ( pDX->m_bSaveAndValidate )
   {
#pragma Reminder("WOKRING HERE - need to validate overall pier geometry")
      // maybe do this on the parent property page...
      // XBeam needs to be long enough to support all girders
      // XBeam needs to be as wide as columns(? not necessarily)
   }
}

BEGIN_MESSAGE_MAP(CPierLayoutPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPierLayoutPage)
   ON_CBN_SELCHANGE(IDC_COLUMN_SHAPE, OnColumnShapeChanged)
   ON_NOTIFY(UDN_DELTAPOS, IDC_COLUMN_COUNT_SPINNER, OnColumnCountChanged)
	ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPierLayoutPage message handlers

BOOL CPierLayoutPage::OnInitDialog() 
{
   FillTransverseLocationComboBox();
   FillRefColumnComboBox();
   FillHeightMeasureComboBox();
   FillColumnShapeComboBox();

   CSpinButtonCtrl* pSpinner = (CSpinButtonCtrl*)GetDlgItem(IDC_COLUMN_COUNT_SPINNER);
   pSpinner->SetRange(1,UD_MAXVAL);

   CPropertyPage::OnInitDialog();

   OnColumnShapeChanged();
   UpdateColumnSpacingControls();

   return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPierLayoutPage::FillTransverseLocationComboBox()
{
   CComboBox* pcbMeasure = (CComboBox*)GetDlgItem(IDC_X5_MEASUREMENT);
   pcbMeasure->ResetContent();
   int idx = pcbMeasure->AddString(_T("from the Alignment"));
   pcbMeasure->SetItemData(idx,(DWORD_PTR)pgsTypes::omtAlignment);
   idx = pcbMeasure->AddString(_T("from the Bridgeline"));
   pcbMeasure->SetItemData(idx,(DWORD_PTR)pgsTypes::omtBridge);
}

void CPierLayoutPage::FillRefColumnComboBox()
{
   CPierDlg* pParent = (CPierDlg*)GetParent();
   CComboBox* pcbRefColumn = (CComboBox*)GetDlgItem(IDC_REFCOLUMN);
   int curSel = pcbRefColumn->GetCurSel();
   pcbRefColumn->ResetContent();
   for ( ColumnIndexType colIdx = 0; colIdx < pParent->m_PierData.m_nColumns; colIdx++ )
   {
      CString strLabel;
      strLabel.Format(_T("Column %d"),LABEL_COLUMN(colIdx));
      pcbRefColumn->AddString(strLabel);
   }

   if ( pcbRefColumn->SetCurSel(curSel) == CB_ERR )
   {
      pcbRefColumn->SetCurSel(0);
   }
}

void CPierLayoutPage::FillHeightMeasureComboBox()
{
   CComboBox* pcbHeightMeasure = (CComboBox*)GetDlgItem(IDC_HEIGHT_MEASURE);
   pcbHeightMeasure->ResetContent();
   int idx = pcbHeightMeasure->AddString(_T("Column Height (H)"));
   pcbHeightMeasure->SetItemData(idx,(DWORD_PTR)CColumnData::chtHeight);
   idx = pcbHeightMeasure->AddString(_T("Bottom Elevation"));
   pcbHeightMeasure->SetItemData(idx,(DWORD_PTR)CColumnData::chtBottomElevation);
}

void CPierLayoutPage::FillColumnShapeComboBox()
{
   CComboBox* pcbColumnShape = (CComboBox*)GetDlgItem(IDC_COLUMN_SHAPE);
   pcbColumnShape->ResetContent();
   int idx = pcbColumnShape->AddString(_T("Circle"));
   pcbColumnShape->SetItemData(idx,(DWORD_PTR)CColumnData::cstCircle);
   idx = pcbColumnShape->AddString(_T("Rectangle"));
   pcbColumnShape->SetItemData(idx,(DWORD_PTR)CColumnData::cstRectangle);
}

void CPierLayoutPage::OnHelp() 
{
#pragma Reminder("UPDATE: Update the help context id")
   //::HtmlHelp( *this, AfxGetApp()->m_pszHelpFilePath, HH_HELP_CONTEXT, IDH_PIERDETAILS_CONNECTIONS );
}

void CPierLayoutPage::OnColumnShapeChanged()
{
   CComboBox* pcbColumnShape = (CComboBox*)GetDlgItem(IDC_COLUMN_SHAPE);
   int curSel = pcbColumnShape->GetCurSel();
   CColumnData::ColumnShapeType shapeType = (CColumnData::ColumnShapeType)pcbColumnShape->GetItemData(curSel);
   if ( shapeType == CColumnData::cstCircle )
   {
      GetDlgItem(IDC_B_LABEL)->SetWindowText(_T("R"));
      GetDlgItem(IDC_D_LABEL)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_D)->ShowWindow(SW_HIDE);
      GetDlgItem(IDC_D_UNIT)->ShowWindow(SW_HIDE);
   }
   else
   {
      GetDlgItem(IDC_B_LABEL)->SetWindowText(_T("B"));
      GetDlgItem(IDC_D_LABEL)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_D)->ShowWindow(SW_SHOW);
      GetDlgItem(IDC_D_UNIT)->ShowWindow(SW_SHOW);
   }
}

void CPierLayoutPage::OnColumnCountChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

   // this is what the count will be
   int new_count = pNMUpDown->iPos + pNMUpDown->iDelta;

   CPierDlg* pParent = (CPierDlg*)GetParent();
   pParent->m_PierData.m_nColumns = new_count;

   *pResult = 0;

   FillRefColumnComboBox();
   UpdateColumnSpacingControls();
}

void CPierLayoutPage::UpdateColumnSpacingControls()
{
   CPierDlg* pParent = (CPierDlg*)GetParent();
   BOOL bEnable = (1 < pParent->m_PierData.m_nColumns ? TRUE : FALSE);
   GetDlgItem(IDC_S_LABEL)->EnableWindow(bEnable);
   m_SpacingControl.EnableWindow(bEnable);
   GetDlgItem(IDC_S_UNIT)->EnableWindow(bEnable);
}
