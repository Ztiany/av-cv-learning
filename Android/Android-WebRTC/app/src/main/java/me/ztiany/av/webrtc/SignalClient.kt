package me.ztiany.av.webrtc

import android.util.Log
import io.socket.client.IO
import io.socket.client.Socket
import org.json.JSONObject
import java.net.URISyntaxException

private const val TAG = "SignalClient"

class SignalClient(
    private val serverAddress: String,
    private val roomName: String,
    private val mSignalEventListener: SignalEventListener,
    private val onNewLog: (String) -> Unit
) {

    private val socket: Socket by lazy {
        IO.socket(serverAddress, IO.Options().apply {
            secure = false
            reconnection = true
            reconnectionAttempts = 5
            reconnectionDelay = 1000
            reconnectionDelayMax = 5000
            timeout = 5000
        })
    }

    init {
        Log.i(TAG, "joinRoom: $roomName of $serverAddress")
        onNewLog("joinRoom: $roomName of $serverAddress")
    }

    interface SignalEventListener {

        fun onConnected()

        fun onConnectingFailed()

        fun onDisconnected()

        fun onUserJoined(roomName: String, userId: String)

        fun onUserLeft(roomName: String, userId: String)

        fun onRemoteUserJoined(roomName: String)

        fun onRemoteUserLeft(roomName: String, userId: String)

        fun onRoomFull(roomName: String, userId: String)

        fun onMessage(message: JSONObject)
    }

    fun joinRoom() {
        try {
            socket.connect()
        } catch (e: URISyntaxException) {
            Log.e(TAG, "joinRoom: ", e)
            return
        }
        listenSignalEvents()
        socket.emit(Command.JOIN.code, roomName)
    }

    fun leaveRoom() {
        Log.i(TAG, "leaveRoom: $roomName")
        socket.emit(Command.LEAVE.code, roomName)
        socket.close()
    }

    fun sendMessage(message: JSONObject) {
        Log.i(TAG, "sendMessage: $message")
        socket.emit("message", roomName, message)
    }

    //侦听从服务器收到的消息
    private fun listenSignalEvents() {
        socket.on(Socket.EVENT_CONNECT_ERROR) { args: Array<Any?> ->
            Log.e(TAG, "onConnectError: " + args.contentToString())
            onNewLog("onConnectError: " + args.contentToString())
            mSignalEventListener.onConnectingFailed()
        }

        socket.on(Socket.EVENT_CONNECT) { args: Array<Any?> ->
            Log.i(TAG, "onConnected: " + args.contentToString())
            onNewLog("onConnected: " + args.contentToString())
            mSignalEventListener.onConnected()
        }

        socket.on(Socket.EVENT_DISCONNECT) { args: Array<Any?> ->
            Log.i(TAG, "onDisconnected: " + args.contentToString())
            onNewLog("onDisconnected: " + args.contentToString())
            mSignalEventListener.onDisconnected()
        }

        socket.on(Command.ON_JOINED.code) { args: Array<Any> ->
            val roomName = args[0] as String
            val userId = args[1] as String
            mSignalEventListener.onUserJoined(roomName, userId)
            Log.i(TAG, "onUserJoined, room:" + roomName + "uid:" + userId)
            onNewLog("onUserJoined, room:" + roomName + "uid:" + userId)
        }

        socket.on(Command.ON_LEAVE.code) { args: Array<Any> ->
            val roomName = args[0] as String
            val userId = args[1] as String
            mSignalEventListener.onUserLeft(roomName, userId)
            Log.i(TAG, "onUserLeaved, room:" + roomName + "uid:" + userId)
            onNewLog("onUserLeaved, room:" + roomName + "uid:" + userId)
        }

        socket.on(Command.ON_PEER_JOINED.code) { args: Array<Any> ->
            val roomName = args[0] as String
            val userId = args[1] as String
            mSignalEventListener.onRemoteUserJoined(roomName)
            Log.i(TAG, "onRemoteUserJoined, room:" + roomName + "uid:" + userId)
            onNewLog("onRemoteUserJoined, room:" + roomName + "uid:" + userId)
        }

        socket.on(Command.ON_PEER_LEAVE.code) { args: Array<Any> ->
            val roomName = args[0] as String
            val userId = args[1] as String
            mSignalEventListener.onRemoteUserLeft(roomName, userId)
            Log.i(TAG, "onRemoteUserLeaved, room:" + roomName + "uid:" + userId)
            onNewLog("onRemoteUserLeaved, room:" + roomName + "uid:" + userId)
        }

        socket.on(Command.ON_FULL.code) { args: Array<Any> ->
            socket.disconnect()
            socket.close()

            val roomName = args[0] as String
            val userId = args[1] as String

            mSignalEventListener.onRoomFull(roomName, userId)
            Log.i(TAG, "onRoomFull, room:" + roomName + "uid:" + userId)
            onNewLog("onRoomFull, room:" + roomName + "uid:" + userId)
        }

        socket.on(Command.ON_MESSAGE.code) { args: Array<Any> ->
            val roomName = args[0] as String
            val msg = args[1] as JSONObject
            mSignalEventListener.onMessage(msg)
            Log.i(TAG, "onMessage, room:" + roomName + "data:" + msg)
        }
    }

}