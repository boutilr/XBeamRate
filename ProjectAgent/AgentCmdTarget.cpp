#include "stdafx.h"
#include "resource.h"
#include "AgentCmdTarget.h"

#include <MFCTools\Prompts.h>
#include <EAF\EAFTransactions.h>
#include <txnEditProject.h>
#include <txnEditPier.h>

#include "PierDlg.h"

#include <IFace\Project.h>


BEGIN_MESSAGE_MAP(CAgentCmdTarget,CCmdTarget)
   ON_COMMAND(ID_EDIT_PROJECT_NAME,OnEditProjectName)
   ON_COMMAND(ID_EDIT_PIER, &CAgentCmdTarget::OnEditPier)
END_MESSAGE_MAP()

CAgentCmdTarget::CAgentCmdTarget()
{
}

void CAgentCmdTarget::Init(IBroker* pBroker)
{
   m_pBroker = pBroker;
}

void CAgentCmdTarget::OnEditProjectName()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   CString strOldProjectName, strNewProjectName;
   GET_IFACE(IXBRProject,pProject);
   strOldProjectName = pProject->GetProjectName();

   if ( AfxQuestion(_T("Project Name"),_T("Enter project name"),strOldProjectName,strNewProjectName) )
   {
      txnEditProject txn(strOldProjectName,strNewProjectName);
      GET_IFACE(IEAFTransactions,pTransactions);
      pTransactions->Execute(txn);
   }
}

void CAgentCmdTarget::OnEditPier()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   GET_IFACE(IXBRProject,pProject);
   CPierDlg dlg(_T("Edit Pier"));

   txnEditPierData oldPierData;
   oldPierData.m_PierType = pProject->GetPierType();
   oldPierData.m_DeckElevation = pProject->GetDeckElevation();
   oldPierData.m_CrownPointOffset = pProject->GetCrownPointOffset();
   oldPierData.m_BridgeLineOffset = pProject->GetBridgeLineOffset();
   oldPierData.m_strOrientation = pProject->GetOrientation();

   oldPierData.m_CurbLineDatum = pProject->GetCurbLineDatum();
   pProject->GetCurbLineOffset(&oldPierData.m_LeftCLO,&oldPierData.m_RightCLO);
   pProject->GetCrownSlopes(&oldPierData.m_SL,&oldPierData.m_SR);

   pProject->GetDiaphragmDimensions(&oldPierData.m_DiaphragmHeight,&oldPierData.m_DiaphragmWidth);

   oldPierData.m_nBearingLines = pProject->GetBearingLineCount();
   for ( IndexType brgLineIdx = 0; brgLineIdx < oldPierData.m_nBearingLines; brgLineIdx++ )
   {
      oldPierData.m_BearingLines[brgLineIdx].clear();
      IndexType nBearings = pProject->GetBearingCount(brgLineIdx);
      for ( IndexType brgIdx = 0; brgIdx < nBearings; brgIdx++ )
      {
         txnBearingData brgData;
         if ( brgIdx < nBearings-1 )
         {
            brgData.m_S = pProject->GetBearingSpacing(brgLineIdx,brgIdx);
         }
         pProject->GetBearingReactions(brgLineIdx,brgIdx,&brgData.m_DC,&brgData.m_DW);
         oldPierData.m_BearingLines[brgLineIdx].push_back(brgData);

         IndexType refIdx;
         Float64 refBearingOffset;
         pgsTypes::OffsetMeasurementType refBearingDatum;
         pProject->GetReferenceBearing(brgLineIdx,&refIdx,&refBearingOffset,&refBearingDatum);
         oldPierData.m_RefBearingIdx[brgLineIdx]      = refIdx;
         oldPierData.m_RefBearingLocation[brgLineIdx] = refBearingOffset;
         oldPierData.m_RefBearingDatum[brgLineIdx]    = refBearingDatum;
      }
   }

   oldPierData.m_Ec = pProject->GetModE();

   IndexType nRebarRows = pProject->GetRebarRowCount();
   for ( IndexType rowIdx = 0; rowIdx < nRebarRows; rowIdx++ )
   {
      txnLongutindalRebarData rebarData;
      pProject->GetRebarRow(rowIdx,&rebarData.datum,&rebarData.cover,&rebarData.barSize,&rebarData.nBars,&rebarData.spacing);
      oldPierData.m_Rebar.push_back(rebarData);
   }

   for ( int i = 0; i < 2; i++ )
   {
      pgsTypes::PierSideType side = (pgsTypes::PierSideType)i;
      pProject->GetXBeamDimensions(side,&oldPierData.m_XBeamHeight[side],&oldPierData.m_XBeamTaperHeight[side],&oldPierData.m_XBeamTaperLength[side]);
      oldPierData.m_XBeamOverhang[side] = pProject->GetXBeamOverhang(side);
   }
   oldPierData.m_XBeamWidth = pProject->GetXBeamWidth();

   oldPierData.m_nColumns = pProject->GetColumnCount();
   oldPierData.m_ColumnHeight = pProject->GetColumnHeight(0);
   oldPierData.m_ColumnHeightMeasurementType = pProject->GetColumnHeightMeasurementType();
   oldPierData.m_ColumnSpacing = (oldPierData.m_nColumns == 1 ? 0 : pProject->GetSpacing(0));
   pProject->GetColumnShape(&oldPierData.m_ColumnShape,&oldPierData.m_B,&oldPierData.m_D);

   pProject->GetTransverseLocation(&oldPierData.m_RefColumnIdx,&oldPierData.m_TransverseOffset,&oldPierData.m_TransverseOffsetMeasurement);

   pProject->GetConditionFactor(&oldPierData.m_ConditionFactorType,&oldPierData.m_ConditionFactor);
   oldPierData.m_gDC = pProject->GetDCLoadFactor();
   oldPierData.m_gDW = pProject->GetDWLoadFactor();
   for ( int i = 0; i < 6; i++ )
   {
      pgsTypes::LoadRatingType ratingType = (pgsTypes::LoadRatingType)i;
      oldPierData.m_gLL[ratingType] = pProject->GetLiveLoadFactor(ratingType);
   }

   oldPierData.m_DesignLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pgsTypes::lltDesign);
   oldPierData.m_LegalRoutineLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pgsTypes::lltLegalRating_Routine);
   oldPierData.m_LegalSpecialLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pgsTypes::lltLegalRating_Special);
   oldPierData.m_PermitRoutineLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pgsTypes::lltPermitRating_Routine);
   oldPierData.m_PermitSpecialLiveLoad.m_LLIM = pProject->GetLiveLoadReactions(pgsTypes::lltPermitRating_Special);

   dlg.SetPierData(oldPierData);
   if ( dlg.DoModal() == IDOK )
   {
      txnEditPierData newPierData = dlg.GetPierData();
      txnEditPier txn(oldPierData,newPierData);
      GET_IFACE(IEAFTransactions,pTransactions);
      pTransactions->Execute(txn);
   }
}
