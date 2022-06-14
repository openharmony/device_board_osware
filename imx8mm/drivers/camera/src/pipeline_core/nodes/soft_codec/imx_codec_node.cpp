/*
 * Copyright (C) 2021–2022 Beijing OSWare Technology Co., Ltd
 * This file contains confidential and proprietary information of
 * OSWare Technology Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "imx_codec_node.h"
#include <securec.h>

namespace OHOS::Camera {
uint32_t IMXCodecNode::previewWidth_ = 0;
uint32_t IMXCodecNode::previewHeight_ = 0;
const unsigned long long TIME_CONVERSION_NS_S = 1000000000ULL; /* ns to s */

const char val_num_0 = 0;
const char val_num_1 = 1;
const char val_num_2 = 2;
const char val_num_3 = 3;
const char val_num_4 = 4;
const char val_num_5 = 5;
const char val_num_6 = 6;
const char val_num_7 = 7;
const char val_num_8 = 8;
const char val_num_16 = 16;
const int val_num_128 = 128;
const int val_num_255 = 255;
const int val_num_380 = 380;

const int val_num_812 = 812;
const int val_num_813 = 813;
const int val_num_1000 = 1000;
const int val_num_1159 = 1159;
const int val_num_1164 = 1164;
const int val_num_2018 = 2018;

IMXCodecNode::IMXCodecNode(const std::string& name, const std::string& type) : NodeBase(name, type)
{
    CAMERA_LOGV("%{public}s enter, type(%{public}s)\n", name_.c_str(), type_.c_str());
}

IMXCodecNode::~IMXCodecNode()
{
    CAMERA_LOGI("~IMXCodecNode Node exit.");
}

RetCode IMXCodecNode::Start(const int32_t streamId)
{
    CAMERA_LOGI("IMXCodecNode::Start streamId = %{public}d\n", streamId);
    return RC_OK;
}

RetCode IMXCodecNode::Stop(const int32_t streamId)
{
    CAMERA_LOGI("IMXCodecNode::Stop streamId = %{public}d\n", streamId);
    return RC_OK;
}

RetCode IMXCodecNode::Flush(const int32_t streamId)
{
    CAMERA_LOGI("IMXCodecNode::Flush streamId = %{public}d\n", streamId);
    return RC_OK;
}

void yuvtorgb(int y, int u, int v, int sign, unsigned char *pixel32)
{
    int r, g, b, a;
    static long int ruv, guv, buv;

    if (sign) {
        ruv = val_num_1159 * (v - val_num_128);
        guv = val_num_380 * (u - val_num_128) + val_num_813 * (v - val_num_128);
        buv = val_num_2018 * (u - val_num_128);
    }

    r = (val_num_1164 * (y - val_num_16) + ruv) / val_num_1000;
    g = (val_num_1164 * (y - val_num_16) - guv) / val_num_1000;
    b = (val_num_1164 * (y - val_num_16) + buv) / val_num_1000;

    if (r > val_num_255) {
        r = val_num_255;
    }
    if (g > val_num_255) {
        g = val_num_255;
    }
    if (b > val_num_255) {
        b = val_num_255;
    }
    if (r < val_num_0) {
        r = val_num_0;
    }
    if (g < val_num_0) {
        g = val_num_0;
    }
    if (b < val_num_0) {
        b = val_num_0;
    }
    a = 0xff;

    pixel32[val_num_0] = r;
    pixel32[val_num_1] = g;
    pixel32[val_num_2] = b;
    pixel32[val_num_3] = a;

    return;
}

void IMXCodecNode::Yuv422ToRGBA8888(std::shared_ptr<IBuffer>& buffer)
{
    unsigned int in, out;
    int y0, u, y1, v;
    unsigned int pixel32;
    unsigned char *pixel = (unsigned char *)&pixel32;
    int ret = 0;
    unsigned char *temp = nullptr;
    unsigned char *yuyv = nullptr;

    CAMERA_LOGE("IMXCodecNode::Yuv422ToRGBA8888 enter");
    if (buffer == nullptr) {
        CAMERA_LOGE("IMXCodecNode::Yuv422ToRGBA8888 buffer is nullptr");
        return;
    }

    CAMERA_LOGE("IMXCodecNode::Yuv422ToRGBA8888 buffer->GetSize() = %{public}d", buffer->GetSize());
    temp = (unsigned char *)malloc(buffer->GetSize());
    if (temp == nullptr) {
        CAMERA_LOGI("IMXCodecNode::Yuv422ToRGBA8888 malloc temp == nullptr");
        return;
    }

    yuyv = (unsigned char *)buffer->GetVirAddress();
    if (yuyv == nullptr) {
        CAMERA_LOGI("IMXCodecNode::Yuv422ToRGBA8888 malloc yuyv == nullptr");
        return;
    }

    for (in = 0, out = 0; in < (buffer->GetSize() / val_num_2); in += val_num_4, out += val_num_8) {
        y0 = yuyv[in + val_num_0];
        u  = yuyv[in + val_num_1];
        y1 = yuyv[in + val_num_2];
        v  = yuyv[in + val_num_3];

        yuvtorgb(y0, u, v, val_num_1, pixel);
        temp[out + val_num_0] = pixel[val_num_0];
        temp[out + val_num_1] = pixel[val_num_1];
        temp[out + val_num_2] = pixel[val_num_2];
        temp[out + val_num_3] = pixel[val_num_3];

        yuvtorgb(y1, u, v, val_num_0, pixel);
        temp[out + val_num_4] = pixel[val_num_0];
        temp[out + val_num_5] = pixel[val_num_1];
        temp[out + val_num_6] = pixel[val_num_2];
        temp[out + val_num_7] = pixel[val_num_3];
    }

    ret = memcpy_s(buffer->GetVirAddress(), buffer->GetSize(), (const void *)temp, buffer->GetSize());
    if (ret != 0) {
        CAMERA_LOGI("IMXCodecNode::Yuv422ToRGBA8888 memcpy_s failed!");
    }

    free(temp);
    CAMERA_LOGE("IMXCodecNode::Yuv422ToRGBA8888 exit");
    return;
}

void IMXCodecNode::DeliverBuffer(std::shared_ptr<IBuffer>& buffer)
{
    if (buffer == nullptr) {
        CAMERA_LOGE("IMXCodecNode::DeliverBuffer frameSpec is null");
        return;
    }

    int32_t id = buffer->GetStreamId();
    CAMERA_LOGE("IMXCodecNode::DeliverBuffer StreamId %{public}d", id);

    Yuv422ToRGBA8888(buffer);

    outPutPorts_ = GetOutPorts();
    for (auto& it : outPutPorts_) {
        if (it->format_.streamId_ == id) {
            it->DeliverBuffer(buffer);
            CAMERA_LOGI("IMXCodecNode deliver buffer streamid = %{public}d", it->format_.streamId_);
            return;
        }
    }
}

RetCode IMXCodecNode::Capture(const int32_t streamId, const int32_t captureId)
{
    CAMERA_LOGV("IMXCodecNode::Capture");
    return RC_OK;
}

RetCode IMXCodecNode::CancelCapture(const int32_t streamId)
{
    CAMERA_LOGI("IMXCodecNode::CancelCapture streamid = %{public}d", streamId);

    return RC_OK;
}

REGISTERNODE(IMXCodecNode, {"IMXCodec"})
}  // namespace OHOS::Camera
