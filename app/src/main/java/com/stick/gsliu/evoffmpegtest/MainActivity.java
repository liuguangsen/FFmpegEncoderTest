package com.stick.gsliu.evoffmpegtest;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final String SDCARD_PATH = Environment.getExternalStorageDirectory().getPath();
    private static final String TEST_INPUT_FILE_PATH = SDCARD_PATH + "/1/aa/testyuv1.yuv";
    private static final String TEST_OUTPUT_FILE_PATH = SDCARD_PATH + "/1/aa/test.h264";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("ijkffmpegt");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        File file = new File(TEST_INPUT_FILE_PATH);
        File file1 = new File(TEST_OUTPUT_FILE_PATH);
        final String input = file.getAbsolutePath();
        final String output = file1.getAbsolutePath();
        new Thread(new Runnable() {
            @Override
            public void run() {
                decode(input, output);
            }
        }).start();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native static void decode(String input,String output);
}
