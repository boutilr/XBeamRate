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

/****************************************************************************
CLASS
   CXBRateCommandLineProcessor
****************************************************************************/
#include "stdafx.h"
#include <AgentTools.h>
#include "XBeamRateCommandLineProcessor.h"
#include "XBeamRateCommandLineInfo.h"
#include "XBeamRateDoc.h"

#include <EAF\EAFApp.h>
#include <EAF\EAFDocument.h>

#include <XBeamRateExt\XBeamRateUtilities.h>

#include <IFace\Test.h>
#include <..\..\PGSuper\Include\IFace\Project.h>

#include <PsgLib\BridgeDescription2.h>
#include <PsgLib\GirderLabel.h>



CXBRateCommandLineProcessor::CXBRateCommandLineProcessor()
{
}

CXBRateCommandLineProcessor::~CXBRateCommandLineProcessor()
{
}

BOOL CXBRateCommandLineProcessor::ProcessCommandLineOptions(CEAFCommandLineInfo& cmdInfo)
{
   // cmdInfo is the command line information from the application. The application
   // doesn't know about this plug-in at the time the command line parameters are parsed
   //
   // Re-parse the parameters with our own command line information object
   CXBRateCommandLineInfo xbrCmdInfo;
   EAFGetApp()->ParseCommandLine(xbrCmdInfo);
   cmdInfo = xbrCmdInfo;

   if (xbrCmdInfo.m_bError)
   {
      return FALSE;
   }
   else if (xbrCmdInfo.m_bRegTests)
   {
      auto pBroker = EAFGetBroker();

      GET_IFACE2(pBroker,IXBRTest,pTest);

      if ( IsStandAlone() )
      {
         CString strResultsFile;
         if ( !GetResultsFileName(xbrCmdInfo.m_strFileName,INVALID_INDEX,strResultsFile) )
         {
            CString msg = CString(_T("Error - Test input files must have .xbr extension"));
            ::AfxMessageBox(msg);
            return FALSE;
         }

         HRESULT hr = pTest->RunTest(INVALID_ID,strResultsFile);
         if ( FAILED(hr) )
         {
            CString msg = CString(_T("Error - Running test on file "))+xbrCmdInfo.m_strFileName;
            ::AfxMessageBox(msg);
         }
      }
      else
      {
         GET_IFACE2(pBroker,IBridgeDescription,pBridgeDesc);
         PierIndexType nPiers = pBridgeDesc->GetPierCount();
         for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
         {
            const CPierData2* pPier = pBridgeDesc->GetPier(pierIdx);
            if ( pPier->GetPierModelType() == pgsTypes::pmtPhysical )
            {
               CString strResultsFile;
               if ( !GetResultsFileName(xbrCmdInfo.m_strFileName,pierIdx,strResultsFile) )
               {
                  CString msg = CString(_T("Error - Test input files must have .pgs or .spl extension"));
                  ::AfxMessageBox(msg);
                  return FALSE;
               }

               HRESULT hr = pTest->RunTest(pPier->GetID(),strResultsFile);
               if ( FAILED(hr) )
               {
                  CString msg = CString(_T("Error - Running test on file "))+xbrCmdInfo.m_strFileName;
                  ::AfxMessageBox(msg);
               }
            }
         }
      }
      return TRUE; // command line parameters handled
   }

   BOOL bHandled = FALSE;
   CEAFMainFrame* pFrame = EAFGetMainFrame();
   CEAFDocument* pDoc = pFrame->GetDocument();
   if ( pDoc && pDoc->IsKindOf(RUNTIME_CLASS(CXBeamRateDoc)) )
   {
      bHandled = pDoc->ProcessCommandLineOptions(cmdInfo);
   }

   // If we get this far and there is one parameter and it isn't a file name and it isn't handled -OR-
   // if there is more than one parameter and it isn't handled there is something wrong
   if ( ((1 == xbrCmdInfo.m_Count && xbrCmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen) || (1 < xbrCmdInfo.m_Count)) && !bHandled )
   {
      cmdInfo.m_bError = TRUE;
      bHandled = TRUE;
   }

   return bHandled;
}

BOOL CXBRateCommandLineProcessor::GetResultsFileName(LPCTSTR lpszSourceFile,PierIndexType pierIdx,CString& strResultsFile)
{
   CString strSourceFile(lpszSourceFile);
   strSourceFile.MakeLower();
   int loc = 0;
   if ( pierIdx == INVALID_INDEX )
   {
      loc = strSourceFile.Find(_T(".xbr"),0);
   }
   else
   {
      int loc1 = strSourceFile.Find(_T(".pgs"),0);
      int loc2 = strSourceFile.Find(_T(".spl"),0);
      loc = Max(loc1,loc2);
   }

   if ( 0 < loc )
   {
      if ( pierIdx == INVALID_INDEX )
      {
         strResultsFile.Format(_T("%s.dbr"),CString(lpszSourceFile).Left(strSourceFile.GetLength()-4));
      }
      else
      {
         strResultsFile.Format(_T("%s_Pier_%s.dbr"),CString(lpszSourceFile).Left(strSourceFile.GetLength()-4),LABEL_PIER(pierIdx));
      }
      return TRUE;
   }
   return FALSE;
}
