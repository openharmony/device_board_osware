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
#include <string>
#include <unistd.h>
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"
#include "hdf_io_service_if.h"
#include "uart_test.h"

using namespace testing::ext;

class HdfLiteUartTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteUartTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteUartTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteUartTest::SetUp()
{
}

void HdfLiteUartTest::TearDown()
{
}

#ifdef HDF_LITEOS_TEST
/**
 * @tc.name: UartSetTransModeTest001
 * @tc.desc: uart function test
 * @tc.type: FUNC
 * @tc.require: AR000F8689
 */
HWTEST_F(HdfLiteUartTest, UartSetTransModeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_SET_TRANSMODE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
#endif

/**
  * @tc.name: UartWriteTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartWriteTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_WRITE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartReadTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartReadTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = { TEST_PAL_UART_TYPE, UART_TEST_CMD_READ, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartSetBaudTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartSetBaudTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_SET_BAUD, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartGetBaudTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartGetBaudTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_GET_BAUD, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartSetAttributeTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartSetAttributeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_SET_ATTRIBUTE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartGetAttributeTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartGetAttributeTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_GET_ATTRIBUTE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: UartReliabilityTest001
  * @tc.desc: uart function test
  * @tc.type: FUNC
  * @tc.require: AR000F8689
  */
HWTEST_F(HdfLiteUartTest, UartReliabilityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_UART_TYPE, UART_TEST_CMD_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
