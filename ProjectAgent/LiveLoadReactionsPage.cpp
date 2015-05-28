// LiveLoadReactionsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LiveLoadReactionsPage.h"
#include "PierDlg.h"

void DDX_LiveLoadReactionsGrid(CDataExchange* pDX,CLiveLoadReactionGrid& grid,txnLiveLoadReactions& llData)
{
   if ( pDX->m_bSaveAndValidate )
   {
      grid.GetLiveLoadData(llData);
   }
   else
   {
      grid.SetLiveLoadData(llData);
   }
}


// CLiveLoadReactionsPage dialog

IMPLEMENT_DYNAMIC(CLiveLoadReactionsPage, CPropertyPage)

CLiveLoadReactionsPage::CLiveLoadReactionsPage()
	: CPropertyPage(CLiveLoadReactionsPage::IDD)
{

}

CLiveLoadReactionsPage::~CLiveLoadReactionsPage()
{
}

void CLiveLoadReactionsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

   CPierDlg* pParent = (CPierDlg*)GetParent();

   DDX_Text(pDX,IDC_LF_INVENTORY,pParent->m_PierData.m_gLL[pgsTypes::lrDesign_Inventory]);
   DDX_Text(pDX,IDC_LF_OPERATING,pParent->m_PierData.m_gLL[pgsTypes::lrDesign_Operating]);
   DDX_Text(pDX,IDC_LF_LEGAL_ROUTINE,pParent->m_PierData.m_gLL[pgsTypes::lrLegal_Routine]);
   DDX_Text(pDX,IDC_LF_LEGAL_SPECIAL,pParent->m_PierData.m_gLL[pgsTypes::lrLegal_Special]);
   DDX_Text(pDX,IDC_LF_PERMIT_ROUTINE,pParent->m_PierData.m_gLL[pgsTypes::lrPermit_Routine]);
   DDX_Text(pDX,IDC_LF_PERMIT_SPECIAL,pParent->m_PierData.m_gLL[pgsTypes::lrPermit_Special]);

   DDX_LiveLoadReactionsGrid(pDX,m_DesignGrid,        pParent->m_PierData.m_DesignLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX,m_LegalRoutineGrid,  pParent->m_PierData.m_LegalRoutineLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX,m_LegalSpecialGrid,  pParent->m_PierData.m_LegalSpecialLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX,m_PermitRoutineGrid, pParent->m_PierData.m_PermitRoutineLiveLoad);
   DDX_LiveLoadReactionsGrid(pDX,m_PermitSpecialGrid, pParent->m_PierData.m_PermitSpecialLiveLoad);

}


BEGIN_MESSAGE_MAP(CLiveLoadReactionsPage, CPropertyPage)
   ON_BN_CLICKED(IDC_DESIGN_ADD,OnDesignAdd)
   ON_BN_CLICKED(IDC_DESIGN_REMOVE,OnDesignRemove)
   ON_BN_CLICKED(IDC_LEGAL_ROUTINE_ADD,OnLegalRoutineAdd)
   ON_BN_CLICKED(IDC_LEGAL_ROUTINE_REMOVE,OnLegalRoutineRemove)
   ON_BN_CLICKED(IDC_LEGAL_SPECIAL_ADD,OnLegalSpecialAdd)
   ON_BN_CLICKED(IDC_LEGAL_SPECIAL_REMOVE,OnLegalSpecialRemove)
   ON_BN_CLICKED(IDC_PERMIT_ROUTINE_ADD,OnPermitRoutineAdd)
   ON_BN_CLICKED(IDC_PERMIT_ROUTINE_REMOVE,OnPermitRoutineRemove)
   ON_BN_CLICKED(IDC_PERMIT_SPECIAL_ADD,OnPermitSpecialAdd)
   ON_BN_CLICKED(IDC_PERMIT_SPECIAL_REMOVE,OnPermitSpecialRemove)
END_MESSAGE_MAP()


// CLiveLoadReactionsPage message handlers

BOOL CLiveLoadReactionsPage::OnInitDialog()
{
   m_DesignGrid.SubclassDlgItem(IDC_DESIGN_GRID, this);
   m_DesignGrid.CustomInit();

   m_LegalRoutineGrid.SubclassDlgItem(IDC_LEGAL_ROUTINE_GRID, this);
   m_LegalRoutineGrid.CustomInit();

   m_LegalSpecialGrid.SubclassDlgItem(IDC_LEGAL_SPECIAL_GRID, this);
   m_LegalSpecialGrid.CustomInit();

   m_PermitRoutineGrid.SubclassDlgItem(IDC_PERMIT_ROUTINE_GRID, this);
   m_PermitRoutineGrid.CustomInit();

   m_PermitSpecialGrid.SubclassDlgItem(IDC_PERMIT_SPECIAL_GRID, this);
   m_PermitSpecialGrid.CustomInit();

   CPropertyPage::OnInitDialog();

   // TODO:  Add extra initialization here

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

void CLiveLoadReactionsPage::OnDesignAdd()
{
   m_DesignGrid.AddVehicle();
}

void CLiveLoadReactionsPage::OnDesignRemove()
{
   m_DesignGrid.RemoveSelectedVehicles();
}

void CLiveLoadReactionsPage::OnLegalRoutineAdd()
{
   m_LegalRoutineGrid.AddVehicle();
}

void CLiveLoadReactionsPage::OnLegalRoutineRemove()
{
   m_LegalRoutineGrid.RemoveSelectedVehicles();
}

void CLiveLoadReactionsPage::OnLegalSpecialAdd()
{
   m_LegalSpecialGrid.AddVehicle();
}

void CLiveLoadReactionsPage::OnLegalSpecialRemove()
{
   m_LegalSpecialGrid.RemoveSelectedVehicles();
}

void CLiveLoadReactionsPage::OnPermitRoutineAdd()
{
   m_PermitRoutineGrid.AddVehicle();
}

void CLiveLoadReactionsPage::OnPermitRoutineRemove()
{
   m_PermitRoutineGrid.RemoveSelectedVehicles();
}

void CLiveLoadReactionsPage::OnPermitSpecialAdd()
{
   m_PermitSpecialGrid.AddVehicle();
}

void CLiveLoadReactionsPage::OnPermitSpecialRemove()
{
   m_PermitSpecialGrid.RemoveSelectedVehicles();
}
