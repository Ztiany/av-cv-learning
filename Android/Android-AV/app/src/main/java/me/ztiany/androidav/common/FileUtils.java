package me.ztiany.androidav.common;

import android.content.res.AssetFileDescriptor;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.channels.FileChannel;

import me.ztiany.androidav.AppContext;

public class FileUtils {

    public static void copyAssets(String assetsName, String path) throws IOException {
        AssetFileDescriptor assetFileDescriptor = AppContext.get().getAssets().openFd(assetsName);
        FileChannel from = new FileInputStream(assetFileDescriptor.getFileDescriptor()).getChannel();
        FileChannel to = new FileOutputStream(path).getChannel();
        from.transferTo(assetFileDescriptor.getStartOffset(), assetFileDescriptor.getLength(), to);
    }

    public static void writeBytes(OutputStream outputStream, byte[] array, boolean endFlag) {
        try {
            outputStream.write(array);
            if (endFlag) {
                outputStream.write('\n');
                outputStream.write('\n');
                outputStream.write('\n');
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static final char[] HEX_CHAR_TABLE = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    public static void writeContent(FileWriter writer, byte[] array) {
        StringBuilder sb = new StringBuilder();
        for (byte b : array) {
            sb.append(HEX_CHAR_TABLE[(b & 0xf0) >> 4]);
            sb.append(HEX_CHAR_TABLE[b & 0x0f]);
        }
        try {
            writer.write(sb.toString());
            writer.write("\n");
            writer.write("\n");
            writer.write("\n");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}