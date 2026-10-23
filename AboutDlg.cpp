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
#include "resource.h"
#include "AboutDlg.h"

#include <EAF\EAFUtilities.h>
#include <MFCTools\VersionInfo.h>



IMPLEMENT_DYNAMIC(CAboutDlg, CDialog)

CAboutDlg::CAboutDlg(UINT nResourceID,UINT nIDTemplate,CWnd* pParent /*=nullptr*/)
: CDialog(nIDTemplate == 0 ? IDD_ABOUTBOX : nIDTemplate, pParent)
{
   m_ResourceID = nResourceID;
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
   DDX_Control(pDX,IDC_WSDOT,m_WSDOT);
   DDX_Control(pDX, IDC_TXDOT, m_TxDOT);
   DDX_Control(pDX, IDC_BRIDGESIGHT, m_BridgeSight);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAboutDlg message handlers

BOOL CAboutDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   // put the icon in the dialog
   CStatic* pIcon = (CStatic*)GetDlgItem(IDC_APPICON);

   CWinApp* pApp = AfxGetApp();
   HICON hIcon = pApp->LoadIcon(m_ResourceID);
   if ( hIcon )
   {
      pIcon->SetIcon( hIcon );
   }
   else
   {
      CWnd* pWnd = EAFGetMainFrame();
      hIcon = pWnd->GetIcon(TRUE);
      if ( hIcon )
      {
         pIcon->SetIcon( hIcon );
      }
      else
      {
         pIcon->ShowWindow(SW_HIDE);
      }
   }

   // Get the version information and update the version # and copyright
   CString docString;
   docString.LoadString(m_ResourceID);
   CString strAppName;
   AfxExtractSubString(strAppName, docString, 1);

   CString strTitle;
   strTitle.Format(_T("About %s"),strAppName);
   SetWindowText( strTitle );

   CString strExe = AfxGetApp()->m_pszExeName;
   strExe += _T(".dll");

   CVersionInfo verInfo;
   verInfo.Load(strExe);
   CString strVersion = verInfo.GetFileVersionAsString();
   CString strCopyright = verInfo.GetLegalCopyright();

#if defined _WIN64
   CString strPlatform(_T("x64"));
#else
   CString strPlatform(_T("x86"));
#endif
   CString str;
   str.Format(_T("Version %s (%s)"),strVersion,strPlatform);
   GetDlgItem(IDC_VERSION)->SetWindowText(str);
   GetDlgItem(IDC_COPYRIGHT)->SetWindowText(strCopyright);


   m_WSDOT.SetURL(_T("http://www.wsdot.wa.gov"));
   m_WSDOT.SetTooltip(_T("http://www.wsdot.wa.gov"));
   m_WSDOT.SizeToContent();

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}
