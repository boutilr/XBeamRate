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

#include <XBeamRateExt\LongitudinalRebarData.h>
#include <XBeamRateExt\StirrupData.h>

struct xbrEditReinforcementData
{
   WBFL::Materials::Rebar::Type Type;
   WBFL::Materials::Rebar::Grade Grade;
   xbrLongitudinalRebarData LongitudinalRebar;
   xbrStirrupData LowerXBeamStirrups;
   xbrStirrupData FullDepthStirrups;
   pgsTypes::ConditionFactorType ConditionFactorType;
   Float64 ConditionFactor;
};

class txnEditReinforcement :
   public WBFL::EAF::Transaction
{
public:
   txnEditReinforcement(PierIDType pierID,const xbrEditReinforcementData& oldReinforcement,const xbrEditReinforcementData& newReinforcement);
   ~txnEditReinforcement(void);

   virtual bool Execute();
   virtual void Undo();
   virtual std::unique_ptr<WBFL::EAF::Transaction> CreateClone() const;
   virtual std::_tstring Name() const;
   virtual bool IsUndoable() const;
   virtual bool IsRepeatable() const;

private:
   void Execute(int i);

   PierIDType m_PierID;
	xbrEditReinforcementData m_Reinforcement[2];
};
