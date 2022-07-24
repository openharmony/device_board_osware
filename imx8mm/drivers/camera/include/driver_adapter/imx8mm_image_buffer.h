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

#ifndef HOS_CAMERA_IMX8MM_IMAGE_INFO_H
#define HOS_CAMERA_IMX8MM_IMAGE_INFO_H

#include <cerrno>
#include <cstdio>
#include <string>

namespace OHOS::Camera {
class Imx8mmImageAdditionalInfo {
public:
    Imx8mmImageAdditionalInfo();

    virtual ~Imx8mmImageAdditionalInfo();

    static Imx8mmImageAdditionalInfo * GetInstance();

    virtual uint32_t GetOffset() const;
    virtual uint32_t GetLength() const;

    virtual void SetOffset(const uint32_t offset);
    virtual void SetLength(const uint32_t length);

    virtual void Free();

private:
    uint32_t offset_ = 0;
    uint32_t length_ = 0;
};
} // namespace OHOS::Camera
#endif

