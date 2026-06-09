import time
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

DEVICE_PATH = "/dev/CDD_GPIO"

state = {
    "signal": 1,
    "data": [],
    "running": True
}

def read_device():
    while state["running"]:
        try:
            with open(DEVICE_PATH, "r") as f:
                val = int(f.read().strip())
            ts = time.time()
            state["data"].append((ts, val))
            if len(state["data"]) > 60:
                state["data"].pop(0)
        except Exception as e:
            print(f"Error leyendo dispositivo: {e}")
        time.sleep(1)

class Handler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        pass

    def do_GET(self):
        parsed = urlparse(self.path)

        if parsed.path == "/select":
            qs = parse_qs(parsed.query)
            sig = int(qs.get("signal", [1])[0])
            if sig in (1, 2):
                with open(DEVICE_PATH, "w") as f:
                    f.write(str(sig))
                state["signal"] = sig
                state["data"].clear()
            self.send_response(200)
            self.send_header("Content-Type", "text/plain")
            self.end_headers()
            self.wfile.write(b"OK")

        elif parsed.path == "/stream":
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            last_len = 0
            try:
                while True:
                    if len(state["data"]) > last_len:
                        ts, val = state["data"][-1]
                        msg = f"data: {ts},{val},{state['signal']}\n\n"
                        self.wfile.write(msg.encode())
                        self.wfile.flush()
                        last_len = len(state["data"])
                    time.sleep(0.5)
            except:
                pass

        else:
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.end_headers()
            self.wfile.write(HTML_PAGE.encode())

HTML_PAGE = """<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>CDD Signal Monitor</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial, sans-serif; background: #1a1a2e; color: #eee;
           display: flex; flex-direction: column; align-items: center; padding: 20px; }
    h1 { color: #00d4ff; }
    .controls { margin: 15px 0; display: flex; gap: 10px; }
    button { padding: 10px 25px; border: none; border-radius: 6px;
             font-size: 16px; cursor: pointer; transition: 0.2s; }
    #btn1 { background: #00d4ff; color: #000; }
    #btn2 { background: #ff6b6b; color: #fff; }
    button:hover { opacity: 0.8; transform: scale(1.05); }
    canvas { background: #16213e; border-radius: 10px; padding: 10px; }
    #status { margin: 10px; font-size: 14px; color: #aaa; }
  </style>
</head>
<body>
  <h1>CDD Signal Monitor</h1>
  <div class="controls">
    <button id="btn1" onclick="selectSignal(1)">Señal 1 (GPIO 17)</button>
    <button id="btn2" onclick="selectSignal(2)">Señal 2 (GPIO 27)</button>
  </div>
  <div id="status">Conectando...</div>
  <canvas id="chart" width="800" height="400"></canvas>
  <script>
    let currentSignal = 1;
    let startTime = Date.now() / 1000;
    const ctx = document.getElementById('chart').getContext('2d');
    const chart = new Chart(ctx, {
      type: 'line',
      data: { datasets: [{ label: 'Señal 1 — GPIO 17 (0=LOW / 1=HIGH)',
        data: [], borderColor: '#00d4ff', backgroundColor: 'rgba(0,212,255,0.1)',
        borderWidth: 2, pointRadius: 4, stepped: true }] },
      options: { animation: false,
        scales: {
          x: { type: 'linear', title: { display: true, text: 'Tiempo (s)', color: '#aaa' },
               ticks: { color: '#aaa' }, grid: { color: '#333' } },
          y: { min: -0.1, max: 1.1,
               title: { display: true, text: 'Estado (0=LOW / 1=HIGH)', color: '#aaa' },
               ticks: { color: '#aaa', stepSize: 1 }, grid: { color: '#333' } }
        },
        plugins: { legend: { labels: { color: '#eee' } } }
      }
    });

    function selectSignal(sig) {
      fetch('/select?signal=' + sig).then(() => {
        currentSignal = sig;
        startTime = Date.now() / 1000;
        chart.data.datasets[0].data = [];
        chart.data.datasets[0].borderColor = sig === 1 ? '#00d4ff' : '#ff6b6b';
        chart.data.datasets[0].label = 'Señal ' + sig + ' — GPIO ' + (sig===1?17:27) + ' (0=LOW / 1=HIGH)';
        chart.update();
        document.getElementById('status').textContent = 'Graficando Señal ' + sig;
      });
    }

    const evtSource = new EventSource('/stream');
    evtSource.onmessage = (e) => {
      const [ts, val, sig] = e.data.split(',').map(Number);
      if (sig !== currentSignal) return;
      const relTime = parseFloat((ts - startTime).toFixed(1));
      chart.data.datasets[0].data.push({ x: relTime, y: val });
      if (chart.data.datasets[0].data.length > 60)
        chart.data.datasets[0].data.shift();
      chart.update('none');
      document.getElementById('status').textContent =
        'Señal ' + sig + ' | t=' + relTime + 's | valor=' + val;
    };
    evtSource.onerror = () => {
      document.getElementById('status').textContent = 'Sin conexión con la Raspi';
    };
  </script>
</body>
</html>"""

if __name__ == "__main__":
    print("Iniciando lector de dispositivo...")
    t = threading.Thread(target=read_device, daemon=True)
    t.start()
    print("Servidor web en http://0.0.0.0:8080")
    server = HTTPServer(("0.0.0.0", 8080), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        state["running"] = False
        print("\nServidor detenido.")
