package me.ztiany.av.webrtc

import android.os.Bundle
import android.widget.Toast
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import com.permissionx.guolindev.PermissionX
import me.ztiany.av.webrtc.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private val vb by lazy { ActivityMainBinding.inflate(layoutInflater) }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        setContentView(vb.root)

        // default signal server address and room.
        vb.etSignalServerAddress.setText("http://192.168.1.5:8080/")
        vb.etRoomName.setText("SkyRoom")

        ViewCompat.setOnApplyWindowInsetsListener(vb.main) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }

        vb.btnJoin.setOnClickListener {
            PermissionX.init(this)
                .permissions(android.Manifest.permission.CAMERA, android.Manifest.permission.RECORD_AUDIO)
                .request { allGranted, _, _ ->
                    if (allGranted) {
                        joinChecked()
                    } else {
                        Toast.makeText(this, "You need to grant camera and audio permissions to use this feature.", Toast.LENGTH_SHORT).show()
                    }
                }
        }
    }

    private fun joinChecked() = with(vb) {
        val signalServer = etSignalServerAddress.text.toString().trim()
        if (signalServer.isEmpty()) {
            Toast.makeText(this@MainActivity, "The signal server address is required!", Toast.LENGTH_SHORT).show()
            return
        }

        val roomName = etRoomName.text.toString().trim()
        if (roomName.isEmpty()) {
            Toast.makeText(this@MainActivity, "A room name is required!", Toast.LENGTH_SHORT).show()
            return
        }

        doRealJoin(signalServer, roomName)
    }

    private fun doRealJoin(signalServer: String, roomName: String) {
        RoomActivity.start(this, ConnectionInfo(signalServer, roomName, emptyList()))
    }

}