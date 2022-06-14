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
#include "spi_test.h"

using namespace testing::ext;

class HdfLiteSpiTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteSpiTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteSpiTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteSpiTest::SetUp()
{
}

void HdfLiteSpiTest::TearDown()
{
}

/**
  * @tc.name: SpiTransferTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
HWTEST_F(HdfLiteSpiTest, SpiTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiMultiTransferTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
#ifdef __LITEOS__
HWTEST_F(HdfLiteSpiTest, SpiMultiTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_MULTI_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));

    EXPECT_EQ(0, SpiTestExecute(SPI_MULTI_TRANSFER_TEST));
}
#endif

/**
  * @tc.name: SpiDmaTransferTest001
  * @tc.desc: Spi function test
  * @tc.type: FUNC
  * @tc.require: NA
  */
HWTEST_F(HdfLiteSpiTest, SpiDmaTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_DMA_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiIntTransferTest001
  * @tc.desc: Spi function test
  * @tc.type: FUNC
  * @tc.require: NA
  */
HWTEST_F(HdfLiteSpiTest, SpiIntTransferTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_INT_TRANSFER_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
  * @tc.name: SpiReliabilityTest001
  * @tc.desc: spi function test
  * @tc.type: FUNC
  * @tc.require: SR000DQ0VO
  */
HWTEST_F(HdfLiteSpiTest, SpiReliabilityTest001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_SPI_TYPE, SPI_RELIABILITY_TEST, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
