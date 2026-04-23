/*  listen_server.ino
 *
 *  Replaces the original placeholder which referenced LED pin 5 (not present
 *  on this hardware) and had no useful responses.
 *
 *  Currently serves:
 *    GET /          → JSON with badge identity and status info
 *    GET /ping      → plain-text "pong" for connectivity testing
 *
 *  This will be extended for call signalling:
 *    POST /call     → incoming call request from peer badge
 *    GET  /stream   → audio stream endpoint (future: audio_stream.ino)
 */

// Build the badge info JSON once so it can be reused without re-allocating
String getBadgeInfoJSON()
{
  String json = "{";
  json += "\"badgeID\":\"" + BadgeID + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"firmware\":\"" + FirmwareVer + "\",";
  json += "\"rssi\":"  + String(WiFi.RSSI()) + ",";
  json += "\"status\":\"idle\"";
  json += "}";
  return json;
}

void handle_root(NetworkClient& client)
{
  String body = getBadgeInfoJSON();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.print(body);
}

void handle_ping(NetworkClient& client)
{
  String body = "pong";
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(body.length());
  client.println();
  client.print(body);
}

void handle_not_found(NetworkClient& client)
{
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("Not found");
}

void client_connected()
{
  server.begin();
  NetworkClient client = server.accept();

  if (!client) return;

  debugln("HTTP: new client");

  String requestLine = "";
  String currentLine = "";
  bool   firstLine   = true;

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 2000)
  {
    if (client.available())
    {
      char c = client.read();
      if (c == '\n')
      {
        if (firstLine) {
          requestLine = currentLine;
          firstLine   = false;
          debug("Request: ");
          debugln(requestLine);
        }
        if (currentLine.length() == 0)
        {
          // End of headers — route the request
          if      (requestLine.startsWith("GET / "))     handle_root(client);
          else if (requestLine.startsWith("GET /ping"))  handle_ping(client);
          else                                           handle_not_found(client);
          break;
        }
        currentLine = "";
      }
      else if (c != '\r')
      {
        currentLine += c;
      }
    }
  }

  client.stop();
  debugln("HTTP: client disconnected");
}


//EOF
