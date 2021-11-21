package me.ztiany.androidav.android.camera;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import me.ztiany.androidav.R;

public class DemoActivity extends AppCompatActivity {

    DemoSurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_demo);
        mSurfaceView = (DemoSurfaceView) findViewById(R.id.surface);
    }

    public void capture(View view) {
        mSurfaceView.startCaptrue();
    }

}