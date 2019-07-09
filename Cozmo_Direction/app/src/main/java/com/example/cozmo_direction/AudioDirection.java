package com.example.cozmo_direction;

import android.content.Context;
import android.media.AudioManager;
import android.os.Build;

public enum AudioDirection {

    INSTANCE;

    static {
        System.loadLibrary("native-lib");
    }

    //Native methods
    static native boolean native_create();
    static native boolean native_isAAudioSupported();
    static native boolean native_setAPI(int apiType);
    static native void setDirectionOn(boolean isDirectionOn);
    static native void native_setRecordingDeviceId(int deviceId);
    static native void native_setProcessDeviceId(int deviceId);
    static native void native_delete();
    static native void native_setDefaultSampleRate(int sampleRate);
    static native void native_setDefaultFramesPerBurst(int framesPerBurst);
    static native float test();
}
