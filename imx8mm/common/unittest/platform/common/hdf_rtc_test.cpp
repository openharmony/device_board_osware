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
#include "rtc_test.h"

using namespace testing::ext;

class HdfRtcTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfRtcTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfRtcTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfRtcTest::SetUp()
{
}

void HdfRtcTest::TearDown()
{
}

/**
  * @tc.name: testRtcReadWriteTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcReadWriteMaxTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteMaxTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_MAX_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcReadWriteMinTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteMinTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_MIN_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcReadWriteAlarmTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteAlarmTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_ALARM_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcReadWriteAlarmMaxTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteAlarmMaxTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_ALARM_MAX_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
/**
  * @tc.name: testRtcReadWriteAlarmMinTime001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcReadWriteAlarmMinTime001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_WR_ALARM_MIN_TIME, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

#ifndef HDF_LITEOS_TEST
/**
  * @tc.name: testRtcAlarmEnable001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000F868I
  */
HWTEST_F(HdfRtcTest, testRtcAlarmEnable001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_ALARM_ENABLE, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
#endif

#if defined(HDF_LITEOS_TEST)

/**
  * @tc.name: testRtcAlarmIrq001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcAlarmIrq001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_ALARM_IRQ, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcRegCallback001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcRegCallback001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_REGISTER_CALLBACK, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcRegCallbackNull001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcRegCallbackNull001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_REGISTER_CALLBACK_NULL, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcFreq001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcFreq001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_FREQ, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcMaxFreq001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcMaxFreq001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_MAX_FREQ, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcMinFreq001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcMinFreq001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_MIN_FREQ, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcUserReg001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcUserReg001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_USER_REG, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: testRtcUserRegMaxIndex001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcUserRegMaxIndex001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_USER_REG_MAX_INDEX, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
#endif
/**
  * @tc.name: testRtcReliability001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_WR_RELIABILITY, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

#if defined(HDF_LITEOS_TEST)
/**
  * @tc.name: testRtcModule001
  * @tc.desc: rtc function test
  * @tc.type: FUNC
  * @tc.require: AR000EKRKU
  */
HWTEST_F(HdfRtcTest, testRtcModule001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_RTC_TYPE, RTC_TEST_CMD_RTC_FUNCTION_TEST, -1 };
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
#endif
