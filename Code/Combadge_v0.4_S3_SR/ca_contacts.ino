/*  contacts.ino
 *
 *  Manages the badge address book stored on the SD card at /contacts.json
 *
 *  Each contact entry:
 *    {
 *      "name":    "Support",            - human-readable; used by "Call Support" SR command
 *      "badgeID": "0e74048",            - chip eFuse ID, acts as the "phone number"
 *      "ip":      "fd00::1:0e74"        - Husarnet IPv6, updated via MQTT on contact's boot
 *      "phoneme": "ScPeRT"              - (optional) override SR phoneme for this name
 *    }
 *
 *  SD file format - /contacts.json:
 *    [
 *      {"name":"Support","badgeID":"0e74048","ip":"fd00::1:0e74"},
 *      {"name":"Kirk",   "badgeID":"b7c2beef1234","ip":"fd00::1:b7c2"}
 *    ]
 *
 *  SR commands are rebuilt from this file at every boot by buildSrCommands()
 *  (ComBadge_v0_3_S3_SR.ino).  Add/remove contacts by editing this file on the
 *  SD card and rebooting — no firmware reflash needed.
 *
 *  Functions:
 *    loadContacts()               - reads /contacts.json from SD into memory
 *    saveContacts()               - writes current contact list back to SD
 *    getContactByName(name)       - returns index into contacts[], or -1
 *    getContactByBadgeID(id)      - returns index into contacts[], or -1
 *    updateContactIP(badgeID, ip) - updates stored IP for a known contact (called by MQTT)
 *    printContacts()              - debug dump of all contacts to serial
 *
 *  Requires: ArduinoJson library (v6) - add to Arduino IDE library manager
 *    https://arduinojson.org/
 */

#include <ArduinoJson.h>

#define MAX_CONTACTS      20
#define CONTACTS_FILE     "/contacts.json"
#define CONTACT_NAME_LEN   32
#define CONTACT_ID_LEN     16
#define CONTACT_IP_LEN     40   // enough for a full IPv6 address
#define CONTACT_PHONEME_LEN 48  // optional SR phoneme override

struct Contact {
  char name[CONTACT_NAME_LEN];
  char badgeID[CONTACT_ID_LEN];
  char ip[CONTACT_IP_LEN];
  char phoneme[CONTACT_PHONEME_LEN];  // empty = auto-generate in buildSrCommands()
};

Contact contacts[MAX_CONTACTS];
int     contactCount = 0;

// -----------------------------------------------------------------------

bool loadContacts() {
  if (!SD.exists(CONTACTS_FILE)) {
    debugln("Contacts: /contacts.json not found on SD.");
    return false;
  }

  File f = SD.open(CONTACTS_FILE, FILE_READ);
  if (!f) {
    debugln("Contacts: failed to open /contacts.json.");
    return false;
  }

  // ArduinoJson v6 - size the doc for MAX_CONTACTS entries
  // Each entry ~150 chars (extra for optional phoneme field), plus array overhead
  StaticJsonDocument<MAX_CONTACTS * 160> doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();

  if (err) {
    debug("Contacts: JSON parse error: ");
    debugln(err.c_str());
    return false;
  }

  contactCount = 0;
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject entry : arr) {
    if (contactCount >= MAX_CONTACTS) break;

    const char* name    = entry["name"]    | "";
    const char* badgeID = entry["badgeID"] | "";
    const char* ip      = entry["ip"]      | "";
    const char* phoneme = entry["phoneme"] | "";   // optional — empty if not set

    strncpy(contacts[contactCount].name,    name,    CONTACT_NAME_LEN    - 1);
    strncpy(contacts[contactCount].badgeID, badgeID, CONTACT_ID_LEN      - 1);
    strncpy(contacts[contactCount].ip,      ip,      CONTACT_IP_LEN      - 1);
    strncpy(contacts[contactCount].phoneme, phoneme, CONTACT_PHONEME_LEN - 1);

    contacts[contactCount].name[CONTACT_NAME_LEN       - 1] = '\0';
    contacts[contactCount].badgeID[CONTACT_ID_LEN      - 1] = '\0';
    contacts[contactCount].ip[CONTACT_IP_LEN           - 1] = '\0';
    contacts[contactCount].phoneme[CONTACT_PHONEME_LEN - 1] = '\0';

    contactCount++;
  }

  debug("Contacts: loaded ");
  debug(contactCount);
  debugln(" contacts from SD.");
  return true;
}

// -----------------------------------------------------------------------
// Update Contact IP Address upon change:
bool saveContacts() {
  StaticJsonDocument<MAX_CONTACTS * 160> doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < contactCount; i++) {
    JsonObject entry  = arr.createNestedObject();
    entry["name"]    = contacts[i].name;
    entry["badgeID"] = contacts[i].badgeID;
    entry["ip"]      = contacts[i].ip;
    // Only write phoneme if non-empty (keeps the JSON tidy)
    if (contacts[i].phoneme[0] != '\0') {
      entry["phoneme"] = contacts[i].phoneme;
    }
  }

  SD.remove(CONTACTS_FILE);
  File f = SD.open(CONTACTS_FILE, FILE_WRITE);
  if (!f) {
    debugln("Contacts: failed to open /contacts.json for writing.");
    return false;
  }

  serializeJson(doc, f);
  f.close();
  debugln("Contacts: saved to SD.");
  return true;
}

// -----------------------------------------------------------------------
// Returns index of contact matching name (case-insensitive), or -1
int getContactByName(const String& name) {
  for (int i = 0; i < contactCount; i++) {
    if (name.equalsIgnoreCase(contacts[i].name)) return i;
  }
  return -1;
}

// Returns index of contact matching badgeID, or -1
int getContactByBadgeID(const String& badgeID) {
  for (int i = 0; i < contactCount; i++) {
    if (badgeID.equalsIgnoreCase(contacts[i].badgeID)) return i;
  }
  return -1;
}

// -----------------------------------------------------------------------
// Called by mqtt_presence.ino when a contact publishes their new IP.
// Updates in-memory entry and flushes to SD so it survives reboot.
bool updateContactIP(const String& badgeID, const String& newIP) {
  int idx = getContactByBadgeID(badgeID);
  if (idx < 0) {
    debug("Contacts: unknown badgeID, ignoring IP update: ");
    debugln(badgeID);
    return false;
  }

  if (newIP.equals(contacts[idx].ip)) return true;  // no change

  newIP.toCharArray(contacts[idx].ip, CONTACT_IP_LEN);
  debug("Contacts: updated IP for ");
  debug(contacts[idx].name);
  debug(" → ");
  debugln(contacts[idx].ip);

  saveContacts();   // persist immediately so reboot doesn't lose it
  return true;
}

// -----------------------------------------------------------------------
// If debugging is turned on, this will print on boot:
void printContacts() {
  debugln("=== Contact List ===");
  for (int i = 0; i < contactCount; i++) {
    debug(i);
    debug(": ");
    debug(contacts[i].name);
    debug(" | ");
    debug(contacts[i].badgeID);
    debug(" | ");
    debug(contacts[i].ip);
    if (contacts[i].phoneme[0] != '\0') {
      debug(" | phoneme: ");
      debug(contacts[i].phoneme);
    }
    debugln("");
  }
  debugln("===================");
}

// ==================== SR phoneme encoder ====================
// Converts contact names to MultiNet5 phoneme strings.
//
// The ESP-SR MultiNet tool uses its own phoneme alphabet — these tokens were
// reverse-engineered from the official command list in the main .ino file.
// Words in the lookup table get exact phonemes; anything unknown falls back
// to Title Case (MultiNet handles plain English dictionary words fine).
//
// To add a contact whose name isn't recognised, set the "phoneme" field in
// contacts.json to override auto-generation entirely, e.g.:
//   {"name":"Badge2", "phoneme":"Baq To"}

// Syllable/word -> MultiNet5 phoneme lookup
// Covers digits, NATO alphabet, common first names, and common English words.
struct PhonemeEntry {
  const char* word;
  const char* phoneme;
};

static const PhonemeEntry PHONEME_TABLE[] = {
  // ----- Digits -----
  { "zero",     "ZgRb"     },
  { "one",      "WcN"      },
  { "two",      "To"       },
  { "three",    "vRm"      },
  { "four",     "FeR"      },
  { "five",     "FiV"      },
  { "six",      "SgKS"     },
  { "seven",    "SfVcN"    },
  { "eight",    "dT"       },
  { "nine",     "NiN"      },
  // ----- Common words (from official command set) -----
  { "call",     "KeL"      },
  { "badge",    "Baq"      },
  { "phone",    "FbN"      },
  { "begin",    "BgGgN"    },
  { "open",     "bPcN"     },
  { "config",   "KnNFgG"   },
  { "close",    "KLbS"     },
  { "files",    "FiLZ"     },
  { "update",   "cPDdT"    },
  { "contacts", "KnNTaKTS" },
  { "enable",   "fNdBcL"   },
  { "disable",  "DgSdBcL"  },
  { "sim",      "SgM"      },
  { "wifi",     "WiFi"     },
  { "recording","RcKeRDgl" },
  { "support",  "ScPeRT"   },
  { "control",  "KnNTRbL"  },
  { "tech",     "TcK"      },
  { "base",     "BdS"      },
  // ----- NATO phonetic alphabet -----
  { "alpha",    "aLFa"     },
  { "bravo",    "BRdVb"    },
  { "charlie",  "KaRLc"    },
  { "delta",    "DcLTa"    },
  { "echo",     "cKb"      },
  { "foxtrot",  "FnKSTRnT" },
  { "golf",     "GnLF"     },
  { "hotel",    "HbTcL"    },
  { "india",    "gNDca"    },
  { "juliet",   "JoLcdT"   },
  { "kilo",     "KcLb"     },
  { "lima",     "LcMa"     },
  { "mike",     "MiK"      },
  { "november", "NbVcMBeR" },
  { "oscar",    "nSKaR"    },
  { "papa",     "PaPa"     },
  { "quebec",   "KcBcK"    },
  { "romeo",    "RbMcb"    },
  { "sierra",   "ScRa"     },
  { "tango",    "TaGb"     },
  { "uniform",  "JoNgFeRM" },
  { "victor",   "VgKTeR"   },
  { "whiskey",  "WgSKc"    },
  { "xray",     "cKSRd"    },
  { "yankee",   "JaGKc"    },
  { "zulu",     "ZoLo"     },
  // ----- Common first names -----
  { "al",       "aL"       },
  { "dan",      "DaN"      },
  { "dave",     "DdV"      },
  { "jack",     "JaK"      },
  { "james",    "JdMZ"     },
  { "jane",     "JdN"      },
  { "john",     "JnN"      },
  { "kate",     "KdT"      },
  { "mark",     "MaRK"     },
  { "nick",     "NgK"      },
  { "sam",      "SaM"      },
  { "tom",      "TnM"      },
};
#define PHONEME_TABLE_SIZE  (sizeof(PHONEME_TABLE) / sizeof(PhonemeEntry))

// Look up a single lowercase word in the table. Returns nullptr if not found.
static const char* lookupPhoneme(const char* word) {
  for (int i = 0; i < (int)PHONEME_TABLE_SIZE; i++) {
    if (strcasecmp(PHONEME_TABLE[i].word, word) == 0) {
      return PHONEME_TABLE[i].phoneme;
    }
  }
  return nullptr;
}

// Convert a contact name (may be multi-word) to a MultiNet phoneme string.
// Each word is looked up independently; unknowns fall back to Title Case.
static String nameToPhoneme(const String& name) {
  String result = "";
  String remaining = name;
  remaining.trim();

  while (remaining.length() > 0) {
    // Split off next word
    int sp = remaining.indexOf(' ');
    String word = (sp < 0) ? remaining : remaining.substring(0, sp);
    remaining   = (sp < 0) ? "" : remaining.substring(sp + 1);
    remaining.trim();

    if (result.length() > 0) result += " ";

    // Look up in table (case-insensitive)
    char buf[CONTACT_NAME_LEN];
    word.toLowerCase();
    word.toCharArray(buf, sizeof(buf));
    const char* ph = lookupPhoneme(buf);

    if (ph) {
      result += ph;
    } else {
      // Unknown word — Title Case passthrough; MultiNet handles plain English
      String titled = word;
      titled.toLowerCase();
      if (titled.length() > 0) titled.setCharAt(0, toupper(titled.charAt(0)));
      result += titled;
      debug("SR phoneme: no entry for '");
      debug(word);
      debugln("' — using Title Case fallback");
    }
  }
  return result;
}


// ==================== buildSrCommands() ====================
// Called after loadContacts(). Constructs the full sr_cmd_t array:
//   [fixed commands] + [one "Call <name>" per contact]
//
// Lives here (ca_contacts.ino) so the Contact struct is fully defined.
// Globals sr_commands_dynamic, sr_commands_count, sr_fixed_commands, and
// SR_FIXED_COUNT are defined in the main .ino file which is compiled first.
void buildSrCommands() {
  // Free previous allocation if any (safe to call on first boot)
  if (sr_commands_dynamic) {
    delete[] sr_commands_dynamic;
    sr_commands_dynamic = nullptr;
    sr_commands_count   = 0;
  }

  int total = SR_FIXED_COUNT + contactCount;
  sr_commands_dynamic = new sr_cmd_t[total];
  if (!sr_commands_dynamic) {
    debugln("SR: FATAL - failed to allocate sr_commands_dynamic!");
    return;
  }

  // Copy the fixed commands verbatim into the start of the array
  memcpy(sr_commands_dynamic, sr_fixed_commands, SR_FIXED_COUNT * sizeof(sr_cmd_t));

  // Append one "Call <name>" entry per contact
  for (int i = 0; i < contactCount; i++) {
    int cmdId = SR_CMD_CONTACT_BASE + i;

    // Display label shown in serial debug: "Call Support"
    char displayName[64];
    snprintf(displayName, sizeof(displayName), "Call %s", contacts[i].name);

    // Phoneme string: use manual override from JSON if present, else auto-generate
    String phonemeName = (contacts[i].phoneme[0] != '\0')
                         ? String(contacts[i].phoneme)
                         : nameToPhoneme(String(contacts[i].name));
    String phoneme = "KeL " + phonemeName;

    strncpy(sr_commands_dynamic[cmdId].str,     displayName,      sizeof(sr_commands_dynamic[cmdId].str)     - 1);
    sr_commands_dynamic[cmdId].str[sizeof(sr_commands_dynamic[cmdId].str) - 1] = '\0';
    strncpy(sr_commands_dynamic[cmdId].phoneme, phoneme.c_str(),  sizeof(sr_commands_dynamic[cmdId].phoneme) - 1);
    sr_commands_dynamic[cmdId].phoneme[sizeof(sr_commands_dynamic[cmdId].phoneme) - 1] = '\0';

    debug("SR: registered [");
    debug(cmdId);
    debug("] ");
    debug(displayName);
    debug(" -> ");
    debugln(phoneme);
  }

  sr_commands_count = total;
  debug("SR: total commands: ");
  debugln(sr_commands_count);
}

//EOF
