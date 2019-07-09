//
// Created by hugo on 05-04-19.
//

#include "AudioDirection.h"
#include "C:\Users\Moi\Documents\Cozmo\oboe\src\common/OboeDebug.h"
#include <assert.h>
//#include <logging_macros.h>
#include <climits>

using namespace oboe;

const float kSystemWarmupTime = 0.5f;

AudioDirection::AudioDirection() {

}

AudioDirection::~AudioDirection() {
    stopStream(mProcessStream);
    stopStream(mRecordingStream);

    closeStream(mProcessStream);
    closeStream(mRecordingStream);
}

void AudioDirection::setRecordingDeviceId(int32_t deviceId) {
    mRecordingDeviceId = deviceId;
}

void AudioDirection::setProcessDeviceId(int32_t deviceId) {
    mProcessDeviceId = deviceId;
}

bool AudioDirection::isAAudioSupported() {
    AudioStreamBuilder builder;
    return builder.isAAudioSupported();
}

bool AudioDirection::setAudioApi(AudioApi api) {
    if (mIsDirectionOn) return false;

    mAudioApi = api;
    return true;
}

void AudioDirection::setDirectionOn(bool isOn){
    if (isOn != mIsDirectionOn){
        mIsDirectionOn = isOn;

        if (isOn){
            openAllStreams();
        } else {
            closeAllStreams();
        }
    }
}

void AudioDirection::openAllStreams() {
    openRecordingStream();
    openProcessStream();
    if(mRecordingStream && mProcessStream){
        twoDOA = newDOA(NFFT);
        startStream(mRecordingStream);
        startStream(mProcessStream);
    } else {
        LOGE("Failed to create recording (%p) and/or playback  (*p) stream", mRecordingStream);
        closeAllStreams();
    }
}

void AudioDirection::closeAllStreams() {
    if (mProcessStream != nullptr){
        closeStream(mProcessStream);
        mProcessStream = nullptr;
    }
    if (mRecordingStream != nullptr) {
        closeStream(mRecordingStream);
        mRecordingStream = nullptr;
    }
}

void AudioDirection::openRecordingStream() {
    AudioStreamBuilder builder;

    setupRecordingStreamParameters(&builder);

    Result result = builder.openStream(&mRecordingStream);
    if(result == Result::OK && mRecordingStream) {
        assert(mRecordingStream->getChannelCount() == mInputChannelCount);
        assert(mRecordingStream->getSampleRate() == mSampleRate);
        assert(mRecordingStream->getFormat() == AudioFormat::Float);

        warnIfNotLowLatency(mRecordingStream);
    } else {
        LOGE("Failed to create recording stream. Error: %s", convertToText(result));
    }
}

void AudioDirection::openProcessStream() {
    AudioStreamBuilder builder;

    setupProcessStreamParameters(&builder);
    Result result = builder.openStream(&mProcessStream);
    if (result == oboe::Result::OK && mProcessStream) {
        mSampleRate = mProcessStream->getSampleRate();

        assert(mProcessStream->getFormat() == oboe::AudioFormat::Float);
        assert(mOutputChannelCount == mProcessStream->getChannelCount());

        mSystemStartupFrames =
                static_cast<uint64_t>(mSampleRate * kSystemWarmupTime);
        mProcessedFrameCount = 0;

        warnIfNotLowLatency(mProcessStream);

    } else {
        LOGE("Failed to create process stream. Error: %s",
             oboe::convertToText(result));
    }
}

AudioStreamBuilder *AudioDirection::setupRecordingStreamParameters(AudioStreamBuilder *builder) {
    setupCommonStreamParameters(builder);
    builder->setCallback(nullptr)
        ->setDeviceId(mRecordingDeviceId)
        ->setDirection(Direction::Input)
        ->setSampleRate(mSampleRate)
        ->setChannelCount(mInputChannelCount);
    return builder;
}

AudioStreamBuilder *AudioDirection::setupProcessStreamParameters(AudioStreamBuilder *builder){
    setupCommonStreamParameters(builder);
    builder->setCallback(this)
            ->setDeviceId(mProcessDeviceId)
            ->setDirection(Direction::Output)
            ->setChannelCount(mOutputChannelCount);
    return builder;
}

AudioStreamBuilder *AudioDirection::setupCommonStreamParameters(AudioStreamBuilder *builder) {
    builder->setAudioApi(mAudioApi)
        ->setFormat(mFormat)
        ->setSharingMode(SharingMode::Exclusive)
        ->setPerformanceMode(PerformanceMode::LowLatency);
    return builder;
}

void AudioDirection::startStream(AudioStream *stream) {
    assert(stream);
    if (stream) {
        Result result = stream->requestStart();
        if (result != Result::OK){
            LOGE("Error starting stream. %s", convertToText(result));
        }
    }
}

void AudioDirection::stopStream(AudioStream *stream){
    if (stream){
        Result result = stream->stop(0L);
        if (result != Result::OK){
            LOGE("Error stopping stream. %s", convertToText(result));
        }
    }
}

void AudioDirection::closeStream(AudioStream *stream) {
    if (stream) {
            Result result = stream->close();
        if (result != oboe::Result::OK) {
            LOGE("Error closing stream. %s", oboe::convertToText(result));
        }
    }
}

void AudioDirection::restartStreams(){
    LOGI("Restarting streams");

    if (mRestartingLock.try_lock()) {
        closeAllStreams();
        openAllStreams();
        mRestartingLock.unlock();
    } else {
        LOGW(
                "Restart stream operation already in progress - ignoring this "
                "request");
        // We were unable to obtain the restarting lock which means the restart
        // operation is currently
        // active. This is probably because we received successive "stream
        // disconnected" events.
        // Internal issue b/63087953
    }
}

void AudioDirection::warnIfNotLowLatency(AudioStream *stream) {
    if(stream->getPerformanceMode() != PerformanceMode::LowLatency){
        LOGW(
                "Stream is NOT low latency."
                "Check your request format, sample rate and channel count");
    }
}

float AudioDirection::test() {
    return j;
}

DataCallbackResult AudioDirection::onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    int32_t prevFrameRead =0, framesRead = 0;
    if (mProcessedFrameCount < mSystemStartupFrames) {
        do{
            prevFrameRead = framesRead;

            ResultWithValue<int32_t> status =
                    mRecordingStream->read(audioData, numFrames, 0);
            framesRead = (!status) ? 0 : status.value();

            if (framesRead == 0) break;
        } while (framesRead);

        framesRead = prevFrameRead;
    } else {
        ResultWithValue<int32_t> status =
                mRecordingStream->read(audioData, numFrames, 1000);
        if (!status){
            LOGE("input stream read error: %s",
                 convertToText(status.error()));
            return DataCallbackResult::Stop;
        }
        framesRead= status.value();
        //j = *static_cast<int32_t*>(audioData);
    }

    if(framesRead < numFrames) {
        int32_t bytesPerFrame = mRecordingStream->getChannelCount() * oboeStream->getBytesPerSample();
        int32_t *padPos = static_cast<int32_t *>(audioData) + framesRead * bytesPerFrame;
        memset(padPos, 0, static_cast<size_t>((numFrames - framesRead) * bytesPerFrame));
    }

    mProcessedFrameCount += numFrames;

    //    float *floatData = (float *) audioData;
    //    j = floatData[0];
    //    j=(sizeof(audioData)/*/sizeof(*floatData)*/);
    double *x_frame = (double *) malloc(numFrames * sizeof(double)); // Define memory space for Frame_x
    double *h_frame = (double *) malloc(numFrames * sizeof(double)); // Define memory space for Frame_h

    float *frameData = (float *) audioData;
    for (int iFrame = 0; iFrame < numFrames; iFrame++) {
        x_frame[iFrame] = frameData[0];
        h_frame[iFrame] = frameData[1];
        frameData += channelCount;
    }

    twoDOA->doIT(twoDOA, x_frame, h_frame, win, i, twoDOA->prevmag_fft_Framex, twoDOA->prevmag_fft_Frameh, twoDOA->prevcorrtheta_est, twoDOA->SFxmax, twoDOA->SFxavg, twoDOA->flagSFx, mSampleRate, numFrames, NFFT);
    //TwomicDOA function estimates source location

    i++;
    j = (float) twoDOA->corrtheta_est;


//    int32_t data = *static_cast<int32_t *>(audioData);
//    int16_t x_i = (data>>16);
//    int16_t h_i = data & 0xffff;
//
//    double x_f = (double) x_i/1000.0f;
//    double h_f = (double) h_i/1000.0f;
//    if (abs(x_f) >= 1 || abs(h_f) >=1) {
//        if (abs(x_f)*1.05 > abs(h_f)) j = 90;
//        else if (abs(x_f) < abs(h_f)) j = 270;
//    }
    
    /* For future application, like 360° directionality
    we should use this function, with FFT from both microphones */
    //j = twoDOAProcess(x_f, h_f, numFrames);

    return DataCallbackResult::Continue;
}

void AudioDirection::onErrorBeforeClose(AudioStream *oboeStream, Result error) {
    LOGE("%s stream Error before close: %s",
         convertToText(oboeStream->getDirection()),
         convertToText(error));
}

void AudioDirection::onErrorAfterClose(AudioStream *oboeStream, Result error) {
    LOGE("%s stream Error after close: %s",
         convertToText(oboeStream->getDirection()),
         convertToText(error));
}


