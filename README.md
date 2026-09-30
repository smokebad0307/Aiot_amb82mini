# AMB82-MINI 語音控制藍燈／綠燈

這個專案讓電腦或手機透過瀏覽器麥克風辨識中文指令，再以區域網路將白名單控制指令送至 AMB82-MINI（RTL8735B）。開發板本身提供 HTTP 控制頁；另附一個在同 Wi-Fi 電腦上執行的 HTTPS 反向代理，讓 iPhone Safari 能在安全來源下使用麥克風。HTTPS 代理的設定方式見 [`https-proxy/README.md`](https-proxy/README.md)。

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

7. 讓電腦或手機連上相同 Wi-Fi。電腦可直接開啟序列監控視窗印出的 HTTP 網址；iPhone 請依下方步驟使用 HTTPS。

若板子無法自動進入燒錄模式：按住 **UART_DOWNLOAD**，按一下 **RESET**，放開 RESET 後再放開 UART_DOWNLOAD，然後重新上傳。

## iPhone HTTPS 使用方式

AMB82-MINI 韌體提供區域網路 HTTP 頁面；iPhone 透過同一 Wi-Fi 上的 Windows 電腦 HTTPS 代理操作。電腦、iPhone 與開發板須連在相同 Wi-Fi，且使用時電腦需保持開機。

第一次設定：

1. 在 Windows 安裝 Node.js LTS，並確認序列監控視窗顯示的 AMB82 IP（以下以 `192.168.50.96` 為例）。
2. 在專案資料夾開啟 PowerShell。若出現「已停用指令碼執行」錯誤，先在這個視窗執行以下指令，暫時允許本視窗執行腳本：

   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
   ```

   此設定只對目前 PowerShell 視窗有效，關閉視窗後會還原，不會永久變更系統原則。
3. 執行憑證建立腳本：

   ```powershell
   .\https-proxy\create-cert.ps1 -BoardIp 192.168.50.96
   ```

   設定並記住伺服器憑證密碼。腳本會顯示 iPhone 要開啟的 HTTPS 網址。
4. 將 `https-proxy\certs\ios-root.cer` 傳到 iPhone 並安裝描述檔；接著到「設定 → 一般 → 關於本機 → 憑證信任設定」，對該根憑證啟用完整信任。只在自己信任的裝置安裝此根憑證，勿分享 `server.pfx`（內含私鑰）。

### Windows 桌面版 Chrome 信任憑證

若要在 Windows 桌面版 Chrome 開啟此 HTTPS 網址，也要讓 Windows 信任本專案產生的根憑證：

1. 在檔案總管找到 `https-proxy\certs\ios-root.cer`，按右鍵選「安裝憑證」。
2. 選「目前使用者」，再選「將所有憑證放入以下存放區」→「受信任的根憑證授權單位」，完成匯入。
3. 關閉並重新開啟 Chrome，再前往代理顯示的 HTTPS 網址。

這會讓目前 Windows 使用者信任由此根憑證簽發的網站憑證；只匯入自己剛產生的憑證，不要信任陌生來源的根憑證。Chrome 在 Windows 會使用作業系統提供的自訂根憑證。[Chrome 憑證說明](https://support.google.com/chrome/answer/95617)

在 Chrome 網址列左側的網站資訊圖示中，打開「網站設定」，將「麥克風」設為「允許」，再重新載入頁面。也可到 `chrome://settings/content/microphone` 檢查是否曾封鎖此網站。[Chrome 麥克風權限說明](https://support.google.com/chrome/answer/2693767)

若使用 iPhone 上的 Chrome，請依前面的 iPhone 步驟安裝並信任憑證；不要在 Windows 匯入。iPhone 上手動安裝的根憑證需在 iOS「憑證信任設定」啟用完整信任。[Apple 憑證信任說明](https://support.apple.com/zh-cn/102390)

每次使用：

1. 確認電腦、iPhone、AMB82 都在相同 Wi-Fi。
2. 在專案資料夾 PowerShell 執行 `.\https-proxy\start.ps1`，輸入憑證密碼，並保持視窗開啟。
3. 在 iPhone Safari 開啟憑證設定腳本顯示的 `https://<電腦 Wi-Fi IP>:8443/` 網址，允許麥克風權限。

若日後在新的 PowerShell 視窗執行 `start.ps1` 時也遇到腳本執行遭封鎖，請在該視窗重新執行上述 `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force`；此設定只維持到該視窗關閉。

此代理只適用於可信任的區域網路，沒有登入驗證；不要將代理或開發板埠轉發到網際網路。完整說明見 [`https-proxy/README.md`](https-proxy/README.md)。

## 操作與驗收

建議使用最新版 Chrome 或 Edge（Android 可用 Chrome；iPhone/iPad 使用 Safari，並透過 HTTPS 代理開啟）。Web Speech API 的瀏覽器支援度並非完全一致，而且部分瀏覽器會使用線上辨識服務，因此語音辨識時可能需要網際網路。

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
