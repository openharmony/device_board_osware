/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef OHOS_CAMERA_CAMERA_DEMO_H
#define OHOS_CAMERA_CAMERA_DEMO_H

#include <vector>
#include <map>
#include <iostream>
#include <hdf_log.h>
#include <surface.h>
#include <sys/time.h>
#include <ctime>
#include <display_type.h>
#include "camera.h"
#include "camera_metadata_info.h"
#include "ibuffer.h"
#include "ioffline_stream_operator.h"
#include "icamera_host.h"
#include "camera_host_callback.h"
#include "camera_device_callback.h"
#include "icamera_device.h"
#include "stream_operator_callback.h"
#include "istream_operator_callback.h"
#include "stream_customer.h"
#include "securec.h"
#include "project_camera_demo.h"
#ifdef CAMERA_BUILT_ON_OHOS_LITE
#include "camera_device.h"
#include "camera_host.h"
#include "stream_operator.h"
#else
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "camera_host_proxy.h"
#endif

namespace OHOS::Camera {
enum CaptureMode {
    CAPTURE_PREVIEW = 0,
    CAPTURE_SNAPSHOT,
    CAPTURE_VIDEO,
};

enum DemoActionID {
    STREAM_ID_PREVIEW = 1001,
    STREAM_ID_CAPTURE,
    STREAM_ID_VIDEO,
    CAPTURE_ID_PREVIEW = 2001,
    CAPTURE_ID_CAPTURE,
    CAPTURE_ID_VIDEO,
};

class OhosCameraDemo {
public:
    OhosCameraDemo();
    ~OhosCameraDemo();

    RetCode InitCameraDevice();
    void ReleaseCameraDevice();
    RetCode InitSensors();

    RetCode StartPreviewStream();
    RetCode StartCaptureStream();
    RetCode StartVideoStream();
    RetCode StartDualStreams(const int streamIdSecond);

    RetCode ReleaseAllStream();

    RetCode CaptureOnDualStreams(const int streamIdSecond);
    RetCode CaptureON(const int streamId, const int captureId, CaptureMode mode);
    RetCode CaptureOff(const int captureId, const CaptureMode mode);

    void SetAwbMode(const int mode) const;
    void SetAeExpo();
    void SetMetadata();
    void SetEnableResult();
    void FlashlightOnOff(bool onOff);

    RetCode StreamOffline(const int streamId);

    void QuitDemo();

private:
    void SetStreamInfo(std::shared_ptr<OHOS::Camera::StreamInfo> &streamInfo,
        const std::shared_ptr<StreamCustomer> &streamCustomer,
        const int streamId, const StreamIntent intent);
    void GetStreamOpt();

    RetCode CreateStream(const int streamId, std::shared_ptr<StreamCustomer> &streamCustomer,
        StreamIntent intent);
    RetCode CreateStreams(const int streamIdSecond, StreamIntent intent);

    void StoreImage(const void *bufStart, const uint32_t size) const;
    void StoreVideo(const void *bufStart, const uint32_t size) const;
    void OpenVideoFile();

    RetCode GetFaceDetectMode(std::shared_ptr<CameraAbility> &ability);
    RetCode GetFocalLength(std::shared_ptr<CameraAbility> &ability);
    RetCode GetAvailableFocusModes(std::shared_ptr<CameraAbility> &ability);
    RetCode GetAvailableExposureModes(std::shared_ptr<CameraAbility> &ability);
    RetCode GetExposureCompensationRange(std::shared_ptr<CameraAbility> &ability);
    RetCode GetExposureCompensationSteps(std::shared_ptr<CameraAbility> &ability);
    RetCode GetAvailableMeterModes(std::shared_ptr<CameraAbility> &ability);
    RetCode GetAvailableFlashModes(std::shared_ptr<CameraAbility> &ability);
    RetCode GetMirrorSupported(std::shared_ptr<CameraAbility> &ability);
    RetCode GetStreamBasicConfigurations(std::shared_ptr<CameraAbility> &ability);
    RetCode GetFpsRange(std::shared_ptr<CameraAbility> &ability);
    RetCode GetCameraPosition(std::shared_ptr<CameraAbility> &ability);
    RetCode GetCameraType(std::shared_ptr<CameraAbility> &ability);
    RetCode GetCameraConnectionType(std::shared_ptr<CameraAbility> &ability);
    RetCode GetFaceDetectMaxNum(std::shared_ptr<CameraAbility> &ability);

    int aeStatus_ = 1;
    int videoFd_ = -1;
    unsigned int isPreviewOn_ = 0;
    unsigned int isCaptureOn_ = 0;
    unsigned int isVideoOn_ = 0;

    uint8_t captureQuality_ = 0;
    int32_t captureOrientation_ = 0;
    uint8_t mirrorSwitch_ = 0;
    std::vector<double> gps_;

    std::shared_ptr<StreamCustomer> streamCustomerPreview_ = nullptr;
    std::shared_ptr<StreamCustomer> streamCustomerCapture_ = nullptr;
    std::shared_ptr<StreamCustomer> streamCustomerVideo_ = nullptr;
    std::shared_ptr<CameraAbility> ability_ = nullptr;
    std::shared_ptr<CameraSetting> captureSetting_ = nullptr;
    std::shared_ptr<Camera::CaptureInfo> captureInfo_ = nullptr;
    std::mutex metaDatalock_;

#ifdef CAMERA_BUILT_ON_OHOS_LITE
    std::shared_ptr<CameraHostCallback> hostCallback_ = nullptr;
    std::shared_ptr<IStreamOperator> streamOperator_ = nullptr;
    std::shared_ptr<CameraHost> demoCameraHost_ = nullptr;
    std::shared_ptr<ICameraDevice> demoCameraDevice_ = nullptr;
#else
    OHOS::sptr<CameraHostCallback> hostCallback_ = nullptr;
    OHOS::sptr<IStreamOperator> streamOperator_ = nullptr;
    OHOS::sptr<ICameraHost> demoCameraHost_ = nullptr;
    OHOS::sptr<ICameraDevice> demoCameraDevice_ = nullptr;
#endif
    std::vector<std::string> cameraIds_ = {};
};
#ifdef CAMERA_BUILT_ON_OHOS_LITE

#else
class DemoCameraDeviceCallback : public Camera::CameraDeviceCallbackStub {
public:
    DemoCameraDeviceCallback() = default;
    virtual ~DemoCameraDeviceCallback() = default;
    void OnError(Camera::ErrorType type, int32_t errorMsg) override;
    void OnResult(const uint64_t timestamp,
                  const std::shared_ptr<Camera::CameraMetadata>& result) override;
};
#endif
} // namespace OHOS::Camera
#endif // OHOS_CAMERA_CAMERA_DEMO_H
