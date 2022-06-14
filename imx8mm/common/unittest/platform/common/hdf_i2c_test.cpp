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
#include "i2c_test.h"
#include "hdf_io_service_if.h"

using namespace testing::ext;

class HdfLiteI2cTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteI2cTest::SetUpTestCase()
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_SETUP_ALL, -1};
    HdfTestOpenService();
    HdfTestSendMsgToService(&msg);
}

void HdfLiteI2cTest::TearDownTestCase()
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_TEARDOWN_ALL, -1};
    HdfTestSendMsgToService(&msg);
    HdfTestCloseService();
}

void HdfLiteI2cTest::SetUp()
{
}

void HdfLiteI2cTest::TearDown()
{
}

/**
  * @tc.name: HdfLiteI2cTestTransfer001
  * @tc.desc: i2c transfer test
  * @tc.type: FUNC
  * @tc.require: AR000F8688
  */
HWTEST_F(HdfLiteI2cTest, HdfLiteI2cTestTransfer001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_TRANSFER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteI2cTestWriteRead001
  * @tc.desc: i2c write and read test
  * @tc.type: FUNC
  * @tc.require: AR000F8688
  */
HWTEST_F(HdfLiteI2cTest, HdfLiteI2cTestWriteRead001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_WRITE_READ, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteI2cTestMultiThread001
  * @tc.desc: i2c multithread test
  * @tc.type: FUNC
  * @tc.require: AR000F8688
  */
HWTEST_F(HdfLiteI2cTest, HdfLiteI2cTestMultiThread001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_MULTI_THREAD, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: HdfLiteI2cTestReliability001
  * @tc.desc: i2c reliability test
  * @tc.type: FUNC
  * @tc.require: AR000F8688
  */
HWTEST_F(HdfLiteI2cTest, HdfLiteI2cTestReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_I2C_TYPE, I2C_TEST_CMD_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
