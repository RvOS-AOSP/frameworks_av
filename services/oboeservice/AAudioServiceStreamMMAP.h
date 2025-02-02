/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AAUDIO_AAUDIO_SERVICE_STREAM_MMAP_H
#define AAUDIO_AAUDIO_SERVICE_STREAM_MMAP_H

#include <atomic>

#include <android-base/thread_annotations.h>
#include <android-base/unique_fd.h>
#include <media/audiohal/StreamHalInterface.h>
#include <media/MmapStreamCallback.h>
#include <media/MmapStreamInterface.h>
#include <utils/RefBase.h>
#include <utils/String16.h>
#include <utils/Vector.h>

#include "binding/AAudioServiceMessage.h"
#include "AAudioServiceStreamBase.h"
#include "binding/AudioEndpointParcelable.h"
#include "SharedMemoryProxy.h"
#include "TimestampScheduler.h"
#include "utility/MonotonicCounter.h"

namespace aaudio {

/**
 * These corresponds to an EXCLUSIVE mode MMAP client stream.
 * It has exclusive use of one AAudioServiceEndpointMMAP to communicate with the underlying
 * device or port.
 */
class AAudioServiceStreamMMAP : public AAudioServiceStreamBase {

public:
    AAudioServiceStreamMMAP(android::AAudioService &aAudioService,
                            bool inService);
    ~AAudioServiceStreamMMAP() override = default;

    aaudio_result_t open(const aaudio::AAudioStreamRequest &request) override
            EXCLUDES(mUpMessageQueueLock);

    aaudio_result_t startClient(const android::AudioClient& client,
                                const audio_attributes_t *attr,
                                audio_port_handle_t *clientHandle) override;

    aaudio_result_t stopClient(audio_port_handle_t clientHandle) override;

    const char *getTypeText() const override { return "MMAP"; }

protected:

    /**
     * Stop the flow of data so that start() can resume without loss of data.
     *
     * This is not guaranteed to be synchronous but it currently is.
     * An AAUDIO_SERVICE_EVENT_PAUSED will be sent to the client when complete.
    */
    aaudio_result_t pause_l() REQUIRES(mLock) override;

    aaudio_result_t stop_l() REQUIRES(mLock) override;

    aaudio_result_t standby_l() REQUIRES(mLock) override;
    bool isStandbyImplemented() override {
        return true;
    }

    aaudio_result_t exitStandby_l(AudioEndpointParcelable* parcelable) REQUIRES(mLock) override;

    aaudio_result_t getAudioDataDescription_l(
            AudioEndpointParcelable* parcelable) REQUIRES(mLock) override;

    aaudio_result_t getFreeRunningPosition_l(int64_t *positionFrames,
            int64_t *timeNanos) REQUIRES(mLock) override;

    aaudio_result_t getHardwareTimestamp_l(
            int64_t *positionFrames, int64_t *timeNanos) REQUIRES(mLock) override;

    int64_t nextDataReportTime_l() REQUIRES(mLock) override;

    void reportData_l() REQUIRES(mLock) override;

    /**
     * Device specific startup.
     * @return AAUDIO_OK or negative error.
     */
    aaudio_result_t startDevice_l() REQUIRES(mLock) override;

    aaudio_result_t startClient_l(const android::AudioClient& client,
                                  const audio_attributes_t *attr,
                                  audio_port_handle_t *clientHandle) REQUIRES(mLock) override;

    aaudio_result_t stopClient_l(audio_port_handle_t clientHandle) REQUIRES(mLock) override;

private:

    bool                     mInService = false;
};

} // namespace aaudio

#endif //AAUDIO_AAUDIO_SERVICE_STREAM_MMAP_H
