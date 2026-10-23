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

#pragma once


#include <EAF\Transaction.h>

struct txnEditOptionsData
{
   WBFL::LRFD::BDSManager::Edition m_LRFDEdition;
   WBFL::LRFD::MBEManager::Edition m_LRFREdition;

   bool m_bDesignRating;
   bool m_bDesignRateForShear;

   bool m_bLegalRating;
   bool m_bLegalRateForShear;
   xbrTypes::EmergencyRatingMethod m_EmergencyRatingMethod;

   bool m_bPermitRating;
   bool m_bPermitRateForShear;
   bool m_bCheckYieldStress;
   Float64 m_YieldStressCoefficient;
   xbrTypes::PermitRatingMethod m_PermitRatingMethod;

   Float64 m_SystemFactorFlexure;
   Float64 m_SystemFactorShear;

   Float64 m_MaxLLStepSize;
   IndexType m_MaxLoadedLanes;
   xbrTypes::ReactionLoadApplicationType m_LiveLoadReactionApplication;

   Float64 m_PhiC;
   Float64 m_PhiT;
   Float64 m_PhiV;

   bool m_bDoAnalyzeNegativeMomentBetweenFOC;
   Float64 m_MinColumnWidthForNegMoment; // min column width before we have to analyze between FOC's
};

class txnEditOptions :
   public WBFL::EAF::Transaction
{
public:
   txnEditOptions(const txnEditOptionsData& oldOptionsData,const txnEditOptionsData& newOptionsData);
   ~txnEditOptions(void);

   virtual bool Execute();
   virtual void Undo();
   virtual std::unique_ptr<WBFL::EAF::Transaction>CreateClone() const;
   virtual std::_tstring Name() const;
   virtual bool IsUndoable() const;
   virtual bool IsRepeatable() const;

private:
   void Execute(int i);

   txnEditOptionsData m_Options[2];

};
