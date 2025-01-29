/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "gtest/gtest.h"
#include "musx/musx.h"
#include "test_utils.h"

using namespace musx::dom;

constexpr static musxtest::string_view xml = R"xml(
<?xml version="1.0" encoding="UTF-8"?>
<finale>
  <others>
    <instUsed cmper="0" inci="0">
      <inst>1</inst>
      <trackType>staff</trackType>
      <distFromTop>0</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="1">
      <inst>2</inst>
      <trackType>staff</trackType>
      <distFromTop>-288</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <instUsed cmper="0" inci="2">
      <inst>3</inst>
      <trackType>staff</trackType>
      <distFromTop>-576</distFromTop>
      <range>
        <startMeas>1</startMeas>
        <startEdu>0</startEdu>
        <endMeas>32767</endMeas>
        <endEdu>2147483647</endEdu>
      </range>
    </instUsed>
    <multiStaffInstGroup cmper="2">
      <staffNum1>1</staffNum1>
      <staffNum2>2</staffNum2>
      <staffNum3>3</staffNum3>
    </multiStaffInstGroup>
    <multiStaffGroupID cmper="2">
      <staffGroupID>5</staffGroupID>
    </multiStaffGroupID>
    <partDef cmper="0">
      <nameID>1</nameID>
      <partOrder>0</partOrder>
      <copies>1</copies>
      <printPart/>
    </partDef>
    <staffSpec cmper="1">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>723972d2-a005-492a-b92c-2cb1a6b3f21d</instUuid>
      <transposition>
        <noKeyOpt/>
        <keysig>
          <interval>0</interval>
          <adjust>0</adjust>
        </keysig>
      </transposition>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-4</stemReversal>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
    </staffSpec>
    <staffSpec cmper="2">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>723972d2-a005-492a-b92c-2cb1a6b3f21d</instUuid>
      <defaultClef>3</defaultClef>
      <transposition>
        <noKeyOpt/>
        <keysig>
          <interval>0</interval>
          <adjust>0</adjust>
        </keysig>
      </transposition>
      <hideMeasNums/>
      <hideRepeats/>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-4</stemReversal>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
    </staffSpec>
    <staffSpec cmper="3">
      <staffLines>5</staffLines>
      <lineSpace>24</lineSpace>
      <instUuid>723972d2-a005-492a-b92c-2cb1a6b3f21d</instUuid>
      <defaultClef>3</defaultClef>
      <transposition>
        <noKeyOpt/>
        <keysig>
          <interval>0</interval>
          <adjust>0</adjust>
        </keysig>
      </transposition>
      <hideMeasNums/>
      <hideRepeats/>
      <dwRestOffset>-4</dwRestOffset>
      <wRestOffset>-4</wRestOffset>
      <hRestOffset>-4</hRestOffset>
      <otherRestOffset>-4</otherRestOffset>
      <stemReversal>-4</stemReversal>
      <botRepeatDotOff>-5</botRepeatDotOff>
      <topRepeatDotOff>-3</topRepeatDotOff>
      <vertTabNumOff>-1024</vertTabNumOff>
    </staffSpec>
  </others>
  <details>
    <staffGroup cmper1="0" cmper2="5">
      <startInst>1</startInst>
      <endInst>3</endInst>
      <startMeas>1</startMeas>
      <endMeas>32767</endMeas>
      <fullID>6</fullID>
      <hideName/>
      <abbrvID>7</abbrvID>
      <optimize>asGroup</optimize>
    </staffGroup>
  </details>
</finale>
)xml";

TEST(MultiStaffGroupTest, PopulateFields)
{
    auto doc = musx::factory::DocumentFactory::create<musx::xml::pugi::Document>(xml);
    ASSERT_TRUE(doc);
    auto others = doc->getOthers();
    ASSERT_TRUE(others);

    // Test MultiStaffInstrumentGroup
    auto instGroup = others->get<others::MultiStaffInstrumentGroup>(SCORE_PARTID, 2);
    ASSERT_TRUE(instGroup) << "MultiStaffInstrumentGroup with cmper 2 not found";

    ASSERT_EQ(instGroup->staffNums.size(), 3);
    EXPECT_EQ(instGroup->staffNums[0], 1);
    EXPECT_EQ(instGroup->staffNums[1], 2);
    EXPECT_EQ(instGroup->staffNums[2], 3);

    auto staffAtIndex = instGroup->getStaffAtIndex(1);
    EXPECT_TRUE(staffAtIndex) << "Staff at index 1 not found";
    EXPECT_EQ(staffAtIndex->multiStaffInstId, 2);

    auto indexOfStaff = instGroup->getIndexOf(2);
    EXPECT_TRUE(indexOfStaff.has_value());
    EXPECT_EQ(indexOfStaff.value(), 1);

    auto staffGroup = instGroup->getStaffGroup();
    EXPECT_TRUE(staffGroup) << "Staff group not found";
    EXPECT_EQ(staffGroup->multiStaffGroupId, 2) << "multiStaffGroupId not set for group " << staffGroup->getCmper2();

    // Test MultiStaffGroupId
    auto groupId = others->get<others::MultiStaffGroupId>(SCORE_PARTID, 2);
    ASSERT_TRUE(groupId) << "MultiStaffGroupId with cmper 2 not found";

    EXPECT_EQ(groupId->staffGroupId, 5);
}
