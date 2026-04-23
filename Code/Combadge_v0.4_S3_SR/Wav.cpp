#include "Wav.h"

// Builds a standard 44-byte PCM WAV header for:
//   16000 Hz, mono, 16-bit linear PCM
// which matches the I2S configuration in setup().
//
// Previously this was hardcoded for 44100 Hz — any recording
// played back on a PC would run at the wrong speed.

void CreateWavHeader(byte* header, int waveDataSize) {
  // --- RIFF chunk ---
  header[0]  = 'R'; header[1]  = 'I'; header[2]  = 'F'; header[3]  = 'F';
  // ChunkSize = file size - 8
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
  header[4]  = (byte)( fileSizeMinus8        & 0xFF);
  header[5]  = (byte)((fileSizeMinus8 >>  8) & 0xFF);
  header[6]  = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7]  = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8]  = 'W'; header[9]  = 'A'; header[10] = 'V'; header[11] = 'E';

  // --- fmt sub-chunk ---
  header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
  header[16] = 0x10; header[17] = 0x00; header[18] = 0x00; header[19] = 0x00; // Subchunk1Size = 16
  header[20] = 0x01; header[21] = 0x00;  // AudioFormat = 1 (PCM)
  header[22] = 0x01; header[23] = 0x00;  // NumChannels = 1 (mono)

  // SampleRate = 16000 (0x3E80)
  header[24] = 0x80; header[25] = 0x3E; header[26] = 0x00; header[27] = 0x00;

  // ByteRate = SampleRate * NumChannels * BitsPerSample/8
  //          = 16000 * 1 * 2 = 32000 (0x7D00)
  header[28] = 0x00; header[29] = 0x7D; header[30] = 0x00; header[31] = 0x00;

  // BlockAlign = NumChannels * BitsPerSample/8 = 1 * 2 = 2
  header[32] = 0x02; header[33] = 0x00;

  // BitsPerSample = 16
  header[34] = 0x10; header[35] = 0x00;

  // --- data sub-chunk ---
  header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
  header[40] = (byte)( waveDataSize        & 0xFF);
  header[41] = (byte)((waveDataSize >>  8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}


//EOF
