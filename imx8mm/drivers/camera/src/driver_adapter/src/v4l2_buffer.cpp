/*
 * Copyright© 2021–2022 Beijing OSWare Technology Co., Ltd
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

#include <cerrno>
#include <cstdio>
#include <string>
#include "v4l2_buffer.h"
namespace OHOS::Camera {
HosV4L2Buffers::HosV4L2Buffers(enum v4l2_memory memType, enum v4l2_buf_type bufferType)
    : memoryType_(memType), bufferType_(bufferType)
{
}

HosV4L2Buffers::~HosV4L2Buffers() {}

RetCode HosV4L2Buffers::V4L2ReqBuffers(int fd, int unsigned buffCont)
{
    struct v4l2_requestbuffers req = {};

    CAMERA_LOGD("V4L2ReqBuffers buffCont %d\n", buffCont);

    req.count = buffCont;
    req.type = bufferType_;
    req.memory = memoryType_;
    adapterBufferList = (struct AdapterBuffer*)calloc(req.count, sizeof(struct AdapterBuffer));

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        CAMERA_LOGE("does not support memory mapping %s\n", strerror(errno));
        return RC_ERROR;
    }

    if (req.count != buffCont) {
        CAMERA_LOGE("error Insufficient buffer memory on \n");

        req.count = 0;
        req.type = bufferType_;
        req.memory = memoryType_;
        if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
            CAMERA_LOGE("V4L2ReqBuffers does not release buffer	%s\n", strerror(errno));
            return RC_ERROR;
        }

        return RC_ERROR;
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2QueueBuffer(int fd, const std::shared_ptr<FrameSpec>& frameSpec)
{
    struct v4l2_buffer buf = {};
    struct v4l2_plane planes[1] = {};

    if (frameSpec == nullptr) {
        CAMERA_LOGE("V4L2QueueBuffer: frameSpec is NULL\n");
        return RC_ERROR;
    }

    uint32_t index = (uint32_t)frameSpec->buffer_->GetIndex();
    adapterBufferList[index].userBufPtr = frameSpec->buffer_->GetVirAddress();
    adapterBufferList[index].length = frameSpec->buffer_->GetLength();

    buf.index = (uint32_t)frameSpec->buffer_->GetIndex();
    buf.type = bufferType_;
    buf.memory = memoryType_;

    if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        buf.m.planes = planes;
        buf.m.planes[0].length = frameSpec->buffer_->GetSize();
        buf.m.planes[0].m.userptr = (unsigned long)frameSpec->buffer_->GetVirAddress();
        buf.length = 1;

        CAMERA_LOGD("++++++++++++ V4L2QueueBuffer buf.index = %{public}d, buf.length = \
            %{public}d, buf.m.userptr = %{public}p\n", \
            buf.index, buf.m.planes[0].length, (void*)buf.m.planes[0].m.userptr);
    } else if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        switch (memoryType_) {
            case V4L2_MEMORY_MMAP:
                {
                    adapterBufferList[index].start = mmap(NULL, frameSpec->buffer_->GetLength(),
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, frameSpec->buffer_->GetOffset());
                    if (MAP_FAILED == adapterBufferList[index].start) {
                        CAMERA_LOGE("v4l2 mmap failed\n");
                        return RC_ERROR;
                    }
                    CAMERA_LOGD("++++++++++++ V4L2QueueBuffer buf.index = %{public}d, buf.length = \
                        %{public}d, adapterBufferList[%{public}d].start = %{public}p\n", \
                        buf.index, buf.length, index, (void*)adapterBufferList[index].start);

                    break;
                }
            case V4L2_MEMORY_USERPTR:
                {
                    buf.length = frameSpec->buffer_->GetSize();
                    buf.m.userptr = (unsigned long)frameSpec->buffer_->GetVirAddress();

                    CAMERA_LOGD("++++++++++++ V4L2QueueBuffer buf.index = %{public}d, buf.length = \
                        %{public}d, buf.m.userptr = %{public}p\n", \
                        buf.index, buf.length, (void*)buf.m.userptr);
                    break;
                }
            default:
                CAMERA_LOGE("It can not be happening - incorrect memory type\n");
                return RC_ERROR;
        }
    }

    std::lock_guard<std::mutex> l(bufferLock_);
    int rc = ioctl(fd, VIDIOC_QBUF, &buf);
    if (rc < 0) {
        CAMERA_LOGE("ioctl VIDIOC_QBUF failed: %s\n", strerror(errno));
        return RC_ERROR;
    }

    auto itr = queueBuffers_.find(fd);
    if (itr != queueBuffers_.end()) {
        itr->second[buf.index] = frameSpec;
        CAMERA_LOGD("insert frameMap fd = %{public}d buf.index = %{public}d\n", fd, buf.index);
    } else {
        FrameMap frameMap;
        frameMap.insert(std::make_pair(buf.index, frameSpec));
        queueBuffers_.insert(std::make_pair(fd, frameMap));
        CAMERA_LOGD("insert fd = %{public}d buf.index = %{public}d\n", fd, buf.index);
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2DequeueBuffer(int fd)
{
    struct v4l2_buffer buf = {};
    struct v4l2_plane planes[1] = {};

    buf.type = bufferType_;
    buf.memory = memoryType_;

    if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        buf.m.planes = planes;
        buf.length = 1;
    }
    int rc = ioctl(fd, VIDIOC_DQBUF, &buf);
    if (rc < 0) {
        CAMERA_LOGE("ioctl VIDIOC_DQBUF failed: %s\n", strerror(errno));
        return RC_ERROR;
    }

    if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        CAMERA_LOGD("---------------- V4L2DequeueBuffer index = %{public}d buf.m.ptr = %{public}p len = %{public}d\n",
            buf.index, (void*)buf.m.planes[0].m.userptr, buf.m.planes[0].length);
    } else if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        CAMERA_LOGD("---------------- V4L2DequeueBuffer index = %{public}d buf.m.ptr = %{public}p len = %{public}d\n",
            buf.index, (void*)buf.m.userptr, buf.length);
    }

    if (memoryType_ == V4L2_MEMORY_MMAP) {
        memcpy(adapterBufferList[buf.index].userBufPtr,
            adapterBufferList[buf.index].start, adapterBufferList[buf.index].length);

        if (munmap(adapterBufferList[buf.index].start, adapterBufferList[buf.index].length) < 0) {
            CAMERA_LOGE("munmap is fail\n");
            return RC_ERROR;
        }
    }

    auto IterMap = queueBuffers_.find(fd);
    if (IterMap == queueBuffers_.end()) {
        CAMERA_LOGE("std::map queueBuffers_ no fd\n");
        return RC_ERROR;
    }
    auto& bufferMap = IterMap->second;

    auto Iter = bufferMap.find(buf.index);
    if (Iter == bufferMap.end()) {
        CAMERA_LOGE("V4L2DequeueBuffer buf.index == %{public}d is not find in FrameMap\n", buf.index);
        return RC_ERROR;
    }

    if (dequeueBuffer_ == nullptr) {
        CAMERA_LOGE("V4L2DequeueBuffer buf.index == %{public}d no callback\n", buf.index);
        std::lock_guard<std::mutex> l(bufferLock_);
        bufferMap.erase(Iter);
        return RC_ERROR;
    }

    // callback to up
    dequeueBuffer_(Iter->second);
    std::lock_guard<std::mutex> l(bufferLock_);
    bufferMap.erase(Iter);

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2AllocBuffer(int fd, const std::shared_ptr<FrameSpec>& frameSpec)
{
    struct v4l2_buffer buf = {};
    struct v4l2_plane planes[1] = {};
    CAMERA_LOGD("V4L2AllocBuffer\n");

    if (frameSpec == nullptr) {
        CAMERA_LOGE("V4L2AllocBuffer frameSpec is NULL\n");
        return RC_ERROR;
    }

    switch (memoryType_) {
        case V4L2_MEMORY_MMAP:
        case V4L2_MEMORY_USERPTR:
            buf.type = bufferType_;
            buf.memory = memoryType_;
            buf.index = (uint32_t)frameSpec->buffer_->GetIndex();

            if (bufferType_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
                buf.m.planes = planes;
                buf.length = 1;
            }
            CAMERA_LOGD("V4L2_MEMORY_USERPTR Print the cnt: %{public}d\n", buf.index);

            if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
                CAMERA_LOGE("error: ioctl VIDIOC_QUERYBUF failed: %{public}s\n", strerror(errno));
                return RC_ERROR;
            }

            CAMERA_LOGD("buf.length = %{public}d frameSpec->buffer_->GetSize() = %{public}d\n", buf.length,
                        frameSpec->buffer_->GetSize());

            if (buf.length > frameSpec->buffer_->GetSize()) {
                CAMERA_LOGE("ERROR:user buff < V4L2 buf.length\n");
                return RC_ERROR;
            }
            frameSpec->buffer_->SetLength(buf.length);
            frameSpec->buffer_->SetOffset(buf.m.offset);

            break;
        case V4L2_MEMORY_OVERLAY:
            // to do something
            break;

        case V4L2_MEMORY_DMABUF:
            // to do something
            break;

        default:
            CAMERA_LOGE("It can not be happening - incorrect memory type\n");
            return RC_ERROR;
    }

    return RC_OK;
}

RetCode HosV4L2Buffers::V4L2ReleaseBuffers(int fd)
{
    CAMERA_LOGE("HosV4L2Buffers::V4L2ReleaseBuffers\n");

    std::lock_guard<std::mutex> l(bufferLock_);
    queueBuffers_.erase(fd);

    return V4L2ReqBuffers(fd, 0);
}

void HosV4L2Buffers::SetCallback(BufCallback cb)
{
    CAMERA_LOGD("HosV4L2Buffers::SetCallback OK.");
    dequeueBuffer_ = cb;
}
RetCode HosV4L2Buffers::Flush(int fd)
{
    CAMERA_LOGE("HosV4L2Buffers::Flush\n");
    return RC_OK;
}
} // namespace OHOS::Camera
