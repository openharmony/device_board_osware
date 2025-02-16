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
#include "hdf_io_service_if.h"

using namespace testing::ext;

enum SdioTestCmd {
    SDIO_DISABLE_FUNC_01 = 0,
    SDIO_ENABLE_FUNC_01,
    SDIO_SET_BLOCK_SIZE_01,
    SDIO_INCR_ADDR_READ_AND_WRITE_BYTES_01,
    SDIO_FIXED_ADDR_READ_AND_WRITE_BYTES_01,
    SDIO_FUNC0_READ_AND_WRITE_BYTES_01,
    SDIO_SET_AND_GET_COMMON_INFO_01,
};

class HdfLiteSdioTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteSdioTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteSdioTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteSdioTest::SetUp()
{
}

void HdfLiteSdioTest::TearDown()
{
}

/**
  * @tc.name: SdioDisableFunc001
  * @tc.desc: SdioDisableFunc Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioDisableFunc001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_DISABLE_FUNC_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioEnableFunc001
  * @tc.desc: SdioEnableFunc Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioEnableFunc001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_ENABLE_FUNC_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioSetBlockSize001
  * @tc.desc: SdioSetBlockSize Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioSetBlockSize001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_SET_BLOCK_SIZE_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioIncrAddrReadAndWriteBytes001
  * @tc.desc: SdioReadBytes and SdioWriteBytes Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioIncrAddrReadAndWriteBytes001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_INCR_ADDR_READ_AND_WRITE_BYTES_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioFixedAddrReadAndWriteBytes001
  * @tc.desc: SdioReadBytesFromFixedAddr and SdioWriteBytesToFixedAddr Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioFixedAddrReadAndWriteBytes001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_FIXED_ADDR_READ_AND_WRITE_BYTES_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioFunc0ReadAndWriteBytes001
  * @tc.desc: SdioReadBytesFromFunc0 and SdioWriteBytesToFunc0 Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioFunc0ReadAndWriteBytes001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_SDIO_TYPE, SDIO_FUNC0_READ_AND_WRITE_BYTES_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SdioSetAndGetCommonInfo001
  * @tc.desc: SdioGetCommonInfo and SdioSetCommonInfo Interface test
  * @tc.type: FUNC
  * @tc.require: AR000F868B
  */
HWTEST_F(HdfLiteSdioTest, SdioSetAndGetCommonInfo001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SDIO_TYPE, SDIO_SET_AND_GET_COMMON_INFO_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
