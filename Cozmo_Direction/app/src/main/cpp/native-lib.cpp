#include <jni.h>
#include <string>
#include <math.h>

#include <oboe/Oboe.h>
#include "AudioDirection.h"
//#include "twoDOA.h"
#include "C:\Users\Moi\Documents\Cozmo\oboe\src\common/OboeDebug.h"

static const int kOboeApiAAudio = 0;
static const int kOboeApiOpenSLES = 1;

static AudioDirection *engine = nullptr;

extern "C" {
JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1setDefaultSampleRate(JNIEnv *env,
                                                                              jclass type,
                                                                              jint sampleRate) {
    oboe::DefaultStreamValues::SampleRate = (int32_t) sampleRate;
}

JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1setDefaultFramesPerBurst(JNIEnv *env,
                                                                                  jclass type,
                                                                                  jint framesPerBurst) {
    oboe::DefaultStreamValues::FramesPerBurst = (int32_t) framesPerBurst;
}

JNIEXPORT jboolean JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1create(JNIEnv *env, jclass) {
    if (engine == nullptr) {
        engine = new AudioDirection();
    }

    return (engine != nullptr);
}

JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1delete(JNIEnv *env, jclass) {
    delete engine;
    engine = nullptr;
}

JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1setRecordingDeviceId(JNIEnv *env, jclass,
                                                                              jint deviceId) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine before calling this "
                "method");
        return;
    }

    engine->setRecordingDeviceId(deviceId);
}

JNIEXPORT jboolean JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1setAPI(JNIEnv *env, jclass type,
                                                                jint apiType) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine "
                "before calling this method");
        return JNI_FALSE;
    }

    oboe::AudioApi audioApi;
    switch (apiType) {
        case kOboeApiAAudio:
            audioApi = oboe::AudioApi::AAudio;
            break;
        case kOboeApiOpenSLES:
            audioApi = oboe::AudioApi::OpenSLES;
            break;
        default:
            LOGE("Unknown API selection to setAPI() %d", apiType);
            return JNI_FALSE;
    }

    return static_cast<jboolean>(engine->setAudioApi(audioApi) ? JNI_TRUE
                                                               : JNI_FALSE);
}

JNIEXPORT jboolean JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1isAAudioSupported(JNIEnv *env,
                                                                           jclass type) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine "
                "before calling this method");
        return JNI_FALSE;
    }
    return static_cast<jboolean>(engine->isAAudioSupported() ? JNI_TRUE
                                                             : JNI_FALSE);
}

JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_setDirectionOn(JNIEnv *env, jclass type,
                                                                jboolean isDirectionOn) {

    if (engine == nullptr){
        LOGE( "Engine is null, you must call createEngine before calling this method");
        return;
    }

    engine->setDirectionOn(isDirectionOn);
}
JNIEXPORT jfloat JNICALL
Java_com_example_cozmo_1direction_AudioDirection_test(JNIEnv *env, jclass type) {
    if (engine == nullptr){
        LOGE( "Engine is null, you must call createEngine before calling this method");
        return 0;
    }

    return engine->test();
}

JNIEXPORT void JNICALL
Java_com_example_cozmo_1direction_AudioDirection_native_1setProcessDeviceId(JNIEnv *env,
                                                                            jclass type,
                                                                            jint deviceId) {
    if (engine == nullptr) {
        LOGE(
                "Engine is null, you must call createEngine before calling this "
                "method");
        return;
    }

    engine->setProcessDeviceId(deviceId);
}
}