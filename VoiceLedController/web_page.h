#pragma once

// 網頁直接由 AMB82-MINI 提供，無須另外安裝前端伺服器。
const char WEB_PAGE[] = R"HTML(
<!doctype html>
<html lang="zh-Hant">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <meta name="theme-color" content="#07111f">
  <title>AMB82 語音燈光控制</title>
  <style>
    :root {
      color-scheme: dark;
      --bg: #07111f;
      --panel: rgba(17, 30, 49, .82);
      --line: rgba(255,255,255,.10);
      --muted: #93a4b8;
      --text: #f4f8ff;
      --blue: #2f86ff;
      --green: #24d17e;
      --red: #ff6b76;
      --amber: #ffbd55;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      min-height: 100vh;
      font-family: Inter, ui-sans-serif, system-ui, -apple-system, "Segoe UI", sans-serif;
      color: var(--text);
      background:
        radial-gradient(circle at 15% 5%, rgba(47,134,255,.22), transparent 34rem),
        radial-gradient(circle at 90% 85%, rgba(36,209,126,.15), transparent 30rem),
        var(--bg);
    }
    .shell { width: min(940px, calc(100% - 32px)); margin: 0 auto; padding: 42px 0 56px; }
    header { display: flex; gap: 18px; align-items: center; justify-content: space-between; margin-bottom: 24px; }
    .eyebrow { color: #7fb1ff; font-size: 12px; font-weight: 750; letter-spacing: .16em; text-transform: uppercase; }
    h1 { margin: 5px 0 0; font-size: clamp(27px, 5vw, 44px); line-height: 1.08; letter-spacing: -.03em; }
    .connection { display: inline-flex; align-items: center; gap: 8px; padding: 9px 12px; border: 1px solid var(--line); border-radius: 999px; background: rgba(0,0,0,.18); color: var(--muted); font-size: 13px; white-space: nowrap; }
    .dot { width: 8px; height: 8px; border-radius: 50%; background: var(--amber); box-shadow: 0 0 14px currentColor; }
    .connection.online { color: #9eebc3; }
    .connection.online .dot { background: var(--green); }
    .connection.offline { color: #ffabb2; }
    .connection.offline .dot { background: var(--red); }
    .panel { border: 1px solid var(--line); border-radius: 24px; background: var(--panel); box-shadow: 0 20px 70px rgba(0,0,0,.25); backdrop-filter: blur(16px); }
    .voice { padding: clamp(22px, 5vw, 38px); text-align: center; }
    .hint { margin: 0 auto 24px; color: var(--muted); line-height: 1.7; }
    .hint strong { color: var(--text); }
    .mic {
      position: relative; width: 112px; height: 112px; border: 0; border-radius: 50%;
      color: white; cursor: pointer; background: linear-gradient(145deg, #398cff, #1765da);
      box-shadow: 0 18px 50px rgba(47,134,255,.35), inset 0 1px 1px rgba(255,255,255,.28);
      transition: transform .18s ease, filter .18s ease;
    }
    .mic:hover { transform: translateY(-2px); filter: brightness(1.08); }
    .mic:active { transform: scale(.96); }
    .mic:disabled { cursor: not-allowed; opacity: .45; box-shadow: none; }
    .mic.listening { background: linear-gradient(145deg, #ff707b, #dc3c51); animation: pulse 1.25s infinite; }
    .mic svg { width: 40px; height: 40px; fill: currentColor; }
    @keyframes pulse { 50% { box-shadow: 0 0 0 17px rgba(255,107,118,0), 0 18px 55px rgba(255,107,118,.42); } 0%,100% { box-shadow: 0 0 0 3px rgba(255,107,118,.25), 0 18px 55px rgba(255,107,118,.34); } }
    .listen-label { margin-top: 15px; min-height: 24px; font-size: 14px; color: #acd0ff; }
    .result-box { margin-top: 24px; padding: 18px; border-radius: 16px; background: rgba(3,10,20,.50); border: 1px solid var(--line); text-align: left; }
    .label { display: block; margin-bottom: 8px; color: var(--muted); font-size: 12px; letter-spacing: .08em; }
    #transcript { min-height: 27px; font-size: 19px; font-weight: 680; }
    #execution { min-height: 22px; margin-top: 7px; color: var(--muted); font-size: 14px; line-height: 1.55; }
    #execution.ok { color: #8ce5b7; }
    #execution.error { color: #ff9ea6; }
    .lights { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-top: 16px; }
    .light-card { padding: 22px; display: flex; align-items: center; gap: 16px; }
    .bulb { width: 54px; height: 54px; flex: 0 0 auto; border-radius: 50%; background: #233044; border: 1px solid rgba(255,255,255,.11); transition: all .25s ease; }
    .light-card.blue.on .bulb { background: var(--blue); box-shadow: 0 0 30px rgba(47,134,255,.72); }
    .light-card.green.on .bulb { background: var(--green); box-shadow: 0 0 30px rgba(36,209,126,.64); }
    .light-title { font-size: 17px; font-weight: 750; }
    .light-state { margin-top: 4px; color: var(--muted); font-size: 13px; }
    .reported { margin-left: auto; color: #7f91a7; font-size: 11px; text-align: right; }
    .controls { margin-top: 16px; padding: 18px; display: flex; flex-wrap: wrap; gap: 10px; align-items: center; }
    .controls span { color: var(--muted); font-size: 13px; margin-right: auto; }
    .small-btn { border: 1px solid var(--line); border-radius: 11px; background: rgba(255,255,255,.055); color: var(--text); padding: 10px 13px; cursor: pointer; font: inherit; }
    .small-btn:hover { background: rgba(255,255,255,.10); }
    .small-btn:disabled { opacity: .45; cursor: wait; }
    .notice { display: none; margin-top: 16px; padding: 13px 16px; border-radius: 14px; color: #ffd2d6; background: rgba(255,107,118,.10); border: 1px solid rgba(255,107,118,.25); font-size: 13px; line-height: 1.55; }
    .notice.show { display: block; }
    footer { margin-top: 18px; color: #6f8298; font-size: 12px; text-align: center; line-height: 1.6; }
    @media (max-width: 640px) {
      .shell { width: min(100% - 22px, 940px); padding-top: 24px; }
      header { align-items: flex-start; flex-direction: column; }
      .lights { grid-template-columns: 1fr; }
      .reported { margin-left: auto; }
      .controls span { width: 100%; margin-bottom: 3px; }
    }
  </style>
</head>
<body>
  <main class="shell">
    <header>
      <div><div class="eyebrow">AMB82-MINI · Voice Control</div><h1>語音燈光控制台</h1></div>
      <div id="connection" class="connection"><span class="dot"></span><span id="connectionText">正在連線</span></div>
    </header>

    <section class="panel voice">
      <p class="hint">按下麥克風後說：<strong>「左邊開燈」</strong>（藍燈）或 <strong>「右邊開燈」</strong>（綠燈）</p>
      <button id="mic" class="mic" type="button" aria-label="開始語音辨識">
        <svg viewBox="0 0 24 24" aria-hidden="true"><path d="M12 15a3 3 0 0 0 3-3V5a3 3 0 1 0-6 0v7a3 3 0 0 0 3 3Zm5-3a5 5 0 0 1-10 0H5a7 7 0 0 0 6 6.92V22h2v-3.08A7 7 0 0 0 19 12h-2Z"/></svg>
      </button>
      <div id="listenLabel" class="listen-label">點一下開始聆聽</div>
      <div class="result-box" aria-live="polite">
        <span class="label">語音辨識結果</span>
        <div id="transcript">尚未輸入語音</div>
        <div id="execution">等待指令</div>
      </div>
    </section>

    <section class="lights" aria-label="開發板回報狀態">
      <article id="blueCard" class="panel light-card blue">
        <div class="bulb"></div><div><div class="light-title">左邊 · 藍燈</div><div id="blueState" class="light-state">讀取中</div></div><div class="reported">AMB82<br>板端回報</div>
      </article>
      <article id="greenCard" class="panel light-card green">
        <div class="bulb"></div><div><div class="light-title">右邊 · 綠燈</div><div id="greenState" class="light-state">讀取中</div></div><div class="reported">AMB82<br>板端回報</div>
      </article>
    </section>

    <section class="panel controls">
      <span>手動測試／緊急關閉</span>
      <button class="small-btn" data-command="left_on">藍燈開</button>
      <button class="small-btn" data-command="left_off">藍燈關</button>
      <button class="small-btn" data-command="right_on">綠燈開</button>
      <button class="small-btn" data-command="right_off">綠燈關</button>
      <button class="small-btn" data-command="all_off">全部關閉</button>
    </section>
    <div id="notice" class="notice" role="alert"></div>
    <footer>只有白名單控制語句會送到開發板；其他內容不會改變 LED 狀態。</footer>
  </main>

  <script>
    'use strict';
    const $ = (id) => document.getElementById(id);
    const ui = {
      mic: $('mic'), listenLabel: $('listenLabel'), transcript: $('transcript'), execution: $('execution'),
      connection: $('connection'), connectionText: $('connectionText'), notice: $('notice'),
      blueCard: $('blueCard'), greenCard: $('greenCard'), blueState: $('blueState'), greenState: $('greenState')
    };
    const buttons = [...document.querySelectorAll('[data-command]')];
    let commandPending = false;

    function setConnection(ok) {
      ui.connection.className = 'connection ' + (ok ? 'online' : 'offline');
      ui.connectionText.textContent = ok ? '開發板已連線' : '通訊中斷';
    }
    function showNotice(message) {
      ui.notice.textContent = message || '';
      ui.notice.classList.toggle('show', Boolean(message));
    }
    function setExecution(message, type = '') {
      ui.execution.textContent = message;
      ui.execution.className = type;
    }
    function renderState(state) {
      ui.blueCard.classList.toggle('on', state.blue === true);
      ui.greenCard.classList.toggle('on', state.green === true);
      ui.blueState.textContent = state.blue ? '已開啟' : '已關閉';
      ui.greenState.textContent = state.green ? '已開啟' : '已關閉';
      setConnection(true);
      showNotice('');
    }
    async function fetchJson(url, options = {}, timeoutMs = 3500) {
      const controller = new AbortController();
      const timer = setTimeout(() => controller.abort(), timeoutMs);
      try {
        const response = await fetch(url, { cache: 'no-store', ...options, signal: controller.signal });
        let data;
        try { data = await response.json(); } catch (_) { throw new Error('開發板回應格式錯誤'); }
        if (!response.ok || data.ok === false) throw new Error(data.message || `HTTP ${response.status}`);
        return data;
      } finally { clearTimeout(timer); }
    }
    async function refreshState(silent = false) {
      try {
        renderState(await fetchJson('/api/state'));
      } catch (error) {
        setConnection(false);
        if (!silent) showNotice(`無法取得開發板狀態：${error.name === 'AbortError' ? '連線逾時' : error.message}`);
      }
    }
    async function sendCommand(command, sourceText) {
      if (commandPending) return;
      commandPending = true;
      buttons.forEach((button) => button.disabled = true);
      setExecution('指令傳送中…');
      try {
        const state = await fetchJson(`/api/command?name=${encodeURIComponent(command)}`, { method: 'POST' });
        renderState(state);
        setExecution(`執行成功：${state.message}（已收到開發板回報）`, 'ok');
      } catch (error) {
        setConnection(false);
        const reason = error.name === 'AbortError' ? '連線逾時' : error.message;
        setExecution(`執行失敗：${reason}；LED 狀態不明，請重新連線確認。`, 'error');
        showNotice(`控制指令「${sourceText}」未確認成功，請檢查 Wi-Fi 與開發板。`);
      } finally {
        commandPending = false;
        buttons.forEach((button) => button.disabled = false);
      }
    }

    // 完整比對白名單，避免一般談話誤觸 LED。
    const commandPhrases = new Map([
      ['左邊開燈', 'left_on'], ['左边开灯', 'left_on'], ['左邊開藍燈', 'left_on'], ['左边开蓝灯', 'left_on'], ['開藍燈', 'left_on'], ['开蓝灯', 'left_on'],
      ['右邊開燈', 'right_on'], ['右边开灯', 'right_on'], ['右邊開綠燈', 'right_on'], ['右边开绿灯', 'right_on'], ['開綠燈', 'right_on'], ['开绿灯', 'right_on'],
      ['左邊關燈', 'left_off'], ['左边关灯', 'left_off'], ['關藍燈', 'left_off'], ['关蓝灯', 'left_off'],
      ['右邊關燈', 'right_off'], ['右边关灯', 'right_off'], ['關綠燈', 'right_off'], ['关绿灯', 'right_off'],
      ['全部關燈', 'all_off'], ['全部关灯', 'all_off'], ['關閉全部', 'all_off'], ['关闭全部', 'all_off']
    ]);
    function normalizeSpeech(text) { return text.trim().replace(/[\s，。！？、,.!?：:；;]/g, ''); }
    function handleTranscript(text) {
      ui.transcript.textContent = text || '未辨識到語音';
      const command = commandPhrases.get(normalizeSpeech(text));
      if (!command) {
        setExecution('非控制指令：未傳送，LED 狀態保持不變。', 'error');
        refreshState(true);
        return;
      }
      sendCommand(command, text);
    }

    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    let recognition = null;
    if (SpeechRecognition) {
      recognition = new SpeechRecognition();
      recognition.lang = 'zh-TW';
      recognition.continuous = false;
      recognition.interimResults = false;
      recognition.maxAlternatives = 1;
      recognition.onstart = () => {
        ui.mic.classList.add('listening');
        ui.listenLabel.textContent = '正在聆聽，請說出指令…';
        ui.transcript.textContent = '辨識中…';
        setExecution('尚未傳送控制指令');
      };
      recognition.onresult = (event) => handleTranscript(event.results[0][0].transcript);
      recognition.onerror = (event) => {
        const messages = {
          'not-allowed': '麥克風權限遭拒，請在瀏覽器設定中允許後重試。',
          'no-speech': '沒有聽到語音，LED 狀態未改變。',
          'audio-capture': '找不到可用的麥克風。',
          'network': '語音辨識服務無法連線。'
        };
        const message = messages[event.error] || `語音辨識失敗（${event.error}）`;
        ui.transcript.textContent = '辨識失敗';
        setExecution(`${message} 未傳送控制指令。`, 'error');
      };
      recognition.onend = () => {
        ui.mic.classList.remove('listening');
        ui.listenLabel.textContent = '點一下再次聆聽';
      };
      ui.mic.addEventListener('click', () => {
        try {
          recognition.start();
        } catch (error) {
          // 快速連點時瀏覽器會丟 InvalidStateError；其他錯誤必須讓使用者知道。
          if (error.name !== 'InvalidStateError') {
            setExecution(`無法啟動麥克風：${error.message}`, 'error');
          }
        }
      });
    } else {
      ui.mic.disabled = true;
      ui.listenLabel.textContent = '此瀏覽器不支援 Web Speech API';
      setExecution('請改用最新版 Chrome、Edge 或 Safari；也可先用下方按鈕測試通訊。', 'error');
    }

    buttons.forEach((button) => button.addEventListener('click', () => {
      const label = button.textContent.trim();
      ui.transcript.textContent = `手動操作：${label}`;
      sendCommand(button.dataset.command, label);
    }));
    refreshState();
    setInterval(() => { if (!commandPending) refreshState(true); }, 5000);
  </script>
</body>
</html>
)HTML";
