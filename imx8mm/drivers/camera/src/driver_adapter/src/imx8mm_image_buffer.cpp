/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "imx8mm_image_buffer.h"

namespace OHOS::Camera {
Imx8mmImageAdditionalInfo::Imx8mmImageAdditionalInfo()
{
    offset_ = 0;
    length_ = 0;
}

Imx8mmImageAdditionalInfo::~Imx8mmImageAdditionalInfo()
{
    Free();
}

Imx8mmImageAdditionalInfo * Imx8mmImageAdditionalInfo::GetInstance()
{
    static Imx8mmImageAdditionalInfo addiInfo;
    return &addiInfo; 
}

uint32_t Imx8mmImageAdditionalInfo::GetOffset() const
{
    return offset_;
}

uint32_t Imx8mmImageAdditionalInfo::GetLength() const
{
    return length_;
}

void Imx8mmImageAdditionalInfo::SetOffset(const uint32_t offset)
{
    offset_ = offset;
    return;
}

void Imx8mmImageAdditionalInfo::SetLength(const uint32_t length)
{
    length_ = length;
    return;
}

void Imx8mmImageAdditionalInfo::Free()
{
    offset_ = 0;
    length_ = 0;

    return;
}

} // namespace OHOS::Camera

