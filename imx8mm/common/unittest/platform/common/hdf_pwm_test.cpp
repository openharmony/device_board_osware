/*
 * Copyright (C) 2021–2022 Beijing OSWare Technology Co., Ltd
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
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include "hdf_uhdf_test.h"
#include "pwm_test.h"
#include "hdf_io_service_if.h"

using namespace testing::ext;

class HdfLitePwmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLitePwmTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLitePwmTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLitePwmTest::SetUp()
{
}

void HdfLitePwmTest::TearDown()
{
}

/**
  * @tc.name: PwmSetPeriodTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetPeriodTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_PERIOD_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetDutyTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetDutyTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_DUTY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetPolarityTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868C
  */
HWTEST_F(HdfLitePwmTest, PwmSetPolarityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_PWM_TYPE, PWM_SET_POLARITY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmSetGetConfigTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmSetGetConfigTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_SET_GET_CONFIG_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmEnableTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmEnableTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_ENABLE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: PwmDisableTest001
  * @tc.desc: pwm function test
  * @tc.type: FUNC
  * @tc.require: AR000F868D
  */
HWTEST_F(HdfLitePwmTest, PwmDisableTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_PWM_TYPE, PWM_DISABLE_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
