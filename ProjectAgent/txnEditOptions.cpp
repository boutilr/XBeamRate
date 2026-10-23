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
#include "ProjectAgent.h"
#include "txnEditOptions.h"

#include <IFace\Project.h>
#include <IFace\RatingSpecification.h>



txnEditOptions::txnEditOptions(const txnEditOptionsData& oldOptions,const txnEditOptionsData& newOptions)
{
   m_Options[0] = oldOptions;
   m_Options[1] = newOptions;
}

txnEditOptions::~txnEditOptions(void)
{
}

bool txnEditOptions::Execute()
{
   Execute(1);
   return true;
}

void txnEditOptions::Undo()
{
   Execute(0);
}

void txnEditOptions::Execute(int i)
{
   auto pBroker = EAFGetBroker();


   GET_IFACE2(pBroker,IXBREvents, pEvents);
   pEvents->HoldEvents(); // don't fire any changed events until all changes are done

   GET_IFACE2(pBroker,IXBRRatingSpecification,pRatingSpec);

   WBFL::LRFD::BDSManager::SetEdition(m_Options[i].m_LRFDEdition);
   WBFL::LRFD::MBEManager::SetEdition(m_Options[i].m_LRFREdition);

   pRatingSpec->EnableRating(pgsTypes::lrDesign_Inventory,m_Options[i].m_bDesignRating);
   pRatingSpec->EnableRating(pgsTypes::lrDesign_Operating,m_Options[i].m_bDesignRating);
   pRatingSpec->RateForShear(pgsTypes::lrDesign_Inventory,m_Options[i].m_bDesignRateForShear);
   pRatingSpec->RateForShear(pgsTypes::lrDesign_Operating,m_Options[i].m_bDesignRateForShear);

   pRatingSpec->EnableRating(pgsTypes::lrLegal_Routine,m_Options[i].m_bLegalRating);
   pRatingSpec->EnableRating(pgsTypes::lrLegal_Special, m_Options[i].m_bLegalRating);
   pRatingSpec->EnableRating(pgsTypes::lrLegal_Emergency, m_Options[i].m_bLegalRating);
   pRatingSpec->RateForShear(pgsTypes::lrLegal_Routine,m_Options[i].m_bLegalRateForShear);
   pRatingSpec->RateForShear(pgsTypes::lrLegal_Special, m_Options[i].m_bLegalRateForShear);
   pRatingSpec->RateForShear(pgsTypes::lrLegal_Emergency, m_Options[i].m_bLegalRateForShear);
   pRatingSpec->SetEmergencyRatingMethod(m_Options[i].m_EmergencyRatingMethod);

   pRatingSpec->EnableRating(pgsTypes::lrPermit_Routine,m_Options[i].m_bPermitRating);
   pRatingSpec->EnableRating(pgsTypes::lrPermit_Special,m_Options[i].m_bPermitRating);
   pRatingSpec->RateForShear(pgsTypes::lrPermit_Routine,m_Options[i].m_bPermitRateForShear);
   pRatingSpec->RateForShear(pgsTypes::lrPermit_Special,m_Options[i].m_bPermitRateForShear);
   pRatingSpec->CheckYieldStressLimit(m_Options[i].m_bCheckYieldStress);
   pRatingSpec->SetYieldStressLimitCoefficient(m_Options[i].m_YieldStressCoefficient);
   pRatingSpec->SetPermitRatingMethod(m_Options[i].m_PermitRatingMethod);

   GET_IFACE2(pBroker,IXBRProject,pProject);
   pProject->SetMaxLiveLoadStepSize(m_Options[i].m_MaxLLStepSize);
   pProject->SetMaxLoadedLanes(m_Options[i].m_MaxLoadedLanes);

   pProject->SetReactionLoadApplicationType(INVALID_ID, m_Options[i].m_LiveLoadReactionApplication);

   pProject->SetSystemFactorFlexure(m_Options[i].m_SystemFactorFlexure);
   pProject->SetSystemFactorShear(m_Options[i].m_SystemFactorShear);
   pProject->SetFlexureResistanceFactors(m_Options[i].m_PhiC,m_Options[i].m_PhiT);
   pProject->SetShearResistanceFactor(m_Options[i].m_PhiV);

   pProject->SetDoAnalyzeNegativeMomentBetweenFocOptions(m_Options[i].m_bDoAnalyzeNegativeMomentBetweenFOC, m_Options[i].m_MinColumnWidthForNegMoment);

   pEvents->FirePendingEvents();
}

std::unique_ptr<WBFL::EAF::Transaction> txnEditOptions::CreateClone() const
{
   return std::make_unique<txnEditOptions>(m_Options[0],m_Options[1]);
}

std::_tstring txnEditOptions::Name() const
{
   return _T("Edit Options");
}

bool txnEditOptions::IsUndoable() const
{
   return true;
}

bool txnEditOptions::IsRepeatable() const
{
   return false;
}
