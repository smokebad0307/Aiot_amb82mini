# AMB82-MINI 語音控制藍燈／綠燈

這個專案讓電腦或手機透過瀏覽器麥克風辨識中文指令，再以區域網路 HTTP 將白名單控制指令送至 AMB82-MINI（RTL8735B）。控制頁由開發板本身提供，不需要另外架設 Web 伺服器。

## 已實作功能

- 「左邊開燈」控制板載藍燈 `LED_B`。
- 「右邊開燈」控制板載綠燈 `LED_G`。
- 另提供左右關燈、全部關燈，以及手動測試按鈕。
- 頁面同時顯示辨識文字、指令執行結果與開發板回傳的 LED 狀態。
- 非白名單語句不傳送；非法 API 指令也不執行 `digitalWrite()`。
- HTTP 逾時、Wi-Fi 中斷、麥克風權限遭拒或語音辨識失敗都有明確提示。

## 燒錄步驟

1. 安裝 Arduino IDE。
2. 在「檔案 → 偏好設定 → 額外的開發板管理員網址」加入：

   `https://github.com/Ameba-AIoT/ameba-arduino-pro2/raw/main/Arduino_package/package_realtek_amebapro2_index.json`

3. 在開發板管理員安裝 **Realtek Ameba Boards**，選擇「AmebaPro2 ARM (32-bits) Boards → AMB82-MINI」。
4. 將 `VoiceLedController/wifi_config.example.h` 複製為 `VoiceLedController/wifi_config.h`，再填入 Wi-Fi SSID 與密碼。`wifi_config.h` 已列入 `.gitignore`，不會提交真實密碼。
5. 用 Arduino IDE 開啟 `VoiceLedController/VoiceLedController.ino`，編譯並上傳。
6. 開啟序列監控視窗，鮑率設為 `115200`。重新啟動後會顯示例如：

   `Open this URL: http://192.168.1.123`

7. 讓電腦或手機連上相同 Wi-Fi，用瀏覽器開啟該網址，允許麥克風權限後即可操作。

若板子無法自動進入燒錄模式：按住 **UART_DOWNLOAD**，按一下 **RESET**，放開 RESET 後再放開 UART_DOWNLOAD，然後重新上傳。

## 操作與驗收

建議使用最新版 Chrome 或 Edge（Android 可用 Chrome；iPhone/iPad 可嘗試 Safari）。Web Speech API 的瀏覽器支援度並非完全一致，而且部分瀏覽器會使用線上辨識服務，因此語音辨識時可能需要網際網路。

| 測試 | 預期結果 |
|---|---|
| 說「左邊開燈」 | 辨識結果顯示原句；藍燈亮；介面收到板端回報後顯示「已開啟」 |
| 說「右邊開燈」 | 辨識結果顯示原句；綠燈亮；介面收到板端回報後顯示「已開啟」 |
| 說「今天天氣很好」 | 顯示「非控制指令」；不傳送控制要求；LED 保持原狀 |
| 拒絕麥克風權限 | 顯示權限錯誤；LED 保持原狀 |
| 操作時關閉板子或 Wi-Fi | 約 3.5 秒內顯示連線逾時／通訊中斷，不把前端狀態冒充成板端狀態 |
| 呼叫 `POST /api/command?name=invalid` | 回傳 HTTP 400；兩顆 LED 保持原狀 |

## HTTP API

- `GET /api/state`：取得板端藍燈、綠燈狀態與最後一次合法指令。
- `POST /api/command?name=left_on`
- `POST /api/command?name=right_on`
- `POST /api/command?name=left_off`
- `POST /api/command?name=right_off`
- `POST /api/command?name=all_off`

此服務未做登入驗證，設計用途是課堂展示或可信任的區域網路。請勿將 80 埠直接轉發到網際網路。
