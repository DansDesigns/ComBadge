


void createWebServer()
{
  {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      //~~~~~~~~~~~~~~~~~~~~ LCARS ui ~~~~~~~~~~~~~~~~~~~~
      // ~~~~~~~~~~~~~~~~~~~~ !WARNING, DO NOT USE DUE TO POTENTIAL LEGAL PROBLEMS! ~~~~~~~~~~~~~~~~~~~~
      /*
        content = "<!doctype html><html><head><metacharset='UTF-8'><title>SciScanner_Setup</title><style type='text/css'>";

        content += "html,body {font-family: 'arial', monospace; color: #fff; background: black;},p {},h1 {font-size: 270%;font-weight: bold;},h2";

        content += "{font-size:40%;font-weight: bold;},h3 {margin-top: 1em;}pre {display: inline;white-space: pre-line;word-wrap: break-word;}";

        content += ".lcars-tan-bg.lcars-tan-bg {background-color: #fc6;fill: #fc6;}.lcars-elbow {position: relative; width:9.5rem; min-width: 9.5rem; height: 4.5rem; min-height: 4.5rem; background: #fc6; margin: 0;}";

        content += ".lcars-elbow a {display: inline-block; position: absolute; color: #000; bottom: 0; padding: 0.25rem;} .lcars-elbow:after {content: ''; display: block; position: absolute; width: 2rem; height: 3rem; background: #000;}";

        content += ".lcars-elbow.left-bottom { border-top-left-radius: 3.75rem;}.lcars-elbow.left-bottom a { right: 2.25rem;}.lcars-elbow.left-bottom:after { right: 0; top: 1.5rem; border-top-left-radius: 1.875rem;}";

        content += ".lcars-elbow.left-top {border-bottom-left-radius: 3.75rem;}.lcars-elbow.left-top a { top: 0; right: 2.25rem;}.lcars-elbow.left-top:after { right: 0; bottom: 1.5rem; border-bottom-left-radius: 3.75rem;}";

        content += ".lcars-elbow.right-bottom { border-top-right-radius: 3.75rem;}.lcars-elbow.right-bottom:after { top: 1.5rem; border-top-right-radius: 1.875rem;}.lcars-elbow.right-bottom a { left: 2.25rem;}";

        content += ".lcars-elbow.right-top { border-bottom-right-radius: 3.75rem;}.lcars-elbow.right-top a { display: inline-block; top: 0; left: 2.25rem;}.lcars-elbow.right-top:after {  bottom: 1.5rem; border-bottom-right-radius: 1.875rem;}";

        content += ".lcars-bar { position: relative; color: #000; height: 100%; width: 100%; margin: 0; display: inline-block; background-color: #fc6;}.lcars-bar:after { content: '';  display: block; position: absolute; background: #000;}";

        content += ".lcars-bar.spacer { margin: 0; padding: 0;  background-color: transparent; width: 0.25rem; min-width: 0.25rem; height: 0.25rem; min-height: 0.25rem;} .lcars-bar.double-spacer { background-color: transparent; width: 0.5rem; min-width: 0.5rem; height: 0.5rem; min-height: 0.5rem;}.lcars-bar.left-space { margin-left: 0.25rem;}.lcars-bar.left-double-space { margin-left: 0.5rem;}";

        content += ".lcars-bar.right-space { margin-right: 0.25rem;}.lcars-bar.right-double-space { margin-right: 0.5rem;}.lcars-bar.horizontal { height: 1.5rem;}.lcars-bar.left-end { width: 1.5rem; max-width: 1.5rem; min-width: 1.5rem; border-top-left-radius: 0.75rem; border-bottom-left-radius: 0.75rem;}";

        content += ".lcars-bar.left-end.decorated:after { right: 0.25rem; width: 0.25rem; height: 100%; background-color: #000;}.lcars-bar.right-end { width: 1.5rem; max-width: 1.5rem; min-width: 1.5rem; border-top-right-radius: 0.75rem;";

        content += " border-bottom-right-radius: 0.75rem;}.lcars-bar.right-end.decorated:after { left: 0.25rem; width: 0.25rem; height: 100%; background-color: #000; background-color: #000;}.lcars-bar.fill { flex: 1;}";

        content += ".lcars-bar.bottom { align-self: flex-end;}.lcars-bar .lcars-title { color: #fff; background-color: #000; text-transform: uppercase; margin: 0; margin-left: 0.833333333333335rem; padding-left: 0.166666666666667rem; padding-right: 0.166666666666667rem; padding-bottom: 0.166666666666667rem; display: inline-block; font-size: 150%;  height: 100%;}";

        content += ".lcars-bar .lcars-title.right { float: right; margin-right: 0.833333333333335rem;}.lcars-app-container {  display: flex; width: calc(100% - 1rem); height: calc(100% - 1rem); overflow: hidden; margin: 0.5rem;}";

        content += ".lcars-app-container #left-menu { position: fixed;  padding-top: 4.5rem; height: 100%; left: 0.5rem;}.lcars-app-container #header { position: fixed; background-color: #000;  width: calc(100% - 1rem); top: 0; left: 0.5rem; padding-top: 0.5rem; margin-bottom: 0.25rem; z-index: 1;}";

        content += ".lcars-app-container #footer { position: fixed;  background-color: #000; bottom: 0; left: 0.5rem; padding-bottom: 0.5rem; width: calc(100% - 1rem); z-index: 1;}";

        content += ".lcars-app-container #container { margin-top: 0.25rem; margin-left: 0.25rem; padding-left: 9.5rem; padding-top: 4.5rem; padding-bottom: 4.5rem; width: 100%; height: 100%; overflow: auto;}.lcars-row { display: inline-flex;  flex-direction: row; width: 100%;}";

        content += ".lcars-column { display: inline-flex; flex-direction: column;}.lcars-u-1 { width: 7.5rem;}.lcars-column.start-space { margin-top: 0.25rem;}.lcars-u-10 {width: 77.25rem;}.lcars-vu-15 { height: 48.5rem;.lcars-u-16-8 {width: 123.75rem;min-width: 123.75rem;height:25.75rem;min-height: 25.75rem;}}";

        content += "a:link {color: #fc6;font-weight: 400;}a:visited {color: #fc6;font-weight: normal;}a:focus {color: #fc6;background-color: #080602;font-weight: 400;}a:hover {color: #fc6;background-color: #090703;font-weight: 400;}a:active {color: #ff9;font-weight: 500;background-color: #0c0806;}.lcars-u-5 {width: 38.5rem;}.lcars-element.button {cursor: pointer;}.lcars-element.button.button:active {background-color: #c96;}";

        content += "</style></head><body><div class='lcars-app-container'>";

        content += "<!-- ~~~~~~~~~~ HEADER~~~~~~~~~~  --><div id='header' class='lcars-row header'><!-- ELBOW --><div class='lcars-elbow left-bottom lcars-tan-bg'></div><!-- BAR WITH TITLE --><div class='lcars-bar horizontal'><div class='lcars-title right'>Wifi Scanner</div></div><!-- ROUNDED EDGE DECORATED --><div class='lcars-bar horizontal right-end decorated'></div></div>";

        content += "<!-- ~~~~~~~~~~ SIDE MENU~~~~~~~~~~  --><div id='left-menu' class='lcars-column start-space lcars-u-1'><!-- FILLER --><div class='lcars-bar lcars-u-1'>";

        //content += "<!-- SIDE text or elements here-->\n<form action=\'/scan\' method=\'POST\'><input style='background-color:#36f;font-size:25;' type=\"submit\" value=\"SCAN\"></form>";

        content += "</div>";      // close the SIDE MENU section

        content += "<!-- ~~~~~~~~~~ FOOTER ~~~~~~~~~~ --><div id='footer' class='lcars-row '><!-- ELBOW --><divclass='lcars-elbow left-top lcars-tan-bg'></div><!-- BAR --><div class='lcars-bar horizontal both-divider bottom'></div><!-- ROUNDED EDGE -->  <div class='lcars-bar horizontal right-end left-divider bottom'></div></div>";

        content += "\t\t<!-- ~~~~~~~~~~ MAIN CONTAINER ~~~~~~~~~~~ -->\n\t\t<div id='container'>\n\t\t\t<!-- COLUMN LAYOUT -->\n\t\t\t<div class='lcars-column lcars-u-16-8'>";

        content += "      \t\t\t<h1>Available Networks</h1>\n      \t\t<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();} </script>\n";

        content += st;

        content += "      \t\t\t<h2>Manual Entry:</h2><form method='get' action='setting'><input style='font-size:18; color:#c96;' id='s' name='ssid' length=32 placeholder='SSID'><input style='font-size:18; color:#c96;' id='p' name='pass' length=64 placeholder='PASSWORD'><input style='background-color:##36f;color:#000;' type='submit'></form></h2>\n";

        content += "</div></div></div></body></html>";
      */


      //~~~~~~~~~~~~~~~~~~~~ OG non-LCARS Working Code: ~~~~~~~~~~~~~~~~~~~~
      
      content = "<!DOCTYPE HTML>\r\n<html>";
      //server.send(200, "text/html", index_html);
      content += "<h1>Scicorder Wifi Connector</h1>";
      content += "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();} </script>";

      content += "<p>Setup_AP IP: ";
      content += ipStr;      // ip address of AP
      content += "</p>";       // start of text list
      content += st;        // list ssid and rssi
      content += "<p>Manual Entry:</p><form method='get' action='setting'><input id='s' name='ssid' length=32 placeholder='SSID'><input id='p' name='pass' length=64 placeholder='PASSWORD'><input type='submit'></form>";
      content += "<p>";
      content += "</p>";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";    // scan button
      //content += "</html>";
      content += "</div></div></div></body></html>";
      //server.send(200, "text/html", index_html);
      server.send(200, "text/html", content);
    });
    
    server.on("/scan", []() {     // 2nd page example with refresh function
      //setupAP();
      //IPAddress ip = WiFi.softAPIP();
      //String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<h2>Scanning Wireless Networks..</h2>\n";
      content += "<p><a href='' onclick='history.back();return false;'>Press here to Refresh</a></p>";
      //content = "<!DOCTYPE HTML>/r/n<html>reload()</html>";
      //content = "reload()";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}
