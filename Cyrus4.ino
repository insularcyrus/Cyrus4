#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#include <DHT.h>
#include <time.h>


// =====================================================
// DHT11
// =====================================================

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(
    DHTPIN,
    DHTTYPE
);


// =====================================================
// FIREBASE
// =====================================================

#define API_KEY \
    "AIzaSyAy89R7qsCggt1gHQdeh7ds35QieXaui6o"

#define DATABASE_URL \
    "https://cyrus-ebb57-default-rtdb.europe-west1.firebasedatabase.app"

#define USER_EMAIL \
    "insularcyrus7@gmail.com"

#define USER_PASSWORD \
    "insularcyruss7-123"


// =====================================================
// FIREBASE OBJECTS
// =====================================================

UserAuth user_auth(
    API_KEY,
    USER_EMAIL,
    USER_PASSWORD,
    3000
);

FirebaseApp app;

WiFiClientSecure ssl_client;

AsyncClientClass aClient(
    ssl_client
);

RealtimeDatabase Database;


// =====================================================
// WEB SERVER
// =====================================================

AsyncWebServer server(80);


// =====================================================
// WIFI MANAGER
// =====================================================

const char *AP_SSID =
    "ESP-WIFI-MANAGER";

bool wifiManagerMode =
    false;


// =====================================================
// SENSOR
// =====================================================

unsigned long lastSensorRead =
    0;

const unsigned long SENSOR_INTERVAL =
    10000;


// =====================================================
// FILE DECLARATIONS
// =====================================================

void processFirebase(
    AsyncResult &aResult
);

bool connectToSavedWiFi();

void startWiFiManager();

void startMainWebServer();

void setupFirebase();

void sendSensorData();

String readFile(
    const char *path
);

bool writeFile(
    const char *path,
    const String &data
);

void deleteWiFiFiles();

String getDateString();

String getTimeString();


// =====================================================
// WIFI MANAGER HTML
// =====================================================

const char WIFI_MANAGER_HTML[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html lang="en">

<head>

<meta charset="UTF-8">

<meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
>

<title>CYRUS | WiFi Manager</title>

<style>

* {

    box-sizing: border-box;

    margin: 0;

    padding: 0;

    font-family:
        Arial,
        Helvetica,
        sans-serif;

}


body {

    min-height: 100vh;

    display: flex;

    justify-content: center;

    align-items: center;

    padding: 20px;

    background:
        radial-gradient(
            circle at top,
            #172554 0%,
            #080d18 45%,
            #03060b 100%
        );

    color: #e0f2fe;

}


.wifi-card {

    width: 100%;

    max-width: 500px;

    padding: 35px;

    background:
        linear-gradient(
            145deg,
            rgba(15, 23, 42, .98),
            rgba(3, 7, 18, .98)
        );

    border:
        1px solid
        rgba(56, 189, 248, .35);

    border-radius: 22px;

    box-shadow:
        0 20px 60px
        rgba(0, 0, 0, .55),

        0 0 30px
        rgba(56, 189, 248, .08);

}


.logo {

    text-align: center;

    font-size: 34px;

    font-weight: 800;

    letter-spacing: 5px;

    color: #38bdf8;

    margin-bottom: 8px;

}


.subtitle {

    text-align: center;

    color: #94a3b8;

    font-size: 14px;

    margin-bottom: 30px;

}


.wifi-icon {

    width: 70px;

    height: 70px;

    margin:
        0 auto 20px;

    display: flex;

    justify-content: center;

    align-items: center;

    border-radius: 50%;

    background:
        rgba(
            56,
            189,
            248,
            .08
        );

    border:
        1px solid
        rgba(
            56,
            189,
            248,
            .3
        );

    font-size: 32px;

    box-shadow:
        0 0 25px
        rgba(
            56,
            189,
            248,
            .15
        );

}


.form-group {

    margin-bottom: 18px;

}


.form-group label {

    display: block;

    margin-bottom: 8px;

    font-size: 13px;

    font-weight: 700;

    color: #7dd3fc;

}


.form-group input {

    width: 100%;

    padding:
        13px 14px;

    border-radius: 10px;

    border:
        1px solid #334155;

    background: #020617;

    color: #e0f2fe;

    outline: none;

    font-size: 14px;

}


.form-group input::placeholder {

    color: #64748b;

}


.form-group input:focus {

    border-color:
        #38bdf8;

    box-shadow:
        0 0 0 3px
        rgba(
            56,
            189,
            248,
            .10
        );

}


.wifi-button {

    width: 100%;

    margin-top: 8px;

    padding: 14px;

    border: none;

    border-radius: 10px;

    background:
        linear-gradient(
            135deg,
            #0284c7,
            #38bdf8
        );

    color: white;

    font-size: 15px;

    font-weight: 700;

    cursor: pointer;

}


.info {

    margin-top: 25px;

    padding: 16px;

    border-radius: 12px;

    background:
        rgba(
            14,
            165,
            233,
            .06
        );

    border:
        1px solid
        rgba(
            56,
            189,
            248,
            .15
        );

    color: #94a3b8;

    font-size: 13px;

    line-height: 1.7;

}


.info strong {

    color: #7dd3fc;

}


.status {

    margin-top: 20px;

    text-align: center;

    font-size: 12px;

    color: #64748b;

}


.status span {

    color: #22c55e;

    font-weight: 700;

}

</style>

</head>


<body>


<div class="wifi-card">


    <div class="wifi-icon">
        📶
    </div>


    <div class="logo">
        CYRUS
    </div>


    <div class="subtitle">
        ESP32 WiFi Manager
    </div>


    <form
        action="/"
        method="POST"
    >


        <div class="form-group">

            <label for="ssid">
                WiFi SSID
            </label>

            <input
                type="text"
                id="ssid"
                name="ssid"
                placeholder="Enter WiFi name"
                required
            >

        </div>


        <div class="form-group">

            <label for="pass">
                WiFi Password
            </label>

            <input
                type="password"
                id="pass"
                name="pass"
                placeholder="Enter WiFi password"
                required
            >

        </div>


        <div class="form-group">

            <label for="ip">
                IP Address
            </label>

            <input
                type="text"
                id="ip"
                name="ip"
                placeholder="Optional - leave blank for DHCP"
            >

        </div>


        <div class="form-group">

            <label for="gateway">
                Gateway Address
            </label>

            <input
                type="text"
                id="gateway"
                name="gateway"
                placeholder="Optional - leave blank for DHCP"
            >

        </div>


        <button
            type="submit"
            class="wifi-button"
        >
            Save & Connect
        </button>


    </form>


    <div class="info">

        <strong>
            How it works
        </strong>

        <br>

        1. Enter your WiFi SSID and password.

        <br>

        2. Click <b>Save & Connect</b>.

        <br>

        3. ESP32 saves the WiFi settings.

        <br>

        4. ESP32 restarts automatically.

        <br>

        5. ESP32 connects to your WiFi.

        <br>

        6. Open the ESP32 IP address to access
        the Cyrus main web server.

    </div>


    <div class="status">

        WiFi Manager Status:
        <span>READY</span>

    </div>


</div>


</body>

</html>

)rawliteral";


// =====================================================
// READ FILE
// =====================================================

String readFile(
    const char *path
)
{
    if (
        !LittleFS.exists(path)
    )
    {
        return "";
    }


    File file =
        LittleFS.open(
            path,
            "r"
        );


    if (!file)
    {
        return "";
    }


    String data =
        file.readString();


    file.close();


    data.trim();


    return data;
}


// =====================================================
// WRITE FILE
// =====================================================

bool writeFile(
    const char *path,
    const String &data
)
{
    File file =
        LittleFS.open(
            path,
            "w"
        );


    if (!file)
    {
        Serial.print(
            "Failed to open file: "
        );

        Serial.println(path);

        return false;
    }


    file.print(data);

    file.close();


    return true;
}


// =====================================================
// DELETE WIFI FILES
// =====================================================

void deleteWiFiFiles()
{
    LittleFS.remove(
        "/ssid.txt"
    );

    LittleFS.remove(
        "/pass.txt"
    );

    LittleFS.remove(
        "/ip.txt"
    );

    LittleFS.remove(
        "/gateway.txt"
    );


    Serial.println(
        "WiFi settings deleted."
    );
}


// =====================================================
// CONNECT SAVED WIFI
// =====================================================

bool connectToSavedWiFi()
{
    String ssid =
        readFile(
            "/ssid.txt"
        );

    String pass =
        readFile(
            "/pass.txt"
        );

    String ip =
        readFile(
            "/ip.txt"
        );

    String gateway =
        readFile(
            "/gateway.txt"
        );


    if (
        ssid.length() == 0
    )
    {
        Serial.println();

        Serial.println(
            "NO CONNECTED WIFI"
        );

        Serial.println(
            "No saved WiFi credentials."
        );

        return false;
    }


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       SAVED WIFI FOUND"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "SSID: "
    );

    Serial.println(
        ssid
    );


    WiFi.mode(
        WIFI_STA
    );


    delay(500);


    // -------------------------------------------------
    // OPTIONAL STATIC IP
    // -------------------------------------------------

    if (
        ip.length() > 0 &&
        gateway.length() > 0
    )
    {
        IPAddress local_IP;

        IPAddress gateway_IP;


        if (
            local_IP.fromString(
                ip
            ) &&
            gateway_IP.fromString(
                gateway
            )
        )
        {
            IPAddress subnet(
                255,
                255,
                255,
                0
            );


            if (
                WiFi.config(
                    local_IP,
                    gateway_IP,
                    subnet
                )
            )
            {
                Serial.println(
                    "Static IP configured."
                );
            }
            else
            {
                Serial.println(
                    "Static IP configuration failed."
                );
            }
        }
    }


    WiFi.begin(
        ssid.c_str(),
        pass.c_str()
    );


    Serial.print(
        "Connecting to WiFi"
    );


    unsigned long startTime =
        millis();


    while (
        WiFi.status() != WL_CONNECTED &&
        millis() - startTime < 20000
    )
    {
        Serial.print(
            "."
        );

        delay(500);
    }


    Serial.println();


    if (
        WiFi.status() ==
        WL_CONNECTED
    )
    {
        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "       WIFI CONNECTED"
        );

        Serial.println(
            "================================="
        );


        Serial.print(
            "SSID: "
        );

        Serial.println(
            WiFi.SSID()
        );


        Serial.print(
            "IP Address: "
        );

        Serial.println(
            WiFi.localIP()
        );


        Serial.print(
            "Gateway: "
        );

        Serial.println(
            WiFi.gatewayIP()
        );


        Serial.println();

        return true;
    }


    Serial.println();

    Serial.println(
        "NO CONNECTED WIFI"
    );

    Serial.println(
        "Failed to connect to saved WiFi."
    );


    WiFi.disconnect(
        true
    );


    delay(1000);


    return false;
}


// =====================================================
// START WIFI MANAGER
// =====================================================

void startWiFiManager()
{
    wifiManagerMode =
        true;


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       WIFI MANAGER MODE"
    );

    Serial.println(
        "================================="
    );


    WiFi.mode(
        WIFI_AP
    );


    delay(500);


    bool apStarted =
        WiFi.softAP(
            AP_SSID
        );


    if (!apStarted)
    {
        Serial.println(
            "ERROR: Failed to start WiFi Manager AP!"
        );
    }


    delay(1000);


    Serial.print(
        "AP SSID: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "AP IP Address: "
    );

    Serial.println(
        WiFi.softAPIP()
    );


    // -------------------------------------------------
    // WIFI MANAGER PAGE
    // -------------------------------------------------

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",
                WIFI_MANAGER_HTML
            );
        }
    );


    // -------------------------------------------------
    // SAVE WIFI
    // -------------------------------------------------

    server.on(
        "/",
        HTTP_POST,
        [](AsyncWebServerRequest *request)
        {
            String ssid =
                "";

            String pass =
                "";

            String ip =
                "";

            String gateway =
                "";


            if (
                request->hasParam(
                    "ssid",
                    true
                )
            )
            {
                ssid =
                    request
                    ->getParam(
                        "ssid",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "pass",
                    true
                )
            )
            {
                pass =
                    request
                    ->getParam(
                        "pass",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "ip",
                    true
                )
            )
            {
                ip =
                    request
                    ->getParam(
                        "ip",
                        true
                    )
                    ->value();
            }


            if (
                request->hasParam(
                    "gateway",
                    true
                )
            )
            {
                gateway =
                    request
                    ->getParam(
                        "gateway",
                        true
                    )
                    ->value();
            }


            ssid.trim();

            pass.trim();

            ip.trim();

            gateway.trim();


            if (
                ssid.length() == 0 ||
                pass.length() == 0
            )
            {
                request->send(
                    400,
                    "text/plain",
                    "SSID and password are required."
                );

                return;
            }


            writeFile(
                "/ssid.txt",
                ssid
            );


            writeFile(
                "/pass.txt",
                pass
            );


            writeFile(
                "/ip.txt",
                ip
            );


            writeFile(
                "/gateway.txt",
                gateway
            );


            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#03060b;"
                "color:#e0f2fe;'>"

                "<div style='background:#0d1524;"
                "padding:30px;"
                "border-radius:20px;"
                "max-width:500px;"
                "margin:auto;"
                "border:1px solid #38bdf8;'>"

                "<h1 style='color:#38bdf8;'>"
                "WiFi Saved!"
                "</h1>"

                "<p style='color:#94a3b8;'>"
                "The ESP32 will restart and "
                "connect to the saved WiFi."
                "</p>"

                "<p style='color:#64748b;'>"
                "Please wait..."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );


            delay(1500);


            ESP.restart();
        }
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        " WIFI MANAGER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Connect to WiFi: "
    );

    Serial.println(
        AP_SSID
    );


    Serial.print(
        "Then open: http://"
    );

    Serial.println(
        WiFi.softAPIP()
    );


    Serial.println();
}


// =====================================================
// MAIN WEB SERVER
// =====================================================

void startMainWebServer()
{
    wifiManagerMode =
        false;


    // -------------------------------------------------
    // MAIN WEBSITE
    // -------------------------------------------------

    server.on(
        "/",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            if (
                LittleFS.exists(
                    "/index.html"
                )
            )
            {
                request->send(
                    LittleFS,
                    "/index.html",
                    "text/html"
                );
            }
            else
            {
                request->send(
                    404,
                    "text/plain",
                    "index.html not found."
                );
            }
        }
    );


    // -------------------------------------------------
    // CHANGE WIFI
    // -------------------------------------------------

    server.on(
        "/change-wifi",
        HTTP_GET,
        [](AsyncWebServerRequest *request)
        {
            request->send(
                200,
                "text/html",

                "<html>"
                "<head>"
                "<meta name='viewport' "
                "content='width=device-width, initial-scale=1'>"
                "</head>"

                "<body style='font-family:Arial;"
                "text-align:center;"
                "padding:50px;"
                "background:#03060b;"
                "color:#e0f2fe;'>"

                "<div style='background:#0d1524;"
                "padding:30px;"
                "border-radius:20px;"
                "max-width:500px;"
                "margin:auto;"
                "border:1px solid #38bdf8;'>"

                "<h1 style='color:#38bdf8;'>"
                "Changing WiFi..."
                "</h1>"

                "<p style='color:#94a3b8;'>"
                "WiFi settings will be cleared."
                "</p>"

                "<p style='color:#64748b;'>"
                "The ESP32 will restart."
                "</p>"

                "</div>"
                "</body>"
                "</html>"
            );


            delay(1000);


            deleteWiFiFiles();


            ESP.restart();
        }
    );


    // -------------------------------------------------
    // STATIC FILES
    // -------------------------------------------------

    server.serveStatic(
        "/",
        LittleFS,
        "/"
    );


    server.begin();


    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "      MAIN WEB SERVER READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.println();
}


// =====================================================
// FIREBASE CALLBACK
// =====================================================

void processFirebase(
    AsyncResult &aResult
)
{
    if (
        !aResult.isResult()
    )
    {
        return;
    }


    if (
        aResult.isEvent()
    )
    {
        Firebase.printf(
            "Firebase Event - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.eventLog().message().c_str(),
            aResult.eventLog().code()
        );
    }


    if (
        aResult.isDebug()
    )
    {
        Firebase.printf(
            "Firebase Debug - task: %s, msg: %s\n",
            aResult.uid().c_str(),
            aResult.debug().c_str()
        );
    }


    if (
        aResult.isError()
    )
    {
        Firebase.printf(
            "Firebase Error - task: %s, msg: %s, code: %d\n",
            aResult.uid().c_str(),
            aResult.error().message().c_str(),
            aResult.error().code()
        );
    }


    if (
        aResult.available()
    )
    {
        Firebase.printf(
            "Firebase Payload - task: %s, payload: %s\n",
            aResult.uid().c_str(),
            aResult.c_str()
        );
    }
}


// =====================================================
// FIREBASE SETUP
// =====================================================

void setupFirebase()
{
    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       FIREBASE SETUP"
    );

    Serial.println(
        "================================="
    );


    ssl_client.setInsecure();


    Serial.println(
        "Initializing Firebase..."
    );


    initializeApp(
        aClient,
        app,
        getAuth(user_auth),
        processFirebase,
        "authTask"
    );


    app.getApp<RealtimeDatabase>(
        Database
    );


    Database.url(
        DATABASE_URL
    );


    Serial.println(
        "Firebase initialization started."
    );


    Serial.println();
}


// =====================================================
// GET DATE
// =====================================================

String getDateString()
{
    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {
        return "1970-01-01";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d",
        &timeinfo
    );


    return String(
        buffer
    );
}


// =====================================================
// GET TIME
// =====================================================

String getTimeString()
{
    struct tm timeinfo;


    if (
        !getLocalTime(
            &timeinfo
        )
    )
    {
        return "00:00:00";
    }


    char buffer[20];


    strftime(
        buffer,
        sizeof(buffer),
        "%H:%M:%S",
        &timeinfo
    );


    return String(
        buffer
    );
}


// =====================================================
// SEND SENSOR DATA
// =====================================================

void sendSensorData()
{
    if (
        !app.ready()
    )
    {
        Serial.println();

        Serial.println(
            "Firebase not ready yet..."
        );

        return;
    }


    // -------------------------------------------------
    // DHT11 READING
    // -------------------------------------------------

    float humidity =
        dht.readHumidity();


    float temperature =
        dht.readTemperature();


    // -------------------------------------------------
    // CHECK READING
    // -------------------------------------------------

    if (
        isnan(humidity) ||
        isnan(temperature)
    )
    {
        Serial.println();

        Serial.println(
            "================================="
        );

        Serial.println(
            "ERROR: Failed to read DHT11"
        );

        Serial.println(
            "================================="
        );

        return;
    }


    // -------------------------------------------------
    // DATE / TIME
    // -------------------------------------------------

    String date =
        getDateString();


    String time =
        getTimeString();


    // -------------------------------------------------
    // BASE PATH
    // -------------------------------------------------

    String basePath =
        "/ESP32_Data/" +
        date +
        "/" +
        time;


    // -------------------------------------------------
    // SENSOR PATHS
    // -------------------------------------------------

    String temperaturePath =
        basePath +
        "/temperature";


    String humidityPath =
        basePath +
        "/humidity";


    // -------------------------------------------------
    // SERIAL
    // -------------------------------------------------

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "       DHT11 SENSOR READING"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Temperature: "
    );

    Serial.print(
        temperature,
        1
    );

    Serial.println(
        " °C"
    );


    Serial.print(
        "Humidity: "
    );

    Serial.print(
        humidity,
        1
    );

    Serial.println(
        " %"
    );


    Serial.print(
        "Date: "
    );

    Serial.println(
        date
    );


    Serial.print(
        "Time: "
    );

    Serial.println(
        time
    );


    Serial.print(
        "Firebase base path: "
    );

    Serial.println(
        basePath
    );


    Serial.println();


    // -------------------------------------------------
    // FIREBASE VALUES
    // -------------------------------------------------

    Serial.println(
        "Firebase values to write:"
    );


    Serial.print(
        "Temperature -> "
    );

    Serial.println(
        temperature,
        1
    );


    Serial.print(
        "Humidity    -> "
    );

    Serial.println(
        humidity,
        1
    );


    Serial.println();


    // -------------------------------------------------
    // WRITE TEMPERATURE
    // -------------------------------------------------

    Database.set<float>(
        aClient,
        temperaturePath,
        temperature,
        processFirebase,
        "temperatureTask"
    );


    // -------------------------------------------------
    // WRITE HUMIDITY
    // -------------------------------------------------

    Database.set<float>(
        aClient,
        humidityPath,
        humidity,
        processFirebase,
        "humidityTask"
    );


    // -------------------------------------------------
    // SERIAL CONFIRMATION
    // -------------------------------------------------

    Serial.println(
        "Temperature write task sent."
    );


    Serial.println(
        "Humidity write task sent."
    );


    Serial.println();


    Serial.println(
        "Firebase paths:"
    );


    Serial.print(
        "Temperature: "
    );

    Serial.println(
        temperaturePath
    );


    Serial.print(
        "Humidity: "
    );

    Serial.println(
        humidityPath
    );


    Serial.println(
        "================================="
    );

    Serial.println();
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(
        115200
    );


    delay(1000);


    Serial.println();

    Serial.println();


    Serial.println(
        "================================="
    );

    Serial.println(
        "   CYRUS ACTIVITY 4"
    );

    Serial.println(
        "   ESP32 DHT11 MONITOR"
    );

    Serial.println(
        "================================="
    );


    // =================================================
    // LITTLEFS
    // =================================================

    Serial.println();

    Serial.println(
        "Starting LittleFS..."
    );


    if (
        !LittleFS.begin(true)
    )
    {
        Serial.println(
            "LittleFS mount failed!"
        );


        while (true)
        {
            delay(1000);
        }
    }


    Serial.println(
        "LittleFS ready."
    );


    // =================================================
    // DHT11
    // =================================================

    dht.begin();


    Serial.println(
        "DHT11 initialized."
    );


    // =================================================
    // WIFI
    // =================================================

    bool connected =
        connectToSavedWiFi();


    if (!connected)
    {
        startWiFiManager();

        return;
    }


    // =================================================
    // NTP
    // Philippines UTC+8
    // =================================================

    Serial.println(
        "Starting NTP time..."
    );


    configTime(
        8 * 3600,
        0,
        "pool.ntp.org",
        "time.nist.gov",
        "time.google.com"
    );


    Serial.print(
        "Waiting for time"
    );


    struct tm timeinfo;

    int retry =
        0;


    while (
        !getLocalTime(
            &timeinfo
        ) &&
        retry < 20
    )
    {
        Serial.print(
            "."
        );

        delay(500);

        retry++;
    }


    Serial.println();


    if (
        getLocalTime(
            &timeinfo
        )
    )
    {
        Serial.println(
            "Time synchronized."
        );


        Serial.print(
            "Date: "
        );

        Serial.println(
            getDateString()
        );


        Serial.print(
            "Time: "
        );

        Serial.println(
            getTimeString()
        );
    }
    else
    {
        Serial.println(
            "WARNING: Time synchronization failed."
        );
    }


    // =================================================
    // FIREBASE
    // =================================================

    setupFirebase();


    // =================================================
    // WEB SERVER
    // =================================================

    startMainWebServer();


    // =================================================
    // READY
    // =================================================

    Serial.println();

    Serial.println(
        "================================="
    );

    Serial.println(
        "         SYSTEM READY"
    );

    Serial.println(
        "================================="
    );


    Serial.print(
        "Website: http://"
    );

    Serial.println(
        WiFi.localIP()
    );


    Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    // -------------------------------------------------
    // FIREBASE TASKS
    // -------------------------------------------------

    if (
        !wifiManagerMode
    )
    {
        app.loop();
    }


    // -------------------------------------------------
    // SENSOR EVERY 10 SECONDS
    // -------------------------------------------------

    if (
        !wifiManagerMode &&
        millis() - lastSensorRead >=
        SENSOR_INTERVAL
    )
    {
        lastSensorRead =
            millis();


        sendSensorData();
    }


    delay(10);
}