"use strict";
const $ = id => document.getElementById(id);
const DEFECTO = {kp: 15, ki: 0, kd: 1.36, sp: 0.58, vkp: 2.5, vki: 0.5};   // valores del firmware
const LIM_PID = {kp: [0, 100], ki: [0, 50], kd: [0, 10], sp: [-20, 20]};
const LIM_PI  = {vkp: [0, 20], vki: [0, 10]};
const LIM_REF = 0.5;                                        // m/s
const LIMITES = {...LIM_PID, ...LIM_PI};
const NOMBRES = {kp: "Kp", ki: "Ki", kd: "Kd", sp: "El setpoint", vkp: "Kp de velocidad", vki: "Ki de velocidad"};
const ANGULO_CORTE = 30, DT = 0.05, VENTANA = 400, MAX_REG = 12000, ZM = [12, 11];

let conf = {...DEFECTO}, vel = {act: false, ref: 0}, motor = false, pausado = false, finPausa = 0;
const reg = [];                                             // registro de muestras
const clamp = (v, a, b) => Math.min(b, Math.max(a, v));

/* ---------- Fuente de datos: ROBOT por WebSocket (Etapa 4, solo lectura) ---------- */
let ws, ultimoDato = 0, enlace = false;
function chip(id, txt, cls) { const c = $(id); c.textContent = txt; c.className = "chip " + cls; }

const pendientes = new Map();
let idCmd = 1, armSeg = 0;

// Envía un comando y espera la confirmación (ACK) del robot.
function enviar(cmd) {
  return new Promise(res => {
    if (!ws || ws.readyState !== 1) return res({ok: false, error: "Sin conexión con el robot"});
    const id = idCmd++;
    const t = setTimeout(() => { pendientes.delete(id); res({ok: false, error: "El robot no confirmó el comando"}); }, 2000);
    pendientes.set(id, r => { clearTimeout(t); res(r); });
    let msg = {cmd: cmd.tipo, id};
    if (cmd.tipo === "params") msg = {...msg, ...cmd.valores};
    if (cmd.tipo === "motor") msg.on = !!cmd.on;
    if (cmd.tipo === "vel") msg = {...msg, ...cmd.valores, vact: cmd.valores.vact ? 1 : 0};
    ws.send(JSON.stringify(msg));
  });
}

function conectar() {
  // Si abres index.html desde el PC conectado a la red del robot, también funciona.
  const host = location.protocol === "file:" ? "192.168.4.1" : location.host;
  ws = new WebSocket(`ws://${host}/ws`);
  ws.onopen = () => chip("chipRed", "WiFi: conectado", "ok");
  ws.onclose = () => {
    chip("chipRed", "WiFi: reconectando", "mal"); enlace = false; tarjetasEstado(); mostrarPI();
    setTimeout(conectar, 1000);
  };
  ws.onmessage = e => {
    let j; try { j = JSON.parse(e.data); } catch { return; }
    if (j.tipo === "hola") {
      reg.length = 0; finPausa = 0;
      guardarConf(j); mostrarConf(); llenarForm(conf); mostrarPI();
    } else if (j.tipo === "ack") {
      guardarConf(j); mostrarConf(); mostrarPI();
      const p = pendientes.get(j.id); if (p) { pendientes.delete(j.id); p({ok: true}); }
    } else if (j.tipo === "err") {
      const p = pendientes.get(j.id);
      if (p) { pendientes.delete(j.id); p({ok: false, error: j.msg}); } else aviso(j.msg, true);
    } else if (j.tipo === "tel") recibir(j);
  };
}

function guardarConf(j) {
  conf = {kp: j.kp, ki: j.ki, kd: j.kd, sp: j.sp, vkp: j.vkp, vki: j.vki};
  vel = {act: !!j.vact, ref: j.vref};
}

function recibir(j) {
  ultimoDato = performance.now(); enlace = true;
  chip("chipFuente", "Datos del robot", "ok");
  motor = j.mo === 1; armSeg = j.mo === 2 ? (j.ar || 1) : 0;
  vel = {act: !!j.va, ref: j.vr};
  const m = {t: j.t / 1000, ang: j.ang, sp: j.sp + j.of, err: j.err, om: j.om, u: j.u,
             pl: j.pl, pr: j.pr, rl: j.rl, rr: j.rr, hz: j.hz,
             vm: j.vm, vr: j.vr, of: j.of};
  reg.push(m); if (reg.length > MAX_REG) { reg.shift(); if (pausado) finPausa--; }
  tarjetas(m); graficas();
}

function mostrarPI() {
  $("refGrande").textContent = vel.ref.toFixed(2);
  $("mVr").textContent = vel.ref.toFixed(2);
  const c = $("chipPI");
  if (!vel.act) { c.textContent = "PI velocidad: apagado"; c.className = "chip"; }
  else if (!enlace) { c.textContent = `PI activo · última ref ${vel.ref.toFixed(2)} m/s (sin enlace)`; c.className = "chip alerta"; }
  else { c.textContent = `PI activo · ref ${vel.ref.toFixed(2)} m/s`; c.className = "chip " + (vel.ref !== 0 ? "alerta" : "ok"); }
  $("refAviso").textContent = !enlace
    ? `Sin enlace: el robot conserva la última referencia conocida (${vel.ref.toFixed(2)} m/s). Puede estar desactualizada.`
    : "";
}

function tarjetasEstado() {
  const cm = $("chipMotor");
  if (!enlace) { cm.textContent = "Motores: estado desconocido (sin enlace)"; cm.className = "chip alerta"; }
  else if (armSeg > 0) { cm.textContent = `Armando: ${armSeg} s`; cm.className = "chip alerta"; }
  else { cm.textContent = motor ? "Motores habilitados" : "Motores deshabilitados"; cm.className = "chip " + (motor ? "ok" : ""); }
}

/* ---------- Interfaz ---------- */
let tAviso;
function aviso(txt, error) {
  const a = $("aviso"); a.textContent = txt; a.className = "ver" + (error ? " error" : "");
  clearTimeout(tAviso); tAviso = setTimeout(() => a.className = "", 3500);
}

function tarjetas(m) {
  $("robot").setAttribute("transform", `rotate(${m.ang.toFixed(2)} 100 168)`);
  $("angGrande").textContent = m.ang.toFixed(1);
  $("mAng").textContent = m.ang.toFixed(1);  $("mSp").textContent = m.sp.toFixed(1);
  $("mErr").textContent = m.err.toFixed(1);  $("mOm").textContent = m.om.toFixed(1);
  $("mU").textContent = m.u.toFixed(0);
  $("mPl").textContent = m.pl;  $("mPr").textContent = m.pr;
  $("mRl").textContent = m.rl;  $("mRr").textContent = m.rr;
  $("mVm").textContent = m.vm.toFixed(3);  $("mOf").textContent = m.of.toFixed(2);
  mostrarPI();
  tarjetasEstado();
  $("chipHz").textContent = m.hz.toFixed(1) + " Hz";
}

function dibujar(canvas, filas, series) {
  const dpr = devicePixelRatio || 1, w = canvas.clientWidth, h = canvas.clientHeight;
  if (canvas.width !== Math.round(w * dpr)) { canvas.width = Math.round(w * dpr); canvas.height = Math.round(h * dpr); }
  const g = canvas.getContext("2d");
  g.setTransform(dpr, 0, 0, dpr, 0, 0); g.clearRect(0, 0, w, h);
  if (!filas.length) return;
  let lo = Infinity, hi = -Infinity;
  for (const f of filas) for (const s of series) { lo = Math.min(lo, f[s.k]); hi = Math.max(hi, f[s.k]); }
  if (hi - lo < 1) { const c = (hi + lo) / 2; lo = c - 0.5; hi = c + 0.5; }
  const pad = (hi - lo) * 0.1; lo -= pad; hi += pad;
  const L = 44, R = 8, T = 8, B = 18, span = VENTANA * DT, tFin = filas[filas.length - 1].t;
  const X = t => L + (t - (tFin - span)) / span * (w - L - R);
  const Y = v => T + (hi - v) / (hi - lo) * (h - T - B);
  g.font = "11px ui-monospace, Consolas, monospace"; g.textBaseline = "middle";
  for (let i = 0; i <= 4; i++) {
    const v = lo + (hi - lo) * i / 4, y = Y(v);
    g.strokeStyle = "#26323c"; g.lineWidth = 1; g.beginPath(); g.moveTo(L, y); g.lineTo(w - R, y); g.stroke();
    g.fillStyle = "#7f909d"; g.textAlign = "right"; g.fillText(v.toFixed(Math.abs(hi - lo) < 10 ? 1 : 0), L - 5, y);
  }
  g.textAlign = "left"; g.fillText("-" + span + " s", L, h - 7); g.textAlign = "right"; g.fillText("ahora", w - R, h - 7);
  for (const s of series) {
    g.strokeStyle = s.c; g.lineWidth = 1.6; g.beginPath();
    filas.forEach((f, i) => i ? g.lineTo(X(f.t), Y(f[s.k])) : g.moveTo(X(f.t), Y(f[s.k])));
    g.stroke();
  }
}

const GRAFICAS = [
  ["g1", [{k: "ang", c: "#3fd0c9"}, {k: "sp", c: "#f2b84b"}]],
  ["g2", [{k: "err", c: "#ff7a7a"}]],
  ["g3", [{k: "u", c: "#b49cff"}]],
  ["g4", [{k: "pl", c: "#5fa8ff"}, {k: "pr", c: "#ffa95f"}]],
  ["g5", [{k: "rl", c: "#5fa8ff"}, {k: "rr", c: "#ffa95f"}]],
  ["g6", [{k: "vm", c: "#6ee07a"}, {k: "vr", c: "#f2b84b"}]],
  ["g7", [{k: "of", c: "#b49cff"}]],
];
function graficas() {
  const fin = pausado ? finPausa : reg.length;
  const filas = reg.slice(Math.max(0, fin - VENTANA), fin);
  for (const [id, s] of GRAFICAS) dibujar($(id), filas, s);
}

/* ---------- Formulario y comandos ---------- */
function mostrarConf() { for (const k in conf) $("c_" + k).textContent = conf[k]; }
function llenarForm(v) { for (const k in LIMITES) $(k).value = v[k]; }
function leerForm(lim) {
  const v = {};
  for (const k in lim) {
    const n = parseFloat($(k).value), [a, b] = lim[k];
    if (!Number.isFinite(n) || n < a || n > b) {
      aviso(`${NOMBRES[k]} debe estar entre ${a} y ${b}`, true); $(k).focus(); return null;
    }
    v[k] = n;
  }
  return v;
}
async function aplicar(v) {
  const r = await enviar({tipo: "params", valores: v});
  if (r.ok) { mostrarConf(); aviso("Parámetros confirmados por el robot"); }
  else aviso(r.error || "El robot rechazó el comando", true);
}
// Envía el PI completo; lo que no se cambia se manda con el último valor confirmado.
async function aplicarPI(p, msg) {
  const r = await enviar({tipo: "vel", valores: {vkp: conf.vkp, vki: conf.vki, vref: vel.ref, vact: vel.act, ...p}});
  if (r.ok) aviso(msg || "PI confirmado por el robot");
  else aviso(r.error || "El robot rechazó el comando", true);
}
async function motores(on) {
  const r = await enviar({tipo: "motor", on});
  if (!r.ok) aviso(r.error, true);
  else if (on) aviso("Cuenta regresiva: sostén el robot vertical y suéltalo al llegar a 0");
}
function descargar(nombre, texto, tipo) {
  const a = document.createElement("a");
  a.href = URL.createObjectURL(new Blob([texto], {type: tipo})); a.download = nombre; a.click();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
}

$("bAplicar").onclick = () => { const v = leerForm(LIM_PID); if (v) aplicar(v); };
$("bAplicarPI").onclick = () => { const v = leerForm(LIM_PI); if (v) aplicarPI(v, "Ganancias del PI confirmadas"); };
$("bPIon").onclick = () => aplicarPI({vact: true}, "PI activado (referencia solo con motores habilitados)");
$("bPIoff").onclick = () => aplicarPI({vact: false}, "PI desactivado");
$("bRef").onclick = () => {
  const n = parseFloat($("vref").value);
  if (!Number.isFinite(n) || Math.abs(n) > LIM_REF) { aviso(`La referencia debe estar entre -${LIM_REF} y ${LIM_REF} m/s`, true); $("vref").focus(); return; }
  if (!vel.act) { aviso("Activa el PI antes de enviar una referencia", true); return; }
  if (!motor) { aviso("Habilita los motores antes de enviar una referencia", true); return; }
  aplicarPI({vref: Math.round(n * 100) / 100}, "Referencia confirmada por el robot");
};
$("bRef0").onclick = () => { $("vref").value = 0; aplicarPI({vref: 0}, "Referencia en 0"); };
$("bHab").onclick = () => motores(true);
$("bDes").onclick = () => motores(false);
$("bParo").onclick = async () => { const r = await enviar({tipo: "paro"}); aviso(r.ok ? "Parada de emergencia: motores deshabilitados" : r.error, true); };
$("bExpCfg").onclick = () => descargar("balancin-config.json", JSON.stringify(conf, null, 2), "application/json");
$("bImpCfg").onclick = () => $("fileCfg").click();
$("fileCfg").onchange = async e => {
  const f = e.target.files[0]; e.target.value = ""; if (!f) return;
  try {
    const d = JSON.parse(await f.text());
    for (const k in LIM_PID) if (!(d[k] >= LIM_PID[k][0] && d[k] <= LIM_PID[k][1])) throw new Error(k);
    for (const k in LIM_PI) if (k in d && !(d[k] >= LIM_PI[k][0] && d[k] <= LIM_PI[k][1])) throw new Error(k);
    llenarForm(d); aviso("Configuración cargada. Pulsa «Aplicar al robot» para enviarla.");
  } catch { aviso("El archivo no es una configuración válida", true); }
};
$("bRest").onclick = async () => {
  llenarForm(DEFECTO);
  const {vkp, vki, ...pid} = DEFECTO;
  await aplicar(pid); await aplicarPI({vkp, vki}, "Valores por defecto restaurados");
};
$("bPausa").onclick = () => {
  pausado = !pausado; finPausa = reg.length;
  $("bPausa").textContent = pausado ? "Reanudar" : "Pausar";
};
$("bLimpiar").onclick = () => { reg.length = 0; finPausa = 0; graficas(); };
$("bCsv").onclick = () => {
  const c = ["t_s", "angulo_deg", "setpoint_efectivo_deg", "error_deg", "vel_ang_dps", "salida_pid", "pwm_izq", "pwm_der", "rpm_izq", "rpm_der", "vel_ms", "ref_ms", "offset_deg"];
  const k = ["t", "ang", "sp", "err", "om", "u", "pl", "pr", "rl", "rr", "vm", "vr", "of"];
  const filas = reg.map(m => k.map(x => +m[x].toFixed(3)).join(","));
  descargar("balancin-registro.csv", [c.join(","), ...filas].join("\n"), "text/csv");
};

/* ---------- Arranque ---------- */
llenarForm(conf); mostrarConf(); mostrarPI();
conectar();
setInterval(() => {
  if (ws && ws.readyState === 1 && performance.now() - ultimoDato > 1500) { chip("chipRed", "WiFi: sin datos", "alerta"); enlace = false; tarjetasEstado(); }
  else if (ws && ws.readyState === 1) chip("chipRed", "WiFi: conectado", "ok");
}, 500);
addEventListener("resize", graficas);
