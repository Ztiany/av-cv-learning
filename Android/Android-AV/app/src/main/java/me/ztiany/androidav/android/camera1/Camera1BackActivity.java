package me.ztiany.androidav.android.camera1;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import me.ztiany.androidav.R;

public class Camera1BackActivity extends AppCompatActivity {

    private Camera1BackSurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.camera_activity_api1_back);
        mSurfaceView = findViewById(R.id.camera_surface);
    }

    public void capture(View view) {
        mSurfaceView.startCapture();
    }

}