# iPhone HTTPS 控制頁

AMB82-MINI 的 Arduino 韌體仍透過 HTTP 在區域網路提供頁面與 API；本資料夾提供一個執行於 Windows 電腦的 HTTPS 反向代理。iPhone 瀏覽器只連到電腦上的 HTTPS 網址，代理再把頁面及 `/api/*` 請求轉送到開發板。使用時電腦、iPhone、AMB82 必須連在同一個 Wi-Fi，且電腦需要保持開機並執行代理。

## 一次性設定

1. 確認 AMB82 已連上 Wi-Fi，並從序列監控視窗確認板子 IP。下例使用 `192.168.50.96`；若板子 IP 不同，請換成實際 IP。
2. 在 Windows 安裝 Node.js LTS，安裝後重新開啟 PowerShell。若執行腳本時出現「已停用指令碼執行」，在這個 PowerShell 視窗執行以下指令，再重試腳本：

   ```powershell
   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
   ```

   這只暫時放寬目前視窗的執行原則；關閉視窗後會還原，不會永久變更系統設定。
3. 在專案資料夾執行：

   ```powershell
   .\https-proxy\create-cert.ps1 -BoardIp 192.168.50.96
   ```

   指令會自動找出電腦連往開發板的 Wi-Fi IPv4 位址，並建立本機 CA 與 HTTPS 伺服器憑證。請設定並記住 PFX 密碼。若自動選到的位址不正確，可用 `-ListenIp 192.168.50.xxx` 明確指定電腦的 Wi-Fi 位址。
4. 將 `https-proxy\certs\ios-root.cer` 安全地傳到 iPhone（例如 AirDrop）。在 iPhone 開啟檔案並安裝下載的描述檔，接著前往「設定 → 一般 → 關於本機 → 憑證信任設定」，對該根憑證啟用完整信任。沒有這個信任步驟，Safari 會警告憑證不受信任。

根憑證可讓這台 iPhone 信任此電腦簽發的本機 HTTPS 憑證。只在你控制的裝置上安裝它；不要公開或傳送 `server.pfx`，該檔案含伺服器私鑰。

## 每次使用

1. 確認電腦、iPhone、AMB82 在同一 Wi-Fi。
2. 在專案資料夾執行 `.\https-proxy\start.ps1`，輸入建立憑證時設定的 PFX 密碼，並保持 PowerShell 視窗開啟。若 Windows 防火牆詢問，僅允許「私人網路」上的存取。
3. 在 iPhone Safari 開啟 `create-cert.ps1` 顯示的 HTTPS 網址，例如 `https://192.168.50.197:8443/`，允許麥克風權限後操作。

關閉代理視窗、電腦休眠或 Wi-Fi 中斷時，iPhone 將無法連線。若電腦的 Wi-Fi IP 改變，請以新位址重新建立伺服器憑證；iPhone 使用的網址也要更新。`https-proxy\certs\` 已加入 `.gitignore`，不要將憑證或密碼提交到 Git。

## 設計界線

HTTPS 僅終止於執行代理的電腦；從電腦到 AMB82 的請求仍是同 Wi-Fi 內的 HTTP。請勿把代理埠或開發板 80 埠轉發到網際網路。這個代理沒有登入驗證，僅適合可信任的區域網路展示。
