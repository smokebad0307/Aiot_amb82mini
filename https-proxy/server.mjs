import fs from "node:fs";
import http from "node:http";
import https from "node:https";
import path from "node:path";
import { fileURLToPath } from "node:url";

const here = path.dirname(fileURLToPath(import.meta.url));
const certDir = path.join(here, "certs");
const config = JSON.parse(fs.readFileSync(path.join(certDir, "config.json"), "utf8"));
const pfxPassword = process.env.TLS_PFX_PASSWORD;

if (!pfxPassword) throw new Error("TLS_PFX_PASSWORD is required. Start the server with start.ps1.");
if (!config.boardOrigin || !config.listenIp || !config.port) {
  throw new Error("certs/config.json is incomplete. Run create-cert.ps1 again.");
}

const board = new URL(config.boardOrigin);
if (board.protocol !== "http:") throw new Error("BOARD_ORIGIN must point to the AMB82 HTTP service.");

const tls = {
  pfx: fs.readFileSync(path.join(certDir, "server.pfx")),
  passphrase: pfxPassword,
};

const server = https.createServer(tls, (req, res) => {
  const requestTarget = req.url ?? "/";
  if (!requestTarget.startsWith("/") || requestTarget.startsWith("//")) {
    res.writeHead(400, { "Content-Type": "text/plain; charset=utf-8", "Cache-Control": "no-store" });
    return res.end("Invalid request path");
  }

  const upstream = http.request({
    protocol: board.protocol,
    hostname: board.hostname,
    port: board.port || 80,
    method: req.method,
    path: requestTarget,
    headers: { ...req.headers, host: board.host, connection: "close" },
  }, (boardRes) => {
    res.writeHead(boardRes.statusCode ?? 502, boardRes.statusMessage, boardRes.headers);
    boardRes.pipe(res);
  });

  upstream.setTimeout(5000, () => upstream.destroy(new Error("AMB82 request timed out")));
  upstream.on("error", (error) => {
    if (res.headersSent) return res.destroy(error);
    res.writeHead(502, { "Content-Type": "text/plain; charset=utf-8", "Cache-Control": "no-store" });
    res.end(`無法連接 AMB82-MINI：${error.message}`);
  });
  req.pipe(upstream);
});

server.listen(config.port, config.listenIp, () => {
  console.log(`HTTPS control page: https://${config.listenIp}:${config.port}/`);
  console.log(`Proxy target: ${config.boardOrigin}`);
  console.log("Keep this window and the computer running while using the page.");
});

server.on("error", (error) => {
  console.error(`HTTPS proxy failed: ${error.message}`);
  process.exitCode = 1;
});
