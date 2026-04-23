/*

*/


void addFileSystems() {

  if (SD.begin(cs))
  {
    if (!filemgr.AddFS(SD, "SD-Card", false)) {
      debugln(F("Adding SD failed."));
    }
  }
  else
  {
    debugln(F("SD File System not inited."));
  }
}

uint32_t checkFileFlags(fs::FS &fs, String filename, uint32_t flags) {
  // Checks if target file name is valid for action. This will simply allow everything by returning the queried flag
  if (flags & ESPFMfGK::flagIsValidAction) {
    return flags & (~ESPFMfGK::flagIsValidAction);
  }
  // Checks if target file name is valid for action.
  if (flags & ESPFMfGK::flagIsValidTargetFilename) {
    return flags & (~ESPFMfGK::flagIsValidTargetFilename);
  }
  // Default actions
  uint32_t defaultflags = ESPFMfGK::flagCanDelete | ESPFMfGK::flagCanRename | ESPFMfGK::flagCanGZip |  // ^t
                          ESPFMfGK::flagCanDownload | ESPFMfGK::flagCanUpload | ESPFMfGK::flagCanEdit | // ^t
                          ESPFMfGK::flagAllowPreview;
  return defaultflags;
}

void setupFilemanager() {
  // See above.
  filemgr.checkFileFlags = checkFileFlags;
  filemgr.WebPageTitle = "FileManager";
  filemgr.BackgroundColor = "black";
  filemgr.textareaCharset = "accept-charset=\"utf-8\"";

  if ((WiFi.status() == WL_CONNECTED) && (filemgr.begin())) {
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    debug(F("Open Filemanager with http://"));
    debug(WiFi.localIP());
    debug(F(":"));
    debug(filemanagerport);
    debug(F("/"));
    debugln();
  } else {
    debug(F("Filemanager: did not start"));
  }
}


void start_filebrowser()
{
  filemanager_active = true;
  addFileSystems();
  setupFilemanager();
}

//EOF
