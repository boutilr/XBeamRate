///////////////////////////////////////////////////////////////////////
// XBeamRate - Cross Beam Load Rating
// Copyright © 1999-2025  Washington State Department of Transportation
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

#include "StdAfx.h"
#include "ReportAgent.h"
#include "DevLengthDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\Project.h>
#include <IFace\Bridge.h>
#include <IFace\Pier.h>
#include <Details.h>
#include <IFace\AnalysisResults.h>

#include "XBeamRateReportSpecification.h"

#include <WBFLGenericBridge.h>

#include <XBeamRateExt\LongitudinalRebarData.h>

#include <XBeamRateExt\StirrupData.h>

#include <PsgLib\GirderLabel.h>

CDevLengthDetailsChapterBuilder::CDevLengthDetailsChapterBuilder()
{
}

LPCTSTR CDevLengthDetailsChapterBuilder::GetName() const
{
   return TEXT("Development Length Details");
}

rptChapter* CDevLengthDetailsChapterBuilder::Build(const std::shared_ptr<const WBFL::Reporting::ReportSpecification>& pRptSpec,Uint16 level) const
{
   USES_CONVERSION;

   auto pXBRRptSpec = std::dynamic_pointer_cast<const CXBeamRateReportSpecification>(pRptSpec);

   auto pBroker = pXBRRptSpec->GetBroker();

   rptChapter* pChapter = CXBeamRateChapterBuilder::Build(pRptSpec, level);

   rptParagraph* pParagraph = new rptParagraph(rptStyleManager::GetHeadingStyle());
   *pChapter << pParagraph;

   (*pParagraph) << _T("Development length of longitudinal reinforcement") << rptNewLine;

   pParagraph = new rptParagraph;
   *pChapter << pParagraph;

   (*pParagraph) << _T("AASHTO LRFD BDS ") << WBFL::LRFD::LrfdCw8th(_T("5.11.2.1"), _T("5.10.8.2.1"));

   (*pParagraph) << rptNewLine;

   GET_IFACE2(pBroker, IXBRRebar, pRebar);

   GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

   PierIndexType pierID = pXBRRptSpec->GetPierID();

   IndexType nRebarRows = pRebar->GetRebarRowCount(pierID);

   if (nRebarRows == 0)
   {
       (*pParagraph) << _T("No longitudinal reinforcement defined") << rptNewLine;
   }
   else
   {

           if (WBFL::LRFD::BDSManager::Edition::TenthEdition2024 <= WBFL::LRFD::BDSManager::GetEdition())
           {
               (*pParagraph) << rptNewLine << rptRcEquation(std::_tstring(rptStyleManager::GetImagePath()) + _T("LongitudinalRebarDevelopment_2024.png"), _T("l_d = l_{db} \\times \\lambda_{rl} \\times \\lambda_{cf} \\times \\lambda_{rc} \\\\ \\textit{in which:} \\hspace{5 mm}  l_{db} = 0.17 d_b \\left[ \\dfrac{{\\lambda_{er} f_y - \\frac{F_h} {A_b}}} {1.97 \\lambda {f'_c}^{0.25} }\\right]^2 \\\\ \\textit{where:} \\hspace{5 mm} \\lambda_{cf}=\\lambda_{rc}=\\lambda_{er} = 1.0, \\text{ and } F_h = 0.0")) << rptNewLine;
           }
           else if (WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2016Interims <= WBFL::LRFD::BDSManager::GetEdition())
           {
               (*pParagraph) << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("LongitudinalRebarDevelopment_2016.png")) << rptNewLine;
           }
           else if (WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2015Interims == WBFL::LRFD::BDSManager::GetEdition())
           {
               (*pParagraph) << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("LongitudinalRebarDevelopment_2015.png")) << rptNewLine;
           }
           else
           {
               if (IS_US_UNITS(pDisplayUnits))
               {
                   (*pParagraph) << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("LongitudinalRebarDevelopment_US.png")) << rptNewLine;
               }
               else
               {
                   (*pParagraph) << rptRcImage(std::_tstring(rptStyleManager::GetImagePath()) + _T("LongitudinalRebarDevelopment_SI.png")) << rptNewLine;
               }
           }
   }

   (*pParagraph) << rptNewLine;

   bool is_2015 = WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2015Interims == WBFL::LRFD::BDSManager::GetEdition();
   bool is_2016 = WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2016Interims <= WBFL::LRFD::BDSManager::GetEdition();

   ColumnIndexType nColumns = 8;

   // Versions before this did not consider the factor in pgsuper
   bool is_LocationFactor = WBFL::LRFD::BDSManager::Edition::EighthEdition2017 <= WBFL::LRFD::BDSManager::GetEdition();

   if (is_LocationFactor)
   {
       nColumns += 3; // dist to bottom, lamda rl, lamda lw
   }
   else if (is_2015 || is_2016)
   {
       nColumns += 2; // lamda rl and lamda lw
   }

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns, _T(""));

   ColumnIndexType col = 0;
   (*pTable)(0, col++) << _T("Bar Size");
   (*pTable)(0, col++) << COLHDR(Sub2(_T("A"), _T("b")), rptAreaUnitTag, pDisplayUnits->GetAreaUnit());
   (*pTable)(0, col++) << COLHDR(Sub2(_T("d"), _T("b")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
   (*pTable)(0, col++) << COLHDR(RPT_FY, rptStressUnitTag, pDisplayUnits->GetStressUnit());
   (*pTable)(0, col++) << COLHDR(RPT_FC, rptStressUnitTag, pDisplayUnits->GetStressUnit());
   if (is_LocationFactor)
   {
       (*pTable)(0, col++) << COLHDR(_T("Distance from") << rptNewLine << _T("Bottom"), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
       (*pTable)(0, col++) << symbol(lambda) << Sub(_T("rl"));
       (*pTable)(0, col++) << symbol(lambda);
   }
   else if (is_2016)
   {
       (*pTable)(0, col++) << symbol(lambda) << Sub(_T("rl"));
       (*pTable)(0, col++) << symbol(lambda);
   }
   else if (is_2015)
   {
       (*pTable)(0, col++) << symbol(lambda) << Sub(_T("rl"));
       (*pTable)(0, col++) << symbol(lambda) << Sub(_T("lw"));
   }
   (*pTable)(0, col++) << _T("Modification") << rptNewLine << _T("Factor");
   (*pTable)(0, col++) << COLHDR(Sub2(_T("l"), _T("db")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());
   (*pTable)(0, col++) << COLHDR(Sub2(_T("l"), _T("d")), rptLengthUnitTag, pDisplayUnits->GetComponentDimUnit());

    (*pParagraph) << pTable << rptNewLine;

    GET_IFACE2(pBroker, IXBRProject, pProject);

    INIT_UV_PROTOTYPE(rptStressUnitValue, stress, pDisplayUnits->GetStressUnit(), false);
    INIT_UV_PROTOTYPE(rptLengthUnitValue, length, pDisplayUnits->GetSpanLengthUnit(), false);
    INIT_UV_PROTOTYPE(rptLength2UnitValue, area, pDisplayUnits->GetAreaUnit(), false);

    rptRcScalar scalar;
    scalar.SetFormat(WBFL::System::NumericFormatTool::Format::Fixed);
    scalar.SetWidth(6);
    scalar.SetPrecision(3);
    scalar.SetTolerance(1.0e-6);

	RowIndexType row = pTable->GetNumberOfHeaderRows();

    CComPtr<IRebarSection> rebarSection;
    const auto& stage = xbrTypes::Stage::Stage1;
    pRebar->GetRebarSection(pierID, stage, xbrPointOfInterest(0, 0), &rebarSection);

    CComPtr<IEnumRebarSectionItem> enumRebar;
    rebarSection->get__EnumRebarSectionItem(&enumRebar);

    CComPtr<IRebarSectionItem> rebarSectionItem;
    while (enumRebar->Next(1, &rebarSectionItem, nullptr) != S_FALSE)
    {
        CComPtr<IPoint2d> pntRebar;
        rebarSectionItem->get_Location(&pntRebar);

        Float64 Ybar = pRebar->GetRebarDepth(pierID, xbrPointOfInterest(0, 0), stage, pntRebar); // depth from top of cross beam to rebar

        if (Ybar < 0)
        {
            // rebar is not in the cross section (not applicable in this stage)
            rebarSectionItem.Release();
            continue;
        }

        CComPtr<IRebar> rebar;
        rebarSectionItem->get_Rebar(&rebar);

        USES_CONVERSION;
        CComBSTR name;

        rebar->get_Name(&name);

        WBFL::Materials::Rebar::Size size = WBFL::LRFD::RebarPool::GetBarSize(OLE2CT(name));

        Float64 Ab, db, fy;
        rebar->get_NominalArea(&Ab);
        rebar->get_NominalDiameter(&db);
        rebar->get_YieldStrength(&fy);

        const CConcreteMaterial& concrete = pProject->GetConcrete(pierID);
        Float64 fc = concrete.Fc;

        WBFL::Materials::ConcreteType type = WBFL::Materials::ConcreteType::Normal;
        bool hasFct = false;
        Float64 Fct = 0.0;

        WBFL::LRFD::REBARDEVLENGTHDETAILS details = WBFL::LRFD::Rebar::GetRebarDevelopmentLengthDetails(size, Ab, db, fy, type, fc, hasFct, Fct, concrete.StrengthDensity, false, false, true);

        ColumnIndexType col = 0;
        (*pTable)(row, col++) << name;
        (*pTable)(row, col++) << area.SetValue(Ab);
        (*pTable)(row, col++) << length.SetValue(db);
        (*pTable)(row, col++) << stress.SetValue(fy);
        (*pTable)(row, col++) << stress.SetValue(fc);
        if (is_LocationFactor)
        {
            (*pTable)(row, col++) << length.SetValue(details.distFromBottom); //face?
            (*pTable)(row, col++) << scalar.SetValue(details.lambdaRl);
            (*pTable)(row, col++) << scalar.SetValue(details.lambdaLw);
        }
        else if (is_2015 || is_2016)
        {
            (*pTable)(row, col++) << scalar.SetValue(details.lambdaRl);
            (*pTable)(row, col++) << scalar.SetValue(details.lambdaLw);
        }
        (*pTable)(row, col++) << scalar.SetValue(details.factor);
        (*pTable)(row, col++) << length.SetValue(details.ldb);
        (*pTable)(row, col++) << length.SetValue(details.ld);

        row++;
    

        rebarSectionItem.Release();

    }
            
    return pChapter;
}

