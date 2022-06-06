/*
 * Copyright© 2021–2022 Beijing OSWare Technology Co., Ltd
 * This file contains confidential and proprietary information of
 * OSWare Technology Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"
#include "hdf_io_service_if.h"
#include "watchdog_test.h"

using namespace testing::ext;

class HdfLiteWatchdogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteWatchdogTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteWatchdogTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteWatchdogTest::SetUp()
{
}

void HdfLiteWatchdogTest::TearDown()
{
}

/**
  * @tc.name: HdfLiteWatchdogTestSetGetTimeout001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestSetGetTimeout001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_SET_GET_TIMEOUT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestStartStop001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestStartStop001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_START_STOP, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestFeed001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestFeed001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_FEED, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteWatchdogTestReliability001
  * @tc.desc: watchdog function test
  * @tc.type: FUNC
  * @tc.require: AR000F868G
  */
HWTEST_F(HdfLiteWatchdogTest, HdfLiteWatchdogTestReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_WDT_TYPE, WATCHDOG_TEST_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

