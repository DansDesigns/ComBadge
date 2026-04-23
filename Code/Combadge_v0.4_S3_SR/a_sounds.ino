/*  sounds.ino

    Embedded audio playback for ComBadge.

    Uses a dedicated I2S TX channel on I2S_NUM_1 via the raw ESP-IDF driver.
    This is completely independent of ESP-SR which owns I2S_NUM_0 (the mic).
    No need to suspend ESP-SR, no need to end/begin the shared i2s object.

    Shares the same BCK/WS/DOUT pins as ESP-SR — the MAX98357 amp receives
    both clocks from the same lines, but only DOUT from I2S_NUM_1 drives it.

    Public API:
      soundsSetup()             — call once in setup() before ESP-SR
      soundsLoadFromSD()        — call once AFTER SD.begin(); loads chirp.wav if present
      playSoundAsync(SOUND_xxx) — fire-and-forget, safe to call from anywhere

    SD overrides:
      Place /chirp.wav (16kHz, 16-bit, mono PCM WAV) on the SD card.
      If found at boot, it replaces the built-in chirp.h data for SOUND_CHIRP.
      Remove the file to revert to the built-in chirp.

    Adding a new sound:
      1. Convert WAV to .h (16kHz, 16-bit, mono) and drop in sketch folder
      2. #include it below
      3. Add SOUND_xxx to the SoundID enum
      4. Add matching case to _soundTask() switch
*/

#include "driver/i2s_std.h"
#include <SD.h>

// ==================== Sound Clip Headers ====================
#include "chirp.h"
#include "ring.h"
// #include "beep.h"
// #include "callend.h"

// ==================== Sound IDs ====================
enum SoundID {
  SOUND_CHIRP,
  SOUND_RING,
  // SOUND_BEEP,
  // SOUND_CALL_END,
};
void playSoundAsync(SoundID id);

// ==================== TX channel handle ====================
static i2s_chan_handle_t _tx_handle = NULL;

// ==================== SD chirp override ====================
// If /chirp.wav is found on the SD card at boot, its PCM payload is loaded
// here and used in place of the built-in chirp.h data.
static uint8_t* _sd_chirp_data = nullptr;
static uint32_t _sd_chirp_len  = 0;

#define WAV_HEADER_BYTES 44   // standard PCM WAV header size

// Call this once after SD.begin() succeeds.
// Tries to load /chirp.wav from the SD card into a heap buffer.
// Safe to call even if SD is not mounted — it will just do nothing.
void soundsLoadFromSD() {
  const char* path = "/chirp.wav";

  if (!SD.exists(path)) {
    debugln("Sound: /chirp.wav not found on SD — using built-in chirp.");
    return;
  }

  File f = SD.open(path, FILE_READ);
  if (!f) {
    debugln("Sound: failed to open /chirp.wav — using built-in chirp.");
    return;
  }

  uint32_t fileSize = f.size();
  if (fileSize <= WAV_HEADER_BYTES) {
    debugln("Sound: /chirp.wav too small to be valid — using built-in chirp.");
    f.close();
    return;
  }

  uint32_t pcmLen = fileSize - WAV_HEADER_BYTES;

  uint8_t* buf = (uint8_t*)heap_caps_malloc(pcmLen, MALLOC_CAP_INTERNAL);
  if (!buf) {
    debugln("Sound: not enough RAM for /chirp.wav — using built-in chirp.");
    f.close();
    return;
  }

  // Skip WAV header
  f.seek(WAV_HEADER_BYTES);
  uint32_t bytesRead = f.read(buf, pcmLen);
  f.close();

  if (bytesRead != pcmLen) {
    debugln("Sound: /chirp.wav read incomplete — using built-in chirp.");
    heap_caps_free(buf);
    return;
  }

  // Free any previous buffer (shouldn't exist on first call, but be safe)
  if (_sd_chirp_data) {
    heap_caps_free(_sd_chirp_data);
  }

  _sd_chirp_data = buf;
  _sd_chirp_len  = pcmLen;

  debug("Sound: loaded /chirp.wav from SD (");
  debug(pcmLen);
  debugln(" bytes PCM).");
}

// ==================== Setup ====================
// Call once in setup(), before ESP-SR.begin() — opens I2S_NUM_1 TX only.
void soundsSetup() {
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;

  esp_err_t err = i2s_new_channel(&chan_cfg, &_tx_handle, NULL);
  if (err != ESP_OK) {
    debugln("Sound: i2s_new_channel failed");
    return;
  }

  i2s_std_config_t std_cfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2S_PIN_BCK,
      .ws   = (gpio_num_t)I2S_PIN_WS,
      .dout = (gpio_num_t)I2S_PIN_DOUT,
      .din  = I2S_GPIO_UNUSED,
      .invert_flags = {
        .mclk_inv = false,
        .bclk_inv = false,
        .ws_inv   = false,
      },
    },
  };

  err = i2s_channel_init_std_mode(_tx_handle, &std_cfg);
  if (err != ESP_OK) {
    debugln("Sound: i2s_channel_init_std_mode failed");
    return;
  }

  err = i2s_channel_enable(_tx_handle);
  if (err != ESP_OK) {
    debugln("Sound: i2s_channel_enable failed");
    return;
  }

  debugln("Sound: I2S_NUM_1 TX ready.");
}

// ==================== Internal write ====================
#define SOUND_BUF_SIZE 512

#define SOUND_CHUNK_SAMPLES 256

static void _writeToI2S(const uint8_t* data, uint32_t len) {
  if (_tx_handle == NULL) { debugln("Sound: TX not initialised"); return; }

  // Allocate small reusable stereo chunk — no MALLOC_CAP_DMA needed on S3
  int16_t* buf = (int16_t*)heap_caps_malloc(SOUND_CHUNK_SAMPLES * 4, MALLOC_CAP_INTERNAL);
  if (!buf) { debugln("Sound: alloc failed"); return; }

  const int16_t* src     = (const int16_t*)data;
  uint32_t       samples = len / 2;
  uint32_t       offset  = 0;

  while (offset < samples) {
    uint32_t chunk = min((uint32_t)SOUND_CHUNK_SAMPLES, samples - offset);
    for (uint32_t i = 0; i < chunk; i++) {
      buf[i * 2]     = src[offset + i];  // L
      buf[i * 2 + 1] = src[offset + i];  // R
    }
    size_t written = 0;
    i2s_channel_write(_tx_handle, buf, chunk * 4, &written, portMAX_DELAY);
    offset += chunk;
  }

  heap_caps_free(buf);
}

// ==================== FreeRTOS task ====================
static void _soundTask(void* pvParams) {
  SoundID id = *((SoundID*)pvParams);
  delete (SoundID*)pvParams;


  //--------------- debug output ------------------
  debug("Sound: free internal DMA RAM: ");
  debugln(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA));
  debug("Sound: largest free DMA block: ");
  debugln(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA));
  //----------------------------------------

  
  switch (id) {
    case SOUND_CHIRP:
      debugln("Sound: CHIRP");
      if (_sd_chirp_data && _sd_chirp_len > 0) {
        // Use SD card override
        debugln("Sound: CHIRP (SD)");
        _writeToI2S(_sd_chirp_data, _sd_chirp_len);
      } else {
        // Fall back to built-in chirp.h
        debugln("Sound: CHIRP (built-in)");
        _writeToI2S(chirp_data, CHIRP_DATA_LEN);
      }
      break;
    case SOUND_RING:
      debugln("Sound: RING");
      _writeToI2S(ring_data, RING_DATA_LEN);
      break;
    // case SOUND_BEEP:
    //   _writeToI2S(beep_data, BEEP_DATA_LEN);
    //   break;
    default:
      debugln("Sound: unknown ID");
      break;
  }

  vTaskDelete(NULL);
}


// ==================== Public API ====================
void playSoundAsync(SoundID id) {
  SoundID* p = new SoundID(id);
  xTaskCreatePinnedToCore(
    _soundTask,
    "soundPlay",
    4096,
    p,
    1,
    NULL,
    0    // Core 0 — away from ESP-SR on Core 1
  );
}

//EOF
