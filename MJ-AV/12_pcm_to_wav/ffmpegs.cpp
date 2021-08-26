#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

FFmpegs::FFmpegs() {}

void FFmpegs::pcm2wav(WAVHeader &header, const char *pcmFilename, const char *wavFilename) {
  // step1：打开 pcm 文件
  QFile pcmFile(pcmFilename);
  if (!pcmFile.open(QFile::ReadOnly)) {
    qDebug() << "文件打开失败：" << pcmFilename;
    return;
  }
  qDebug() << "step1：打开 pcm 文件";

  // step2：设置正确的头部参数
  header.blockAlign = header.bitsPerSample * header.numChannels >> 3; // blockAlign：>>3 就是除以 8，比特转为字节。
  header.byteRate = header.sampleRate * header.blockAlign;
  header.dataChunkDataSize = pcmFile.size(); //音频数据长度
  header.riffChunkDataSize = header.dataChunkDataSize + sizeof(WAVHeader) - sizeof(header.riffChunkId) - sizeof(header.riffChunkDataSize);
  qDebug() << "sampleRate = " << header.sampleRate;
  qDebug() << "bitsPerSample = " << header.bitsPerSample;
  qDebug() << "numChannels = " << header.numChannels;
  qDebug() << "blockAlign = " << header.blockAlign;
  qDebug() << "dataChunkDataSize = " << header.dataChunkDataSize;
  qDebug() << "riffChunkDataSize = " << header.riffChunkDataSize;
  qDebug() << "audioFormat = " << header.audioFormat;

  // step3：打开 wav 文件
  QFile wavFile(wavFilename);
  if (!wavFile.open(QFile::WriteOnly)) {
    qDebug() << "文件打开失败" << wavFilename;
    pcmFile.close();
    return;
  }
  qDebug() << "step3：打开 wav 文件";

  // step4：写入 WAV 头部
  wavFile.write((const char *)&header, sizeof(WAVHeader));
  qDebug() << "step4：写入 WAV 头部";

  // step5：写入 pcm 数据
  char buf[1024];
  int size;
  while ((size = pcmFile.read(buf, sizeof(buf))) > 0) {
    wavFile.write(buf, size);
  }
  qDebug() << "step5：写入 pcm 数据";

  // step6：关闭文件
  qDebug() << "step6：关闭文件";
  pcmFile.close();
  wavFile.close();
}
