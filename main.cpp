#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>


const char* ssid = "ssid"; 
const char* password = "password"; 
const char* apSSID = "ChatBox-Setup"; // ESP32 热点名称
const char* apPassword = "12345678"; // ESP32 热点密码

WebServer server(80);
WebSocketsServer webSocket(81);
std::vector<String> messageHistory; // 存储消息历史
const size_t maxMessages = 20; // 最大存储消息数量

// 设置页面HTML
const char* setupHtmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>设置ChatBox网络</title>
    <link rel="icon" type="image/svg+xml" href="data:image/svg+xml;charset=utf-8;base64,PHN2ZyB0PSIxNzE0NzEzMTAzNzMzIiBjbGFzcz0iaWNvbiIgdmlld0JveD0iMCAwIDExMjIgMTAyNCIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHAtaWQ9IjYxMzYiIHdpZHRoPSIxMDgwIiBoZWlnaHQ9IjEwODAiPjxwYXRoIGQ9Ik01NjEuMTUyIDkxMC4zMzZjLTcuNjggMC0yNS4wODgtMTMuODI0LTUzLjI0OC00MS45ODQtMjcuNjQ4LTI4LjE2LTQxLjQ3Mi00NS41NjgtNDEuNDcyLTUzLjI0OCAwLTEyLjI4OCAxMS43NzYtMjIuNTI4IDM1Ljg0LTMwLjcyczQzLjUyLTEyLjggNTkuMzkyLTEyLjhjMTUuODcyIDAgMzUuMzI4IDQuMDk2IDU5LjM5MiAxMi44czM1Ljg0IDE4LjQzMiAzNS44NCAzMC43MmMwIDcuNjgtMTMuODI0IDI1LjYtNDEuNDcyIDUzLjI0OC0yOS4xODQgMjguMTYtNDYuNTkyIDQxLjk4NC01NC4yNzIgNDEuOTg0eiIgZmlsbD0iIzEyOTZEQiIgcC1pZD0iNjEzNyI+PC9wYXRoPjxwYXRoIGQ9Ik03MTUuMjY0IDc1NS43MTJjLTAuNTEyIDAtOC4xOTItNC42MDgtMjMuMDQtMTQuMzM2LTE0LjMzNi05LjcyOC0zMy43OTItMTguOTQ0LTU3Ljg1Ni0yOC42NzItMjQuMDY0LTkuNzI4LTQ4LjY0LTE0LjMzNi03My4yMTYtMTQuMzM2LTI0LjU3NiAwLTQ5LjE1MiA0LjYwOC03My4yMTYgMTQuMzM2LTI0LjA2NCA5LjcyOC00My41MiAxOC45NDQtNTcuODU2IDI4LjY3Mi0xNC4zMzYgOS43MjgtMjIuMDE2IDE0LjMzNi0yMy4wNCAxNC4zMzYtNi42NTYgMC0yNC41NzYtMTQuMzM2LTUzLjI0OC00My4wMDhzLTQzLjAwOC00Ni4wOC00My4wMDgtNTMuMjQ4YzAtNS4xMiAyLjA0OC05LjIxNiA1LjYzMi0xMy4zMTIgMjkuNjk2LTI5LjE4NCA2Ny4wNzItNTIuMjI0IDExMi4xMjgtNjkuMTJzODkuMDg4LTI1LjA4OCAxMzMuMTItMjUuMDg4YzQ0LjAzMiAwIDg4LjA2NCA4LjE5MiAxMzMuMTIgMjUuMDg4IDQ1LjA1NiAxNi44OTYgODIuNDMyIDM5LjkzNiAxMTIuMTI4IDY5LjEyIDMuNTg0IDMuNTg0IDUuNjMyIDguMTkyIDUuNjMyIDEzLjMxMiAwIDYuNjU2LTE0LjMzNiAyNC41NzYtNDMuMDA4IDUzLjI0OC0yOS42OTYgMjguNjcyLTQ3LjEwNCA0My4wMDgtNTQuMjcyIDQzLjAwOHoiIGZpbGw9IiMxOUEzRTAiIHAtaWQ9IjYxMzgiPjwvcGF0aD48cGF0aCBkPSJNODcxLjQyNCA2MDAuMDY0Yy00LjA5NiAwLTguNzA0LTEuNTM2LTEzLjMxMi00LjYwOC01MS43MTItMzkuOTM2LTk5Ljg0LTY5LjYzMi0xNDMuODcyLTg4LjA2NHMtOTUuMjMyLTI4LjE2LTE1My4wODgtMjguMTZjLTMyLjI1NiAwLTY1LjAyNCA0LjA5Ni05Ny4yOCAxMi44UzQwMi45NDQgNTEwLjQ2NCAzNzguODggNTIyLjI0Yy0yNC4wNjQgMTEuNzc2LTQ1LjU2OCAyMy41NTItNjUuMDI0IDM1LjMyOHMtMzQuMzA0IDIyLjAxNi00NS4wNTYgMzAuMjA4Yy0xMS4yNjQgOC4xOTItMTYuODk2IDEyLjgtMTcuOTIgMTIuOC02LjY1NiAwLTI0LjA2NC0xNC4zMzYtNTIuNzM2LTQzLjAwOHMtNDMuMDA4LTQ2LjA4LTQzLjAwOC01My4yNDhjMC00LjYwOCAyLjA0OC04LjcwNCA1LjYzMi0xMi44IDUwLjE3Ni01MC4xNzYgMTExLjEwNC04OS4wODggMTgyLjc4NC0xMTcuMjQ4IDcxLjY4LTI3LjY0OCAxNDMuODcyLTQxLjQ3MiAyMTcuMDg4LTQxLjQ3MnMxNDUuNDA4IDEzLjgyNCAyMTcuMDg4IDQxLjQ3MmM3MS42OCAyNy42NDggMTMyLjYwOCA2Ny4wNzIgMTgyLjc4NCAxMTcuMjQ4IDMuNTg0IDMuNTg0IDUuNjMyIDguMTkyIDUuNjMyIDEyLjggMCA2LjY1Ni0xNC4zMzYgMjQuNTc2LTQzLjAwOCA1My4yNDhzLTQ1LjA1NiA0Mi40OTYtNTEuNzEyIDQyLjQ5NnoiIGZpbGw9IiM0OEI1RTUiIHAtaWQ9IjYxMzkiPjwvcGF0aD48cGF0aCBkPSJNMTAyNi4wNDggNDQ1LjQ0Yy00LjA5NiAwLTguMTkyLTEuNTM2LTEyLjgtNS4xMi02OC4wOTYtNTkuOTA0LTEzOC43NTItMTA0Ljk2LTIxMi40OC0xMzUuMTY4LTczLjIxNi0zMC4yMDgtMTUzLjYtNDUuNTY4LTI0MC4xMjgtNDUuNTY4LTg3LjA0IDAtMTY2LjkxMiAxNS4zNi0yNDAuMTI4IDQ1LjU2OFMxNzYuNjQgMzgwLjQxNiAxMDguNTQ0IDQ0MC4zMmMtNC4wOTYgMy41ODQtOC4xOTIgNS4xMi0xMi44IDUuMTItNi42NTYgMC0yNC4wNjQtMTQuMzM2LTUyLjczNi00My4wMDhTMCAzNTYuMzUyIDAgMzQ5LjE4NGMwLTUuMTIgMi4wNDgtOS4yMTYgNS42MzItMTMuMzEyQzc2LjggMjY1LjIxNiAxNjEuNzkyIDIxMC40MzIgMjYwLjA5NiAxNzEuNTJzMTk4LjY1Ni01OC4zNjggMzAxLjA1Ni01OC4zNjggMjAyLjc1MiAxOS40NTYgMzAxLjA1NiA1OC4zNjggMTgzLjI5NiA5My42OTYgMjU0LjQ2NCAxNjQuMzUyYzMuNTg0IDMuNTg0IDUuNjMyIDguMTkyIDUuNjMyIDEzLjMxMiAwIDYuNjU2LTE0LjMzNiAyNC41NzYtNDMuMDA4IDUzLjI0OC0yOC42NzIgMjguNjcyLTQ2LjU5MiA0My4wMDgtNTMuMjQ4IDQzLjAwOHoiIGZpbGw9IiM4MEQ4RkYiIHAtaWQ9IjYxNDAiPjwvcGF0aD48L3N2Zz4=">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            background-color: #f2f2f2;
        }
        .form-container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            text-align: center;
            width: 300px;
        }
        input[type="text"], input[type="password"], button {
            width: 100%;
            padding: 10px;
            margin-top: 10px;
            border-radius: 5px;
            border: 1px solid #ccc;
            box-sizing: border-box;
        }
        button {
            background-color: #4CAF50;
            color: white;
            border: none;
            cursor: pointer;
        }
        button:hover {
            background-color: #45a049;
        }
        .button-group {
            display: flex;
            justify-content: space-between;
            gap: 10px;
            margin-top: 0px;
        }
        ul {
            list-style-type: none;
            padding: 0;
        }
        li {
            padding: 8px;
            background: #ddd;
            border: none;
            text-align: left;
            cursor: pointer;
            border-radius: 5px;
            margin-top: 2px;
        }
        li:hover {
            background-color: #ccc;
        }
        .modal {
            display: none;
            position: fixed;
            z-index: 1;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            overflow: auto;
            background-color: rgba(0,0,0,0.4);
        }
        .modal-content {
            background-color: #fefefe;
            margin: 15% auto;
            padding: 20px;
            border: 1px solid #888;
            width: 50%; /* Adjust as needed */
        }
        .close {
            color: #aaa;
            float: right;
            font-size: 28px;
            font-weight: bold;
        }
        .close:hover,
        .close:focus {
            color: black;
            text-decoration: none;
            cursor: pointer;
        }
    </style>
</head>
<body onload="scanForNetworks()">
    <div class="form-container">
        <h1>连接到Wi-Fi</h1>
        <form action="/connect" method="post">
            <input type="text" name="ssid" placeholder="SSID">
            <input type="password" name="password" placeholder="Password">
            <button type="submit">连接</button>
        </form>
        <div class="button-group">
            <button onclick="scanForNetworks()">扫描Wi-Fi网络</button>
            <button onclick="openDocumentation()">查看教程文档</button>
        </div>
        <div id="networks"></div>
    </div>
    <!-- The Modal -->
    <div id="myModal" class="modal">
        <div class="modal-content">
            <span class="close">&times;</span>
            <h2>ChatBox设置文档</h2>
            <p>这是配置 ESP-32 设备的简单指南。</p>
            <h3>步骤</h3>
            <ol>
                <li>确保您的 ChatBox 已通电且在 Wi-Fi 路由器的范围内。</li>
                <li>使用“扫描 Wi-Fi 网络”按钮查找可用网络。</li>
                <li>从列表中选择您的网络或手动填写SSID并输入密码。</li>
                <li>按下“连接”尝试连接您填写的 Wi-Fi 信息。</li>
                <li>如提示连接成功即可断开ChatBox-Setup的AP网络。</li>
                <li>在您的设备上连接与ChatBox同一个 Wi-Fi 并输入刚才连接成功时显示的IP。大功告成!!</li>
            </ol>
            <p>如果遇到任何问题，请重新启动设备并重试，或联系支持人员。</p>
        </div>
    </div>
    <script>
        function fillSSID(ssid) {
            document.getElementsByName('ssid')[0].value = ssid;
        }
        function scanForNetworks() {
            fetch('/scan').then(response => response.text()).then(html => {
                document.getElementById('networks').innerHTML = html;
            });
        }
        var modal = document.getElementById("myModal");
        var span = document.getElementsByClassName("close")[0];
        function openDocumentation() {
            modal.style.display = "block";
        }
        span.onclick = function() {
            modal.style.display = "none";
        }
        window.onclick = function(event) {
            if (event.target == modal) {
                modal.style.display = "none";
            }
        }
    </script>
</body>
</html>


)rawliteral";

// 聊天室页面HTML
const char* chatHtmlContent = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>聊天盒 - ChatBox</title>
    <link rel="icon" type="image/svg+xml" href="data:image/svg+xml;charset=utf-8;base64,PHN2ZyB0PSIxNzE0NzEzMTkxNDQyIiBjbGFzcz0iaWNvbiIgdmlld0JveD0iMCAwIDEwMjQgMTAyNCIgdmVyc2lvbj0iMS4xIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHAtaWQ9Ijk4NjUiIHdpZHRoPSIxMDgwIiBoZWlnaHQ9IjEwODAiPjxwYXRoIGQ9Ik01MTIgODkzYy04IDAtMTYtMi40LTIyLjgtNy4xTDMwOS41IDc2MWgtOTEuOWMtNjUuMSAwLTExOC01My4zLTExOC0xMTguOFYyNDkuOGMwLTY1LjUgNTMtMTE4LjggMTE4LTExOC44aDU4OC45YzY1LjEgMCAxMTggNTMuMyAxMTggMTE4Ljh2MzkyLjVjMCA2NS41LTUzIDExOC44LTExOCAxMTguOGgtOTEuOUw1MzQuOCA4ODUuOUM1MjggODkwLjYgNTIwIDg5MyA1MTIgODkzek0yMTcuNCAyMTFjLTIxIDAtMzggMTcuNC0zOCAzOC44djM5Mi41YzAgMjEuNCAxNy4xIDM4LjggMzggMzguOEgzMjJjOC4yIDAgMTYuMSAyLjUgMjIuOCA3LjFMNTEyIDgwNC4zbDE2Ny4yLTExNi4xYzYuNy00LjcgMTQuNy03LjEgMjIuOC03LjFoMTA0LjVjMjEgMCAzOC0xNy40IDM4LTM4LjhWMjQ5LjhjMC0yMS40LTE3LjEtMzguOC0zOC0zOC44SDIxNy40eiIgZmlsbD0iIzRhNWZlMiIgcC1pZD0iOTg2NiI+PC9wYXRoPjxwYXRoIGQ9Ik0zNzIuNSA0NTJtLTUwIDBhNTAgNTAgMCAxIDAgMTAwIDAgNTAgNTAgMCAxIDAtMTAwIDBaIiBmaWxsPSIjN2M0NGUyIiBwLWlkPSI5ODY3Ij48L3BhdGg+PHBhdGggZD0iTTUxMiA0NTJtLTUwIDBhNTAgNTAgMCAxIDAgMTAwIDAgNTAgNTAgMCAxIDAtMTAwIDBaIiBmaWxsPSIjN2M0NGUyIiBwLWlkPSI5ODY4Ij48L3BhdGg+PHBhdGggZD0iTTY1MS4zIDQ1Mm0tNTAgMGE1MCA1MCAwIDEgMCAxMDAgMCA1MCA1MCAwIDEgMC0xMDAgMFoiIGZpbGw9IiM3YzQ0ZTIiIHAtaWQ9Ijk4NjkiPjwvcGF0aD48L3N2Zz4=">

    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
            background-color: #f2f2f2;
        }
        .container {
            flex: 1;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            margin: 20px;
        }
        #chatBox {
            width: 95%;
            max-width: 95%;
            background: white;
            border-radius: 15px;
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
            overflow: hidden;
            padding: 10px;
            margin-bottom: 20px;
            height: 50vh;
            overflow-y: scroll;
        }
        .message {
            background: #f1f1f1;
            border-radius: 10px;
            padding: 10px;
            margin: 5px;
            width: fit-content;
            max-width: 80%;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .input-container {
            display: flex;
            align-items: center;
            width: 100%;
            margin-bottom: 20px;
        }
        input[type="text"] {
            flex: 1;
            padding: 10px;
            border: none;
            border-radius: 20px;
            margin-right: 10px;
        }
        button {
            padding: 10px 20px;
            border: none;
            border-radius: 20px;
            background-color: #4CAF50;
            color: white;
            font-size: 16px;
            cursor: pointer;
        }
    </style>
    <script>
        var ws;
        function initWebSocket() {
            ws = new WebSocket('ws://' + window.location.hostname + ':81/');
            ws.onmessage = function(event) {
                var chatBox = document.getElementById('chatBox');
                var message = document.createElement('div');
                message.classList.add('message');
                message.innerText = event.data;
                chatBox.appendChild(message);
                chatBox.scrollTop = chatBox.scrollHeight;
            };
        }
        function sendMessage() {
            var input = document.getElementById('messageInput');
            if (input.value) {
                ws.send(input.value);
                input.value = '';
            }
        }
        window.onload = initWebSocket;
    </script>
</head>
<body>
    <div class="container">
        <div id="chatBox"></div>
        <div class="input-container">
            <input type="text" id="messageInput" placeholder="输入内容..." onkeypress="if(event.keyCode == 13) sendMessage();">
            <button onclick="sendMessage()">发送</button>
        </div>
    </div>
</body>
</html>
)rawliteral";
void scanNetworks() {
    int n = WiFi.scanNetworks();
    String networkList = "<ul>";
    for (int i = 0; i < n; ++i) {
        // 输出Wi-Fi网络名称（SSID）
        networkList += "<li onclick='fillSSID(\"" + WiFi.SSID(i) + "\")'>" + WiFi.SSID(i) + "</li>";
    }
    networkList += "</ul>";
    server.send(200, "text/html", networkList);
}


void setupAP() {
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Setup AP mode");
    Serial.println("AP SSID: " + String(apSSID) + " Password: " + String(apPassword));
}

void handleRoot() {
    if (WiFi.status() != WL_CONNECTED) {
        server.send(200, "text/html", setupHtmlContent);
    } else {
        server.send(200, "text/html", chatHtmlContent);
    }
}

void handleWiFiConnect() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    WiFi.disconnect();  // 断开任何现有的Wi-Fi连接
    WiFi.begin(ssid.c_str(), password.c_str());  // 尝试连接到新的Wi-Fi网络

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 30) {  // 等待最多30秒以连接Wi-Fi
        delay(1000);
        Serial.print(".");
        counter++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        String ipMessage = "<h1>成功连接至: " + ssid + "</h1><h2>IP地址: " + WiFi.localIP().toString() + "</h2>";
        // 发送HTML页面，显示连接成功和当前IP地址，没有包括进入聊天室的链接
        server.sendHeader("Content-Type", "text/html; charset=utf-8");
        server.send(200, "text/html", ipMessage);
    } else {
        String failMessage = "<h1>连接失败，请返回重试。</h1>";  // 连接失败的消息
        server.sendHeader("Content-Type", "text/html; charset=utf-8");
        server.send(200, "text/html", failMessage);
    }
}




void sendHistory(uint8_t num) {
    for (const String& msg : messageHistory) {
        String messageToSend = msg;
        webSocket.sendTXT(num, messageToSend);
    }
}

void handleChat() {
    if (WiFi.status() == WL_CONNECTED) {
        server.send(200, "text/html", chatHtmlContent);
    } else {
        server.send(200, "text/html", "<h1>请先连接到Wi-Fi</h1>");
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        String externalIP = getExternalIP();
        String location = getIPGeoLocation(externalIP);
        
        String msg = "[" + externalIP + " | " + location + "]: " + String((char*)payload);
        Serial.println(msg);
        if (messageHistory.size() >= maxMessages) {
            messageHistory.erase(messageHistory.begin());
        }
        messageHistory.push_back(msg);
        webSocket.broadcastTXT(msg);
    } else if (type == WStype_CONNECTED) {
        sendHistory(num);
    }
}


String getExternalIP() {
    HTTPClient http;
    http.begin("http://api.ipify.org/?format=text");
    int httpCode = http.GET();
    
    String ip = "";
    if (httpCode == HTTP_CODE_OK) {
        ip = http.getString();  // Get the response as plain text
    }
    http.end();
    return ip;
}

String getIPGeoLocation(String ip) {
    HTTPClient http;
    http.begin("http://ip-api.com/json/" + ip + "?fields=61439&lang=zh-CN");
    int httpCode = http.GET();
    
    String location = "";
    if (httpCode == HTTP_CODE_OK) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, http.getString());
        location = doc["country"].as<String>() + ", " + doc["regionName"].as<String>() + ", " + doc["city"].as<String>();
    }
    http.end();
    return location;
}


void setup() {
    Serial.begin(115200);
    setupAP();
    server.on("/", handleRoot);
    server.on("/connect", HTTP_POST, handleWiFiConnect);
    server.on("/chat", handleChat);
    server.on("/scan", HTTP_GET, scanNetworks); // 路由处理扫描网络请求
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    server.handleClient();
    webSocket.loop();
}
