/*  husarnet_key.ino

    Manages the Husarnet join code securely.

    The join code is stored on the SD card as an AES-128-CBC encrypted
    binary file at /husarnet.key. The AES decryption key lives in NVS
    (Preferences), never on the SD card.

    This separation means:
      - Swapping the SD card changes which Husarnet network the badge joins
      - The decryption key survives SD swaps, reformats, and file browser use
      - Monthly join code renewal = new /husarnet.key pushed via OTA
      - White-label customers get their own AES key + join code file
      - The join code is never visible as plain text anywhere on the device

    NVS namespace: "hnet" - key: "aeskey" (16 bytes, stored as base64 string)

    SD file: /husarnet.key
      Format: [16 bytes IV][N bytes ciphertext]
      Plaintext: the Husarnet join code string, null terminated, padded to
      16-byte boundary with PKCS7 padding.

    AES implementation: mbedTLS (built into ESP32 Arduino core, no extra lib)

    Functions:
      husarnetKeySetup()          - called once in setup(), loads key from NVS
      loadJoinCode(out)           - decrypts /husarnet.key, writes to out String
      provisionAESKey(keyBase64)  - writes a new AES key into NVS (provisioning)
      isKeyProvisioned()          - returns true if NVS contains a valid key
      updateKeyFile(url)          - downloads new encrypted key file from URL to SD
*/

#include "mbedtls/aes.h"
#include "mbedtls/base64.h"

#define HNET_NVS_NAMESPACE  "hnet"
#define HNET_NVS_KEY        "aeskey"
#define HNET_KEY_FILE       "/husarnet.key"
#define HNET_AES_KEYLEN     16    // AES-128 = 16 bytes
#define HNET_IV_LEN         16    // CBC IV = 16 bytes
#define HNET_MAX_CODE_LEN   128   // max join code string length

// ==================== Internal State ====================
static uint8_t  aesKey[HNET_AES_KEYLEN];
static bool     aesKeyLoaded = false;

// ==================== NVS Key Management ====================

// Writes a new 16-byte AES key (supplied as base64 string) into NVS.
// Called once at provisioning time - either via serial provisioner tool
// or via a secure backend command.
bool provisionAESKey(const String& keyBase64) {
  // Decode base64 -> raw bytes
  uint8_t decoded[HNET_AES_KEYLEN + 4];
  size_t  decodedLen = 0;

  int ret = mbedtls_base64_decode(
              decoded, sizeof(decoded), &decodedLen,
              (const unsigned char*)keyBase64.c_str(), keyBase64.length()
            );

  if (ret != 0 || decodedLen != HNET_AES_KEYLEN) {
    debugln("HusarnetKey: invalid base64 AES key - must decode to exactly 16 bytes.");
    return false;
  }

  Preferences prefs;
  prefs.begin(HNET_NVS_NAMESPACE, false);
  size_t written = prefs.putBytes(HNET_NVS_KEY, decoded, HNET_AES_KEYLEN);
  prefs.end();

  if (written != HNET_AES_KEYLEN) {
    debugln("HusarnetKey: failed to write AES key to NVS.");
    return false;
  }

  debugln("HusarnetKey: AES key provisioned to NVS.");
  return true;
}

bool isKeyProvisioned() {
  Preferences prefs;
  prefs.begin(HNET_NVS_NAMESPACE, true);   // read-only
  size_t keyLen = prefs.getBytesLength(HNET_NVS_KEY);
  prefs.end();
  return (keyLen == HNET_AES_KEYLEN);
}

// Loads AES key from NVS into aesKey[] buffer.
// Returns false if key not provisioned.
bool husarnetKeySetup() {
  if (!isKeyProvisioned()) {
    debugln("HusarnetKey: no AES key in NVS. Badge needs provisioning.");
    debugln("HusarnetKey: send 'K<base64key>' via serial to provision.");
    aesKeyLoaded = false;
    return false;
  }

  Preferences prefs;
  prefs.begin(HNET_NVS_NAMESPACE, true);
  size_t read = prefs.getBytes(HNET_NVS_KEY, aesKey, HNET_AES_KEYLEN);
  prefs.end();

  if (read != HNET_AES_KEYLEN) {
    debugln("HusarnetKey: NVS key read failed.");
    aesKeyLoaded = false;
    return false;
  }

  aesKeyLoaded = true;
  debugln("HusarnetKey: AES key loaded from NVS.");
  return true;
}

// ==================== Decrypt join code from SD ====================

// Reads /husarnet.key from SD, decrypts it with the NVS key,
// and writes the plaintext join code into joinCode String.
// Returns false on any error.
bool loadJoinCode(String& joinCode) {
  if (!aesKeyLoaded) {
    debugln("HusarnetKey: AES key not loaded - cannot decrypt.");
    return false;
  }

  if (!SD.exists(HNET_KEY_FILE)) {
    debugln("HusarnetKey: /husarnet.key not found on SD.");
    return false;
  }

  File f = SD.open(HNET_KEY_FILE, FILE_READ);
  if (!f) {
    debugln("HusarnetKey: failed to open /husarnet.key.");
    return false;
  }

  size_t fileSize = f.size();
  // File must be: 16 (IV) + at least 16 (one AES block) bytes
  if (fileSize < HNET_IV_LEN + HNET_AES_KEYLEN || fileSize > HNET_IV_LEN + HNET_MAX_CODE_LEN + 16) {
    debugln("HusarnetKey: /husarnet.key has unexpected size.");
    f.close();
    return false;
  }

  // Read IV (first 16 bytes)
  uint8_t iv[HNET_IV_LEN];
  if (f.read(iv, HNET_IV_LEN) != HNET_IV_LEN) {
    debugln("HusarnetKey: failed to read IV from key file.");
    f.close();
    return false;
  }

  // Read ciphertext
  size_t cipherLen = fileSize - HNET_IV_LEN;
  uint8_t* cipher = (uint8_t*)malloc(cipherLen);
  uint8_t* plain  = (uint8_t*)malloc(cipherLen);

  if (!cipher || !plain) {
    debugln("HusarnetKey: malloc failed.");
    free(cipher); free(plain);
    f.close();
    return false;
  }

  if (f.read(cipher, cipherLen) != cipherLen) {
    debugln("HusarnetKey: failed to read ciphertext from key file.");
    free(cipher); free(plain);
    f.close();
    return false;
  }
  f.close();

  // Decrypt with AES-128-CBC
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, aesKey, HNET_AES_KEYLEN * 8);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, cipherLen, iv, cipher, plain);
  mbedtls_aes_free(&aes);

  // Remove PKCS7 padding
  uint8_t padLen = plain[cipherLen - 1];
  if (padLen == 0 || padLen > 16) {
    debugln("HusarnetKey: invalid PKCS7 padding - wrong key or corrupted file.");
    free(cipher); free(plain);
    return false;
  }

  size_t plainLen = cipherLen - padLen;
  plain[plainLen] = '\0';   // null terminate

  joinCode = String((char*)plain);
  free(cipher);
  free(plain);

  debugln("HusarnetKey: join code decrypted OK.");
  return true;
}

// ==================== OTA key file update ====================

// Downloads a new encrypted /husarnet.key from the given URL and saves to SD.
// Called by firmware_update.ino when a new key file is available.
// The AES key in NVS is NOT changed here - only the encrypted join code file.
bool updateKeyFile(const String& url) {
  if (WiFi.status() != WL_CONNECTED) {
    debugln("HusarnetKey: WiFi not connected, cannot update key file.");
    return false;
  }

  debugln("HusarnetKey: downloading new key file...");

  WiFiClientSecure client;
  client.setCACert(rootCACertificate);
  HTTPClient https;

  if (!https.begin(client, url)) {
    debugln("HusarnetKey: HTTPS begin failed.");
    return false;
  }

  int httpCode = https.GET();
  if (httpCode != HTTP_CODE_OK) {
    debug("HusarnetKey: download failed, HTTP code: ");
    debugln(httpCode);
    https.end();
    return false;
  }

  // Write directly to SD
  SD.remove(HNET_KEY_FILE);
  File f = SD.open(HNET_KEY_FILE, FILE_WRITE);
  if (!f) {
    debugln("HusarnetKey: failed to open key file for writing.");
    https.end();
    return false;
  }

  WiFiClient* stream = https.getStreamPtr();
  uint8_t buf[128];
  int     total = 0;
  int     contentLen = https.getSize();

  while (https.connected() && (contentLen > 0 || contentLen == -1)) {
    size_t avail = stream->available();
    if (avail) {
      size_t toRead = min(avail, sizeof(buf));
      size_t read   = stream->readBytes(buf, toRead);
      f.write(buf, read);
      total += read;
      if (contentLen > 0) contentLen -= read;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  f.close();
  https.end();

  debug("HusarnetKey: key file saved to SD, ");
  debug(total);
  debugln(" bytes.");
  return true;
}


//EOF
