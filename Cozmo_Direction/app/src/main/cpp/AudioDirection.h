
#ifndef COZMO_DIRECTION_AUDIODIRECTION_H
#define COZMO_DIRECTION_AUDIODIRECTION_H

#include <jni.h>
#include <oboe/Oboe.h>
#include <string>
#include <thread>
#include "twoDOA.h"

using namespace oboe;

class AudioDirection : public AudioStreamCallback{
public:
    AudioDirection();
    ~AudioDirection();
    void setRecordingDeviceId(int32_t deviceId);
    void setProcessDeviceId(int32_t deviceId); //Device used to send data to Cozmo
    void setDirectionOn(bool isOn);

    DataCallbackResult onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames);
    void onErrorBeforeClose(AudioStream *oboeStream, Result error);
    void onErrorAfterClose(AudioStream *oboeStr, Result error);

    bool setAudioApi(AudioApi);
    bool isAAudioSupported(void);
    float test();

private:
    twomicDOA *twoDOA = newDOA(NFFT);
    float j = 0;
    uint32_t i = 0;
    uint16_t NFFT = 2048;
    uint8_t channelCount = 2;
    bool mIsDirectionOn = false;
    uint64_t mProcessedFrameCount = 0;
    uint64_t mSystemStartupFrames = 0;
    int32_t mRecordingDeviceId = kUnspecified;
    int32_t mProcessDeviceId = kUnspecified;
    AudioFormat mFormat = AudioFormat::Float;
    int32_t mSampleRate = DefaultStreamValues::SampleRate;
    int32_t mInputChannelCount = ChannelCount::Stereo;
    int32_t mOutputChannelCount = ChannelCount::Stereo;
    AudioStream *mRecordingStream = nullptr;
    AudioStream *mProcessStream = nullptr;
    std::mutex mRestartingLock;
    AudioApi mAudioApi = AudioApi ::AAudio;

    void openRecordingStream();
    void openProcessStream();

    void startStream(AudioStream *stream);
    void stopStream(AudioStream *stream);
    void closeStream(AudioStream *stream);

    void openAllStreams();
    void closeAllStreams();
    void restartStreams();

    AudioStreamBuilder *setupCommonStreamParameters(
            AudioStreamBuilder *builder);
    AudioStreamBuilder *setupRecordingStreamParameters(
            AudioStreamBuilder *builder);
    AudioStreamBuilder *setupProcessStreamParameters(
            AudioStreamBuilder *builder);

    void warnIfNotLowLatency(AudioStream *stream);

};

#endif //COZMO_DIRECTION_AUDIODIRECTION_H
