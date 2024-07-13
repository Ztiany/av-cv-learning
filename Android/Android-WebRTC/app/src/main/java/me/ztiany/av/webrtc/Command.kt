package me.ztiany.av.webrtc

enum class Command(val code: String) {

    JOIN("join"),
    ON_JOINED("on_joined"),
    ON_FULL("on_full"),
    ON_PEER_JOINED("on_peer_joined"),
    LEAVE("leave"),
    ON_LEAVE("on_leave"),
    ON_PEER_LEAVE("on_peer_leave"),
    ON_MESSAGE("message"),

}