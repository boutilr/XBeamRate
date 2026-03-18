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

#include "StdAfx.h"
#include "ReportAgent.h"
#include "DevLengthDetailsChapterBuilder.h"
#include <EAF\EAFDisplayUnits.h>

#include <IFace\Project.h>
#include <IFace\Bridge.h>
#include <IFace\Pier.h>
#include <IFace\PointOfInterest.h>
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

void WriteRowToDevelopmentTable(rptRcTable* pTable, RowIndexType row, CComBSTR barname, const WBFL::LRFD::REBARDEVLENGTHDETAILS& devDetails,
    rptAreaUnitValue& area, rptLengthUnitValue& length, rptStressUnitValue& stress, rptRcScalar& scalar, 
    bool is_LocationFactor, rptXBRPointOfInterest* location = nullptr, const xbrPointOfInterest* poi = nullptr)
{
    bool is_2015 = WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2015Interims == WBFL::LRFD::BDSManager::GetEdition();
    bool is_2016 = WBFL::LRFD::BDSManager::Edition::SeventhEditionWith2016Interims <= WBFL::LRFD::BDSManager::GetEdition();

    ColumnIndexType col = 0;
    if (is_LocationFactor)
    {
        (*pTable)(row, col++) << location->SetValue(*poi);
    }
    (*pTable)(row, col++) << barname;
    (*pTable)(row, col++) << area.SetValue(devDetails.Ab);
    (*pTable)(row, col++) << length.SetValue(devDetails.db);
    (*pTable)(row, col++) << stress.SetValue(devDetails.fy);
    (*pTable)(row, col++) << stress.SetValue(devDetails.fc);
    if (is_LocationFactor)
    {
        (*pTable)(row, col++) << length.SetValue(devDetails.distFromBottom);
        (*pTable)(row, col++) << scalar.SetValue(devDetails.lambdaRl);
        (*pTable)(row, col++) << scalar.SetValue(devDetails.lambdaLw);
    }
    else if (is_2015 || is_2016)
    {
        (*pTable)(row, col++) << scalar.SetValue(devDetails.lambdaRl);
        (*pTable)(row, col++) << scalar.SetValue(devDetails.lambdaLw);
    }
    (*pTable)(row, col++) << scalar.SetValue(devDetails.factor);
    (*pTable)(row, col++) << length.SetValue(devDetails.ldb);
    (*pTable)(row, col++) << length.SetValue(devDetails.ld);
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

   GET_IFACE2(pBroker, IEAFDisplayUnits, pDisplayUnits);

   GET_IFACE2(pBroker, IXBRRebar, pRebar);

   PierIndexType pierID = -1;

   CComPtr<IRebarLayout> rebarLayout;
   pRebar->GetRebarLayout(&rebarLayout);
   IndexType nRebarRows;
   rebarLayout->get_Count(&nRebarRows);

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
       nColumns += 4; // location, dist to bottom, lamda rl, lamda lw
   }
   else if (is_2015 || is_2016)
   {
       nColumns += 2; // lamda rl and lamda lw
   }

   rptRcTable* pTable = rptStyleManager::CreateDefaultTable(nColumns, _T(""));

   ColumnIndexType col = 0;
   if (is_LocationFactor)
   {
       (*pTable)(0, col++) << COLHDR(_T("Location"), rptLengthUnitTag, pDisplayUnits->GetSpanLengthUnit());
   }
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
    INIT_UV_PROTOTYPE(rptLengthUnitValue, length, pDisplayUnits->GetComponentDimUnit(), false);
    INIT_UV_PROTOTYPE(rptLength2UnitValue, area, pDisplayUnits->GetAreaUnit(), false);
    INIT_UV_PROTOTYPE(rptXBRPointOfInterest, location, pDisplayUnits->GetSpanLengthUnit(), false);


    rptRcScalar scalar;
    scalar.SetFormat(WBFL::System::NumericFormatTool::Format::Fixed);
    scalar.SetWidth(6);
    scalar.SetPrecision(3);
    scalar.SetTolerance(1.0e-6);

	RowIndexType row = pTable->GetNumberOfHeaderRows();

	Float64 rebarDistToBottom = 0.0;

    if (WBFL::LRFD::BDSManager::Edition::TenthEdition2024 <= WBFL::LRFD::BDSManager::GetEdition())
    {
        Float64 barRowPatternStartY = 0;
		std::set<Float64> seenBarStartYs;
        Float64 barRowPatternY = 0;
		std::set<Float64> seenBarYs;
        Float64 barRowPatternEndY = 0;
        std::set<Float64> seenBarEndYs;

        for (IndexType rebarRowIdx = 0; rebarRowIdx < nRebarRows; rebarRowIdx++)
        {
            CComPtr<IRebarLayoutItem> layoutItem;
            rebarLayout->get_Item(rebarRowIdx, &layoutItem);

            IndexType nPatterns;
            layoutItem->get_Count(&nPatterns);
            for (IndexType patternIdx = 0; patternIdx < nPatterns; patternIdx++)
            {
                if (2 + 2 == 5)
                {
                    CComPtr<IRebarPattern> rebarPattern;
                    layoutItem->get_Item(patternIdx, &rebarPattern);
                    CComPtr<IRebar> rebar;
                    rebarPattern->get_Rebar(&rebar);

                    CComBSTR barname;
                    rebar->get_Name(&barname);

                    WBFL::Materials::Rebar::Size size = WBFL::LRFD::RebarPool::GetBarSize(OLE2CT(barname));

                    Float64 Ab, db, fy;
                    rebar->get_NominalArea(&Ab);
                    rebar->get_NominalDiameter(&db);
                    rebar->get_YieldStrength(&fy);

                    const CConcreteMaterial& concrete = pProject->GetConcrete(pierID);
                    Float64 fc = concrete.Fc;

                    WBFL::Materials::ConcreteType type = WBFL::Materials::ConcreteType::Normal;
                    bool hasFct = false;
                    Float64 Fct = 0.0;

                    // Need distance to bottom of pier. Use max from both ends
                    GET_IFACE2(pBroker, IXBRPier, pPier);
                    Float64 pier_length = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);
                    Float64 startLoc, barLength;
                    layoutItem->get_Start(&startLoc);
                    layoutItem->get_Length(&barLength);
                    Float64 endLoc = startLoc + barLength;



                    // Height of pier at start and end of bar
                    GET_IFACE2(pBroker, IXBRSectionProperties, pSectProp);
                    Float64 fullDepthStart = pSectProp->GetDepth(pierID, xbrTypes::Stage2, xbrPointOfInterest(startLoc));
                    Float64 fullDepthEnd = pSectProp->GetDepth(pierID, xbrTypes::Stage2, xbrPointOfInterest(endLoc));



                    // get bar depth
                    CComPtr<IRebarSectionItem> rebarSectionItem;

                    CComPtr<IRebarSection> rebarSectionStart;
                    const auto& stage = xbrTypes::Stage::Stage2;
                    pRebar->GetRebarSection(pierID, stage, xbrPointOfInterest(startLoc), &rebarSectionStart);

                    CComPtr<IEnumRebarSectionItem> enumRebarStart;
                    rebarSectionStart->get__EnumRebarSectionItem(&enumRebarStart);

                    while (enumRebarStart->Next(1, &rebarSectionItem, nullptr) != S_FALSE)
                    {
                        CComPtr<IPoint2d> pntRebar;
                        rebarSectionItem->get_Location(&pntRebar);

                        Float64 barStartY = pRebar->GetRebarDepth(pierID, startLoc, xbrTypes::Stage2, pntRebar); // depth from top of cross beam to rebar

                        if (barStartY < 0)
                        {
                            // rebar is not in the cross section (not applicable in this stage)
                            rebarSectionItem.Release();
                            continue;
                        }

                        if (!(seenBarStartYs.find(barStartY) != seenBarStartYs.end()))
                        {
                            barRowPatternStartY = barStartY;
                            seenBarStartYs.insert(barStartY);
                            rebarSectionItem.Release();
                            break;
                        }

                        rebarSectionItem.Release();
                    }

                    CComPtr<IRebarSection> rebarSectionEnd;
                    pRebar->GetRebarSection(pierID, xbrTypes::Stage2, xbrPointOfInterest(endLoc), &rebarSectionEnd);

                    CComPtr<IEnumRebarSectionItem> enumRebarEnd;
                    rebarSectionEnd->get__EnumRebarSectionItem(&enumRebarEnd);

                    while (enumRebarEnd->Next(1, &rebarSectionItem, nullptr) != S_FALSE)
                    {
                        CComPtr<IPoint2d> pntRebar;
                        rebarSectionItem->get_Location(&pntRebar);

                        Float64 barEndY = pRebar->GetRebarDepth(pierID, xbrPointOfInterest(endLoc), xbrTypes::Stage2, pntRebar); // depth from top of cross beam to rebar

                        if (barEndY < 0)
                        {
                            // rebar is not in the cross section (not applicable in this stage)
                            rebarSectionItem.Release();
                            continue;
                        }

                        if (!(seenBarEndYs.find(barEndY) != seenBarEndYs.end()))
                        {
                            barRowPatternEndY = barEndY;
                            seenBarEndYs.insert(barEndY);
                            rebarSectionItem.Release();
                            break;
                        }

                        rebarSectionItem.Release();

                    }

                    rebarDistToBottom = Max(fullDepthStart - barRowPatternStartY, fullDepthEnd - barRowPatternEndY);

                    WBFL::LRFD::REBARDEVLENGTHDETAILS details = WBFL::LRFD::Rebar::GetRebarDevelopmentLengthDetails(
                        size, Ab, db, fy, type, fc, hasFct, Fct, concrete.StrengthDensity, rebarDistToBottom, false, true);

                    WriteRowToDevelopmentTable(pTable, row, barname, details, area, length, stress, scalar, is_LocationFactor);

                    row++;
                }
                else
                {
                    CComPtr<IRebarPattern> rebarPattern;
                    layoutItem->get_Item(patternIdx, &rebarPattern);
                    CComPtr<IRebar> rebar;
                    rebarPattern->get_Rebar(&rebar);

                    CComBSTR barname;
                    rebar->get_Name(&barname);

                    WBFL::Materials::Rebar::Size size = WBFL::LRFD::RebarPool::GetBarSize(OLE2CT(barname));

                    Float64 Ab, db, fy;
                    rebar->get_NominalArea(&Ab);
                    rebar->get_NominalDiameter(&db);
                    rebar->get_YieldStrength(&fy);

                    const CConcreteMaterial& concrete = pProject->GetConcrete(pierID);
                    Float64 fc = concrete.Fc;

                    WBFL::Materials::ConcreteType type = WBFL::Materials::ConcreteType::Normal;
                    bool hasFct = false;
                    Float64 Fct = 0.0;

                    GET_IFACE2(pBroker, IXBRPointOfInterest, pPoi);
                    std::vector<xbrPointOfInterest> vPoi = pPoi->GetXBeamPointsOfInterest(pierID);

                    for (IndexType i = 0; i < vPoi.size(); i++)
                    {
                        const xbrPointOfInterest& poi = vPoi[i];
                    
                        // Height of pier at start and end of bar
                        GET_IFACE2(pBroker, IXBRSectionProperties, pSectProp);
                        Float64 fullDepth = pSectProp->GetDepth(pierID, xbrTypes::Stage2, poi);

                        // get bar depth
                        CComPtr<IRebarSectionItem> rebarSectionItem;

                        CComPtr<IRebarSection> rebarSection;
                        pRebar->GetRebarSection(pierID, xbrTypes::Stage2, xbrPointOfInterest(poi), &rebarSection);

                        CComPtr<IEnumRebarSectionItem> enumRebar;
                        rebarSection->get__EnumRebarSectionItem(&enumRebar);

                        while (enumRebar->Next(1, &rebarSectionItem, nullptr) != S_FALSE)
                        {
                            CComPtr<IPoint2d> pntRebar;
                            rebarSectionItem->get_Location(&pntRebar);

                            Float64 barY = pRebar->GetRebarDepth(pierID, poi, xbrTypes::Stage2, pntRebar); // depth from top of cross beam to rebar

                            if (barY < 0)
                            {
                                // rebar is not in the cross section (not applicable in this stage)
                                rebarSectionItem.Release();
                                continue;
                            }

                            if (!(seenBarYs.find(barY) != seenBarYs.end()))
                            {
                                barRowPatternY = barY;
                                seenBarYs.insert(barY);
                                rebarSectionItem.Release();
                                break;
                            }

                            rebarSectionItem.Release();

                        }
                        
                        rebarDistToBottom = fullDepth - barRowPatternY;

                        WBFL::LRFD::REBARDEVLENGTHDETAILS details = WBFL::LRFD::Rebar::GetRebarDevelopmentLengthDetails(
                            size, Ab, db, fy, type, fc, hasFct, Fct, concrete.StrengthDensity, rebarDistToBottom, false, true);

                        WriteRowToDevelopmentTable(pTable, row, barname, details, 
                        area, length, stress, scalar, is_LocationFactor, &location, &poi);

                        row++;
                    }
                }
            } // next patternIdx
        } // next rebarIdx
    }
    else
    {
        // Cycle over all rebar in section and output development details for each unique size
        RowIndexType row(1);
        std::set<Float64> diamSet;
        for (IndexType rebarIdx = 0; rebarIdx < nRebarRows; rebarIdx++)
        {
            CComPtr<IRebarLayoutItem> layoutItem;
            rebarLayout->get_Item(rebarIdx, &layoutItem);
            IndexType nPatterns;
            layoutItem->get_Count(&nPatterns);
            for (IndexType patternIdx = 0; patternIdx < nPatterns; patternIdx++)
            {
                CComPtr<IRebarPattern> rebarPattern;
                layoutItem->get_Item(patternIdx, &rebarPattern);
                CComPtr<IRebar> rebar;
                rebarPattern->get_Rebar(&rebar);
                Float64 diam;
                rebar->get_NominalDiameter(&diam);
                if (diamSet.end() == diamSet.find(diam))
                {
                    // We have a unique bar
                    diamSet.insert(diam);

                    CComBSTR barname;
                    rebar->get_Name(&barname);

                    WBFL::Materials::Rebar::Size size = WBFL::LRFD::RebarPool::GetBarSize(OLE2CT(barname));

                    Float64 Ab, db, fy;
                    rebar->get_NominalArea(&Ab);
                    rebar->get_NominalDiameter(&db);
                    rebar->get_YieldStrength(&fy);

                    const CConcreteMaterial& concrete = pProject->GetConcrete(pierID);
                    Float64 fc = concrete.Fc;

                    WBFL::Materials::ConcreteType type = WBFL::Materials::ConcreteType::Normal;
                    bool hasFct = false;
                    Float64 Fct = 0.0;

                    // Need distance to bottom of pier. Use max from both ends
                    GET_IFACE2(pBroker, IXBRPier, pPier);
                    Float64 pier_length = pPier->GetXBeamLength(xbrTypes::xblBottomXBeam, pierID);
                    Float64 startLoc, barLength;
                    layoutItem->get_Start(&startLoc);
                    layoutItem->get_Length(&barLength);
                    Float64 endLoc = startLoc + barLength;
             
                    GET_IFACE2(pBroker, IXBRSectionProperties, pSectProp);
					Float64 depthStart = pSectProp->GetDepth(pierID, xbrTypes::Stage2, xbrPointOfInterest(startLoc));
					Float64 depthEnd = pSectProp->GetDepth(pierID, xbrTypes::Stage2, xbrPointOfInterest(endLoc));

                    // elevation of bar at ends
                    CComPtr<IPoint2d> barStart, barEnd;
                    rebarPattern->get_Location(0.0, 0, &barStart);
                    rebarPattern->get_Location(barLength, 0, &barEnd);
                    Float64 barStartY, barEndY;
                    barStart->get_Y(&barStartY);
                    barEnd->get_Y(&barEndY);

                    rebarDistToBottom = Max(depthStart + barStartY, depthEnd + barEndY);

                    WBFL::LRFD::REBARDEVLENGTHDETAILS details = WBFL::LRFD::Rebar::GetRebarDevelopmentLengthDetails(size, Ab, db, fy, type, fc, hasFct, Fct, concrete.StrengthDensity, rebarDistToBottom, false, true);

                    WriteRowToDevelopmentTable(pTable, row, barname, details, area, length, stress, scalar, is_LocationFactor);

                    row++;
                } // end if
            } // next patternIdx
        } // next rebarIdx
    } // end if

    return pChapter;
}

