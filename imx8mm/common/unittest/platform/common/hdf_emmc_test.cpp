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
#include "emmc_if.h"
#include "hdf_io_service_if.h"
#include "hdf_uhdf_test.h"

using namespace testing::ext;

enum EmmcTestCmd {
    EMMC_GET_CID_01 = 0,
};

class HdfLiteEmmcTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfLiteEmmcTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfLiteEmmcTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfLiteEmmcTest::SetUp()
{
}

void HdfLiteEmmcTest::TearDown()
{
}


/**
  * @tc.name: EmmcGetCid001
  * @tc.desc: test EmmcGetCid/EmmcGetHuid interface in kernel and user status.
  * @tc.type: FUNC
  * @tc.require: AR000F5LSD AR000F5LV3
  */
HWTEST_F(HdfLiteEmmcTest, EmmcGetCid001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_EMMC_TYPE, EMMC_GET_CID_01, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
