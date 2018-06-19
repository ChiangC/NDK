package com.fmtech.fmphix;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "FMPhix";
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    TextView result;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        result = (TextView) findViewById(R.id.result);
        checkPermission();
    }

    private void checkPermission() {
        if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE}, 100);
        } else {
            init();
        }
    }

    private void init(){
        DexManager.getInstance().setContext(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults[0] == PackageManager.PERMISSION_GRANTED){
            init();
        }
    }

    public void calculate(View view) {
        Calculator calculator =new Calculator();

        Calculator calculator1 =new Calculator();
        Calculator calculator3 =new Calculator();
        calculator.calculate();
        calculator1.calculate();
        calculator3.calculate();
        result.setText("Result: "+ calculator3.calculate());
        Log.i(TAG, "Calculate: "+ calculator1.calculate());
    }

    public void fix(View view) {
        DexManager.getInstance().loadDexFile(new File(Environment.getExternalStorageDirectory(),"fixed.dex"));
    }

}
