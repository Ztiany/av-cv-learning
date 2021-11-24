package me.ztiany.androidav.common;

public class YUVUtils {

    /**
     * NV21 画面旋转 90 度。参考：
     * <p>
     * 1. https://stackoverflow.com/questions/44994510/how-to-convert-rotate-raw-nv21-array-image-android-media-image-from-front-ca
     * 2. https://stackoverflow.com/questions/6853401/camera-pixels-rotated/31425229#31425229
     * </p>
     */
    public static byte[] rotateNV21CW(final byte[] yuv, final int width, final int height, final int rotation) {
        if (rotation == 0) {
            return yuv;
        }

        if (rotation % 90 != 0 || rotation < 0 || rotation > 270) {
            throw new IllegalArgumentException("0 <= rotation < 360, rotation % 90 == 0");
        }

        final byte[] output = new byte[yuv.length];
        final int frameSize = width * height;
        final boolean swap = rotation % 180 != 0;
        final boolean xflip = rotation % 270 != 0;
        final boolean yflip = rotation >= 180;

        for (int j = 0; j < height; j++) {

            for (int i = 0; i < width; i++) {
                final int yIn = j * width + i;
                final int uIn = frameSize + (j >> 1) * width + (i & ~1);
                final int vIn = uIn + 1;

                final int wOut = swap ? height : width;
                final int hOut = swap ? width : height;
                final int iSwapped = swap ? j : i;
                final int jSwapped = swap ? i : j;
                final int iOut = xflip ? wOut - iSwapped - 1 : iSwapped;
                final int jOut = yflip ? hOut - jSwapped - 1 : jSwapped;

                final int yOut = jOut * wOut + iOut;
                final int uOut = frameSize + (jOut >> 1) * wOut + (iOut & ~1);
                final int vOut = uOut + 1;

                output[yOut] = (byte) (0xff & yuv[yIn]);
                output[uOut] = (byte) (0xff & yuv[uIn]);
                output[vOut] = (byte) (0xff & yuv[vIn]);
            }
        }

        return output;
    }

    public static void rotateNV21CW(final byte[] yuv, final byte[] output, final int width, final int height, final int rotation) {
        if (rotation == 0) {
            return;
        }

        if (rotation % 90 != 0 || rotation < 0 || rotation > 270) {
            throw new IllegalArgumentException("0 <= rotation < 360, rotation % 90 == 0");
        }

        final int frameSize = width * height;
        final boolean swap = rotation % 180 != 0;
        final boolean xflip = rotation % 270 != 0;
        final boolean yflip = rotation >= 180;

        for (int j = 0; j < height; j++) {

            for (int i = 0; i < width; i++) {
                final int yIn = j * width + i;
                final int uIn = frameSize + (j >> 1) * width + (i & ~1);
                final int vIn = uIn + 1;

                final int wOut = swap ? height : width;
                final int hOut = swap ? width : height;
                final int iSwapped = swap ? j : i;
                final int jSwapped = swap ? i : j;
                final int iOut = xflip ? wOut - iSwapped - 1 : iSwapped;
                final int jOut = yflip ? hOut - jSwapped - 1 : jSwapped;

                final int yOut = jOut * wOut + iOut;
                final int uOut = frameSize + (jOut >> 1) * wOut + (iOut & ~1);
                final int vOut = uOut + 1;

                output[yOut] = (byte) (0xff & yuv[yIn]);
                output[uOut] = (byte) (0xff & yuv[uIn]);
                output[vOut] = (byte) (0xff & yuv[vIn]);
            }
        }
    }

    /**
     * Camera API2 注意事项：
     * <pre>
     *     <ol>
     *         <li>
     *             图像格式问题：经过在多台设备上测试，明明设置的预览数据格式是 ImageFormat.YUV_420_888（4 个 Y对应一组 UV，即平均 1 个像素占 1.5 个 byte，12 位），但是拿到的数据却都是 YUV_422 格式（2 个 Y 对应一组 UV，即平均 1 个像素占 2 个 byte，16 位），且 U 和 V 的长度都少了一些（在 Oneplus 5 和 Samsung Tab s3 上长度都少了 1 ），也就是  (u.length == v.length) && (y.length / 2 > u.length) && (y.length / 2 ≈ u.length); 而 YUV_420_888 数据的 Y、U、V 关系应该是： y.length / 4 == u.length == v.length;
     *         </li>
     *         <li>
     *             图像宽度不一定为 stride（步长）：在有些设备上，回传的图像的 rowStride 不一定为 previewSize.getWidth()，比如在 OPPO K3 手机上，选择的分辨率为 1520x760，但是回传的图像数据的 rowStride 却是 1536，且总数据少了 16 个像素（Y 少了 16，U 和 V 分别少了 8）。
     *         </li>
     *         <li>
     *             数组越界：Camera2 设置的预览数据格式是 ImageFormat.YUV_420_888 时，回传的 Y,U,V 的关系一般是 (u.length == v.length) && (y.length / 2 > u.length) && (y.length / 2 ≈ u.length)；U 和 V 是有部分缺失的，因此我们在进行数组操作时需要注意越界问题。
     *         </li>
     *     </ol>
     * </pre>
     *
     * 【Camera2】将 Y:U:V == 4:2:2 的数据转换为 nv21。
     *
     * @param y      Y 数据
     * @param u      U 数据
     * @param v      V 数据
     * @param nv21   生成的 nv21，需要预先分配内存。
     * @param stride 步长
     * @param height 图像高度
     */
    public static void yuv422ToYuv420sp(byte[] y, byte[] u, byte[] v, byte[] nv21, int stride, int height) {
        System.arraycopy(y, 0, nv21, 0, y.length);
        // 注意，若 length 值为 y.length * 3 / 2 会有数组越界的风险，需使用真实数据长度计算。
        int length = y.length + u.length / 2 + v.length / 2;
        int uIndex = 0, vIndex = 0;
        int endOfY = stride * height;
        for (int i = endOfY; i < length; i += 2) {
            nv21[i] = v[vIndex];
            nv21[i + 1] = u[uIndex];
            vIndex += 2;
            uIndex += 2;
        }
    }

    /**
     * 【Camera2】将 Y:U:V == 4:1:1 的数据转换为 nv21
     *
     * @param y      Y 数据
     * @param u      U 数据
     * @param v      V 数据
     * @param nv21   生成的nv21，需要预先分配内存
     * @param stride 步长
     * @param height 图像高度
     */
    public static void yuv420ToYuv420sp(byte[] y, byte[] u, byte[] v, byte[] nv21, int stride, int height) {
        System.arraycopy(y, 0, nv21, 0, y.length);
        // 注意，若length值为 y.length * 3 / 2 会有数组越界的风险，需使用真实数据长度计算
        int length = y.length + u.length + v.length;
        int uIndex = 0, vIndex = 0;
        for (int i = stride * height; i < length; i++) {
            nv21[i] = v[vIndex++];
            nv21[i + 1] = u[uIndex++];
        }
    }

    public static void nv21toNV12(byte[] nv21, byte[] nv12) {
        int size = nv21.length;
        int len = size * 2 / 3;//length of y planer.
        System.arraycopy(nv21, 0, nv12, 0, len);
        int i = len;
        while (i < size - 1) {
            nv12[i] = nv21[i + 1];
            nv12[i + 1] = nv21[i];
            i += 2;
        }
    }

    /**
     * NV12 顺时针旋转 90°
     */
    public static void rotateNV12CW90D(byte[] data, byte[] output, int width, int height) {
        int y_len = width * height;
        int uvHeight = height >> 1;
        int k = 0;

        for (int j = 0; j < width; j++) {
            for (int i = height - 1; i >= 0; i--) {
                output[k++] = data[width * i + j];
            }
        }

        for (int j = 0; j < width; j += 2) {
            for (int i = uvHeight - 1; i >= 0; i--) {
                output[k++] = data[y_len + width * i + j];
                output[k++] = data[y_len + width * i + j + 1];
            }
        }
    }

}
