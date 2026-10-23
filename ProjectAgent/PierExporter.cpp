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
#include "PierExporter.h"
#include "ProjectAgentImp.h"
#include <MFCTools\Prompts.h>
#include <numeric>

#include <PsgLib\GirderLabel.h>

#include <IFace\Bridge.h>
#include <EAF\EAFApp.h>
#include <PsgLib\BridgeDescription2.h>





class CPierChoiceValidator : public CMultiChoiceValidator
{
public:
   virtual BOOL IsValid(const std::vector<int>& options);
   virtual void DisplayValidationErrorMessage();

private:
   CString m_strMessage;
};

BOOL CPierChoiceValidator::IsValid(const std::vector<int>& options)
{
   if ( options.size() == 0 )
   {
      m_strMessage = _T("Please select at least one pier to export");
      return FALSE;
   }
   return TRUE;
}

void CPierChoiceValidator::DisplayValidationErrorMessage()
{
   AfxMessageBox(m_strMessage,MB_OK | MB_ICONEXCLAMATION);
}


////////////////////////////////////////////////

CPierExporter::CPierExporter(std::weak_ptr<WBFL::EAF::Broker> pBroker,CProjectAgentImp* pProjectAgent)
{
   m_pBroker = pBroker;
   m_pProjectAgent = pProjectAgent;
}

HRESULT CPierExporter::Export(PierIndexType pierIdx)
{
   if ( pierIdx == INVALID_INDEX )
   {
      // Prompt for pier
      auto broker = m_pBroker.lock();
      GET_IFACE2(broker,IBridge,pBridge);
      PierIndexType nPiers = pBridge->GetPierCount();
      CString strPiers;
      for ( PierIndexType pIdx = 0; pIdx < nPiers; pIdx++ )
      {
         pgsTypes::PierModelType modelType = pBridge->GetPierModelType(pIdx);
         if ( modelType == pgsTypes::pmtPhysical )
         {
            CString str;
            str.Format(_T("Pier %s"),LABEL_PIER(pIdx));
            if ( strPiers.GetLength() == 0 )
            {
               strPiers = str;
            }
            else
            {
               strPiers += _T("\n") + str;
            }
         }
      }

      if ( strPiers.GetLength() == 0 )
      {
         AfxMessageBox(_T("There aren't any physical pier models for this bridge. There is nothing to export."));
         return S_OK;
      }
      else
      {
         int result = AfxChoose(_T("Select Pier"),_T("Select pier model to export to XBRate"),strPiers,0,TRUE);
         if ( result < 0 )
         {
            // Cancel button was pressed
            return S_OK;
         }

         pierIdx = (PierIndexType)(result+1);
      }
   }

   CString strDefaultFileName = GetDefaultPierExportFile(pierIdx);

   CString strFileName;
   if ( GetFileName(strDefaultFileName,strFileName) == IDOK )
   {
      HRESULT hr = ExportPierModel(pierIdx,strFileName);
      if ( SUCCEEDED(hr) )
      {
         if ( AfxMessageBox(_T("Export Complete.\r\nWould you like to open the XBRate file?"),MB_ICONQUESTION | MB_YESNO) == IDYES )
         {
            CEAFApp* pApp = EAFGetApp();

            // Use ReplaceDocumentFile instead of OpenDocumentFile
            // OpenDocumentFile opens the document right now... we are in the middle of an agent method call
            // closing the current file attempts to shutdown the broker and destroy all the agents... 
            // this agent isn't ready to die yet.
            // ReplaceDocumentFile delays opening the document file until this agent method call is done
            // and the call stack gets back to the message loop.
            pApp->ReplaceDocumentFile(strFileName);
         }
      }
      else
      {
         AfxMessageBox(_T("An error occurred while exporting the pier data."),MB_ICONEXCLAMATION | MB_OK);
         return E_FAIL;
      }
   }

   return S_OK;
}

HRESULT CPierExporter::BatchExport()
{
   std::vector<PierIndexType> vPiers;
   CString strPiers;
   auto broker = m_pBroker.lock();
   GET_IFACE2(broker,IBridgeDescription,pIBridgeDesc);
   PierIndexType nPiers = pIBridgeDesc->GetPierCount();
   for ( PierIndexType pierIdx = 0; pierIdx < nPiers; pierIdx++ )
   {
      const CPierData2* pPier = pIBridgeDesc->GetPier(pierIdx);
      if ( pPier->GetPierModelType() == pgsTypes::pmtPhysical )
      {
         CString strPier;
         strPier.Format(_T("Pier %s\n"),LABEL_PIER(pierIdx));
         strPiers += strPier;
         vPiers.push_back(pierIdx);
      }
   }

   if ( strPiers.IsEmpty() )
   {
      AfxMessageBox(_T("The model doesn't have any exportable piers."),MB_ICONINFORMATION | MB_OK);
      return S_OK;
   }

   // by default, selected everything
   // make a default choice list of 0,1,2,...
   std::vector<int> vDefChoices;
   vDefChoices.resize(vPiers.size());
   std::iota(vDefChoices.begin(),vDefChoices.end(),0);

   CPierChoiceValidator validator;
   std::vector<int> vChoices = AfxMultiChoice(_T("Export Piers to XBRate"),_T("Select Piers to Export"),strPiers,&validator,vDefChoices,TRUE,TRUE);

   for (const auto& i : vChoices)
   {
      PierIndexType pierIdx = vPiers[i];
      CString strDefaultFileName = GetDefaultPierExportFile(pierIdx);
      CString strFileName;
      if ( GetFileName(strDefaultFileName,strFileName) == IDOK )
      {
         HRESULT hr = ExportPierModel(pierIdx,strFileName);
         if ( FAILED(hr) )
         {
            CString strMsg;
            strMsg.Format(_T("An error occurred while exporting Pier %s."),LABEL_PIER(pierIdx));
            AfxMessageBox(strMsg,MB_ICONEXCLAMATION | MB_OK);
         }
      }
   }

   if ( 0 < vChoices.size() )
   {
      AfxMessageBox(__T("Export Complete"),MB_ICONINFORMATION | MB_OK);
   }

   return S_OK;
}

INT_PTR CPierExporter::GetFileName(const CString& strDefaultFileName,CString& strFileName)
{
   CFileDialog  fileDlg(FALSE,_T("xbr"),strDefaultFileName,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("XBRate File (*.xbr)|*.xbr||"));
   INT_PTR result = fileDlg.DoModal();
   if ( result == IDOK )
   {
      strFileName = fileDlg.GetPathName();
   }
   return result;
}

CString CPierExporter::GetDefaultPierExportFile(PierIndexType pierIdx)
{
   CString strDefaultFileName;
   auto broker = m_pBroker.lock();
   GET_IFACE2(broker,IEAFDocument,pDoc);

   strDefaultFileName.Format(_T("%s%s_Pier_%s.xbr"),
                             pDoc->GetFileRoot(), // path to the file
                             pDoc->GetFileTitle(), // the file name without path or extension
                             LABEL_PIER(pierIdx));

   return strDefaultFileName;
}

HRESULT CPierExporter::ExportPierModel(PierIndexType pierIdx,LPCTSTR lpszPathName)
{
   return m_pProjectAgent->SavePier(pierIdx,lpszPathName);
}
