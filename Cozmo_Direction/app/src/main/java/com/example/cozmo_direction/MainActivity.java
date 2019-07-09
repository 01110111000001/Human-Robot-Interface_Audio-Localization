package com.example.cozmo_direction;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioDeviceInfo;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.constraint.solver.widgets.WidgetContainer;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity
        implements ActivityCompat.OnRequestPermissionsResultCallback {

    private static final String TAG = MainActivity.class.getName();
    private static final int AUDIO_DIRECTION_REQUEST = 0;
    private static final int OBOE_API_AAUDIO = 0;
    private static final int OBOE_API_OPENSL_ES = 1;

    private TextView tvAngleNbr, statusText, tvAngle;
    private Button btnDirection;
    private ImageView mic_1, mic_2;
    private ProgressBar progressBar;

    final Handler handler = new Handler();

    private boolean isDirecting = false;

    private int apiSelection = OBOE_API_AAUDIO;
    private boolean aaudioSupported = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        init();

        //Set all the setOnClickListener()

        btnDirection.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                toggleDirection();
            }
        });
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            AudioDirection.native_setRecordingDeviceId(getRecordingDeviceId());
            AudioDirection.native_setProcessDeviceId(getProcessDeviceId());
            AudioManager myAudioMgr = (AudioManager) getApplicationContext().getSystemService(Context.AUDIO_SERVICE);
            String sampleRateStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            int defaultSampleRate = Integer.parseInt(sampleRateStr);
            String framesPerBurstStr = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            int defaultFramesPerBurst = Integer.parseInt(framesPerBurstStr);

            AudioDirection.native_setDefaultSampleRate(defaultSampleRate);
            AudioDirection.native_setDefaultFramesPerBurst(defaultFramesPerBurst);
            statusText.setText("Default params have been set.");
        }

        AudioDirection.native_create();
        aaudioSupported = AudioDirection.native_isAAudioSupported();
        EnableAudioApiUI(true);
        AudioDirection.native_setAPI(apiSelection);
        handler.post(new Runnable() {
            @Override
            public void run() {
                handler.postDelayed(this, 100L);
                if (isDirecting){
                    float angle = AudioDirection.test();
                    tvAngleNbr.setText(String.format("%f", angle));
//                    progressBar.setProgress(Math.round(angle));
//                    if (AudioDirection.test() == 90) {
//                        mic_1.setVisibility(View.VISIBLE);
//                        mic_2.setVisibility(View.INVISIBLE);
//                    } else if (AudioDirection.test() == 270) {
//                        mic_1.setVisibility(View.INVISIBLE);
//                        mic_2.setVisibility(View.VISIBLE);
//                    }
                } else {
                    mic_1.setVisibility(View.INVISIBLE);
                    mic_2.setVisibility(View.INVISIBLE);
                }
            }
        });
    }

    private void init() {
        tvAngle = findViewById(R.id.tvAngle);
        tvAngleNbr = findViewById(R.id.tvAngleNbr);
        btnDirection = findViewById(R.id.btnDirection);
        statusText = findViewById(R.id.statusView);
        mic_1 = findViewById(R.id.mic_1);
        mic_2 = findViewById(R.id.mic_2);
//        progressBar = findViewById(R.id.progressBar);
    }

    private void EnableAudioApiUI(boolean enable) {
        if(!aaudioSupported)
        {
            apiSelection = OBOE_API_OPENSL_ES;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    @Override
    protected void onDestroy() {
        AudioDirection.native_delete();
        super.onDestroy();
    }

    private int getRecordingDeviceId(){
        return (AudioDeviceInfo.TYPE_BUILTIN_MIC);
    }

    private int getProcessDeviceId(){
        return (AudioDeviceInfo.TYPE_IP);
    }

    private boolean isRecordPermissionGranted(){
        return (ActivityCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) ==
                PackageManager.PERMISSION_GRANTED);
    }

    private void requestRecordPermission(){
        ActivityCompat.requestPermissions(
                this,
                new String[]{Manifest.permission.RECORD_AUDIO},
                AUDIO_DIRECTION_REQUEST);
    }

    private void resetStatusView(){
        statusText.setText(R.string.status_warning);
        tvAngleNbr.setText("0");
    }

    private void toggleDirection() {
        if (isDirecting){
            stopDirection();
            EnableAudioApiUI(true);
        } else {
            EnableAudioApiUI(false);
            AudioDirection.native_setAPI(apiSelection);
            startDirection();
        }
    }

    private void startDirection(){
        Log.d(TAG, "Attempting to start");

        if (!isRecordPermissionGranted()){
            requestRecordPermission();
            return;
        }
        AudioDirection.setDirectionOn(true);
        statusText.setText(R.string.status_playing);
        btnDirection.setText(R.string.stop_direction);
        isDirecting = true;
    }

    private void stopDirection(){
        Log.d(TAG, "Directing, attempting to stop");
        AudioDirection.setDirectionOn(false);
        resetStatusView();
        btnDirection.setText(R.string.start_direction);
        isDirecting = false;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {

        if(AUDIO_DIRECTION_REQUEST != requestCode){
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 1 || grantResults[0] != PackageManager.PERMISSION_GRANTED){
            statusText.setText(R.string.status_record_audio_denied);
            Toast.makeText(getApplicationContext(), getString(R.string.need_record_audio_permission),
                    Toast.LENGTH_SHORT).show();
        } else {
            toggleDirection();
        }
    }

}
