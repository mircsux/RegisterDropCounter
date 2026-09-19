/* Register Drop Counter 2.29.0 — same integer-cent drop math as Windows / iPhone / Android / C23. */
(function () {
  "use strict";

  var VERSION = "2.29.0";
  var RELEASE = "2026-09-19";
  var REGISTER_COUNT = 10;
  var HISTORY_LIMIT = 200;
  var TILL_NAME_MAX = 20;
  var BASE_OPTIONS = [100, 200, 300, 400, 500];
  var KEYS = [
    "penny", "nickel", "dime", "quarter",
    "pRoll", "nRoll", "dRoll", "qRoll",
    "one", "two", "five", "ten", "twenty", "fifty", "hundred",
  ];
  var DENOMS = [
    { key: "penny", label: "0.01", cents: 1, kind: "coin" },
    { key: "nickel", label: "0.05", cents: 5, kind: "coin" },
    { key: "dime", label: "0.10", cents: 10, kind: "coin" },
    { key: "quarter", label: "0.25", cents: 25, kind: "coin" },
    { key: "pRoll", label: "0.50", cents: 50, kind: "roll", rollLetter: "P" },
    { key: "nRoll", label: "2.00", cents: 200, kind: "roll", rollLetter: "N" },
    { key: "dRoll", label: "5.00", cents: 500, kind: "roll", rollLetter: "D" },
    { key: "qRoll", label: "10.00", cents: 1000, kind: "roll", rollLetter: "Q" },
    { key: "one", label: "1", cents: 100, kind: "bill" },
    { key: "two", label: "2", cents: 200, kind: "bill" },
    { key: "five", label: "5", cents: 500, kind: "bill" },
    { key: "ten", label: "10", cents: 1000, kind: "bill" },
    { key: "twenty", label: "20", cents: 2000, kind: "bill" },
    { key: "fifty", label: "50", cents: 5000, kind: "bill" },
    { key: "hundred", label: "100", cents: 10000, kind: "bill" },
  ];
  var DROP_ORDER = [
    "hundred", "fifty", "twenty", "ten", "five", "two", "one",
    "quarter", "dime", "nickel", "qRoll", "dRoll", "nRoll", "pRoll", "penny",
  ];
  var BILL_KEYS = ["one", "two", "five", "ten", "twenty", "fifty", "hundred"];
  var LOOSE = ["penny", "nickel", "dime", "quarter"];
  var ROLLS = ["pRoll", "nRoll", "dRoll", "qRoll"];
  var COIN_AND_ROLL = LOOSE.concat(ROLLS);
  var BY_KEY = {};
  DENOMS.forEach(function (d) { BY_KEY[d.key] = d; });

  function emptyCounts() {
    var c = {};
    KEYS.forEach(function (k) { c[k] = 0; });
    return c;
  }
  function emptyRegisters() {
    var a = [];
    for (var i = 0; i < REGISTER_COUNT; i++) a.push(emptyCounts());
    return a;
  }
  function parseCount(raw) {
    if (typeof raw === "number") {
      if (!isFinite(raw) || raw < 0) return 0;
      return Math.min(99999, Math.floor(raw));
    }
    var t = String(raw == null ? "" : raw).trim().replace(/,/g, "");
    if (!t) return 0;
    var n = Number(t);
    if (!isFinite(n) || n < 0) return 0;
    return Math.min(99999, Math.floor(n));
  }
  function computeRegister(countsIn, baseDollars) {
    var counts = emptyCounts();
    DENOMS.forEach(function (d) { counts[d.key] = parseCount(countsIn[d.key]); });
    var baseCents = Math.round(baseDollars * 100);
    var amountCents = 0;
    DENOMS.forEach(function (d) { amountCents += counts[d.key] * d.cents; });
    var remaining = Math.max(0, amountCents - baseCents);
    var drop = emptyCounts();
    var left = emptyCounts();
    DROP_ORDER.forEach(function (key) {
      var cents = BY_KEY[key].cents;
      var available = counts[key];
      var d = Math.max(0, Math.min(available, Math.floor(remaining / cents)));
      drop[key] = d;
      left[key] = available - d;
      remaining -= d * cents;
    });
    var dropCents = 0, leftCents = 0, hasCount = false;
    DENOMS.forEach(function (d) {
      dropCents += drop[d.key] * d.cents;
      leftCents += left[d.key] * d.cents;
      if (counts[d.key] > 0) hasCount = true;
    });
    return {
      counts: counts, drop: drop, left: left,
      amountCents: amountCents, dropCents: dropCents, leftCents: leftCents,
      undistributableCents: remaining,
      balanced: leftCents === baseCents, hasCount: hasCount,
    };
  }
  function centsOf(piece, keys) {
    var s = 0;
    keys.forEach(function (k) { s += piece[k] * BY_KEY[k].cents; });
    return s;
  }
  function formatMoney(cents) {
    var n = cents / 100;
    var parts = Math.abs(n).toFixed(2).split(".");
    var grouped = parts[0].replace(/\B(?=(\d{3})+(?!\d))/g, ",");
    return (n < 0 ? "-$" : "$") + grouped + "." + parts[1];
  }
  function formatPlain(cents) { return (cents / 100).toFixed(2); }
  function logRow(register, bills, coinCents, totalCents) {
    return {
      register: register,
      ones: bills.one, twos: bills.two, fives: bills.five, tens: bills.ten,
      twenties: bills.twenty, fifties: bills.fifty, hundreds: bills.hundred,
      coinCents: coinCents, totalCents: totalCents,
    };
  }
  function buildDeposit(i, r) {
    var coin = centsOf(r.drop, LOOSE);
    return logRow(i, r.drop, coin, centsOf(r.drop, BILL_KEYS) + coin);
  }
  function buildEod(i, r) {
    var coin = centsOf(r.counts, COIN_AND_ROLL);
    return logRow(i, r.counts, coin, centsOf(r.counts, BILL_KEYS) + coin);
  }
  function buildReset(i, r, base) {
    var baseCents = Math.round(base * 100);
    var leftoverBills = centsOf(r.left, BILL_KEYS);
    var coin = r.hasCount ? baseCents - leftoverBills : 0;
    return logRow(i, r.left, coin, r.leftCents);
  }
  function sumLog(rows) {
    var a = logRow(0, emptyCounts(), 0, 0);
    rows.forEach(function (r) {
      a.ones += r.ones; a.twos += r.twos; a.fives += r.fives; a.tens += r.tens;
      a.twenties += r.twenties; a.fifties += r.fifties; a.hundreds += r.hundreds;
      a.coinCents += r.coinCents; a.totalCents += r.totalCents;
    });
    return a;
  }
  function areaTsv(rows) {
    function line(label, r) {
      return [label, r.ones, r.twos, r.fives, r.tens, r.twenties, r.fifties, r.hundreds, formatPlain(r.coinCents), formatPlain(r.totalCents)].join("\t");
    }
    return rows.map(function (r) { return line("R" + r.register, r); }).concat([line("Totals", sumLog(rows))]).join("\r\n");
  }
  function oneTsv(r) {
    return [r.ones, r.twos, r.fives, r.tens, r.twenties, r.fifties, r.hundreds, formatPlain(r.coinCents)].join("\t");
  }
  function padR(s, n) { s = String(s); if (s.length > n) s = s.slice(0, n); return s + Array(n - s.length + 1).join(" "); }
  function padL(s, n) { s = String(s); if (s.length > n) s = s.slice(0, n); return Array(n - s.length + 1).join(" ") + s; }
  function center(s, n) { s = String(s); if (s.length > n) s = s.slice(0, n); var l = Math.floor((n - s.length) / 2); return Array(l + 1).join(" ") + s; }
  function denomSlipName(d) {
    if (d.kind === "bill") return "$" + d.label;
    if (d.kind === "roll") return d.rollLetter + " roll (" + d.label + ")";
    if (d.key === "penny") return "Pennies";
    if (d.key === "nickel") return "Nickels";
    if (d.key === "dime") return "Dimes";
    if (d.key === "quarter") return "Quarters";
    return d.label;
  }
  function dropSlipText(opts) {
    var now = opts.now || new Date();
    var date = now.toLocaleDateString("en-US", { weekday: "short", month: "short", day: "numeric", year: "numeric" });
    var time = now.toLocaleTimeString("en-US", { hour: "numeric", minute: "2-digit" });
    var till = (opts.till || "").trim() || ("R" + (opts.registerIndex + 1));
    var bag = (opts.bag || "").trim() || "________";
    var initials = (opts.initials || "").trim() || "________";
    var r = opts.result;
    var balanced = !r.hasCount ? "Empty" : r.balanced ? "Yes" : "No - off base";
    var rows = [];
    DENOMS.forEach(function (d) {
      var n = r.drop[d.key];
      if (n <= 0) return;
      rows.push(padR(denomSlipName(d), 22) + padL(String(n), 6) + " " + padL(formatMoney(n * d.cents), 13));
    });
    if (!rows.length) rows = ["(nothing to drop)"];
    function tot(label, cents) { return padR(label, 29) + padL(formatMoney(cents), 13); }
    return [
      center("DROP SLIP", 42), Array(43).join("-").slice(1), date, time,
      "Till: " + till, "Base: " + formatMoney(opts.base * 100), "Bag #: " + bag, "Initials: " + initials,
      Array(43).join("-").slice(1),
      tot("DROP TOTAL", r.dropCents), tot("LEFT IN DRAWER", r.leftCents),
      Array(43).join("-").slice(1),
      padR("ITEM", 22) + padL("QTY", 6) + " " + padL("AMOUNT", 13),
    ].concat(rows).concat([
      Array(43).join("-").slice(1),
      tot("DROP TOTAL", r.dropCents), tot("LEFT IN DRAWER", r.leftCents), tot("COUNTED", r.amountCents),
      "Balanced: " + balanced, Array(43).join("-").slice(1), center("Star 80mm receipt", 42),
    ]).join("\n");
  }

  var SAMPLE = [
    { nickel: 6, dime: 48, quarter: 27, one: 109, five: 5, ten: 10, twenty: 103, fifty: 6, hundred: 12 },
    { nickel: 51, dime: 18, quarter: 16, one: 100, five: 2, ten: 1, twenty: 98, fifty: 3, hundred: 5 },
    { penny: 5, nickel: 63, dime: 40, one: 89, five: 19, ten: 25, twenty: 154, fifty: 2, hundred: 13 },
  ];

  function cloneRegs(regs) { return regs.map(function (r) { return Object.assign(emptyCounts(), r); }); }
  function defaultNames() {
    var a = [];
    for (var i = 0; i < REGISTER_COUNT; i++) a.push("R" + (i + 1));
    return a;
  }
  function sanitizeName(raw, i) {
    var t = String(raw || "").replace(/\s+/g, " ").trim().slice(0, TILL_NAME_MAX);
    return t || ("R" + (i + 1));
  }
  function newId() {
    if (window.crypto && crypto.randomUUID) return crypto.randomUUID();
    return "h-" + Date.now() + "-" + Math.random().toString(16).slice(2);
  }
  function hasCount(c) { return KEYS.some(function (k) { return c[k] > 0; }); }
  function dateKey(ms) {
    var d = new Date(ms);
    return d.getFullYear() + "-" + String(d.getMonth() + 1).padStart(2, "0") + "-" + String(d.getDate()).padStart(2, "0");
  }
  function todayKey() { return dateKey(Date.now()); }

  function dropEvents(history) {
    var out = [];
    (history || []).forEach(function (entry) {
      var idxs = [];
      if (entry.kind === "register" && entry.registerIndex != null) idxs = [entry.registerIndex];
      else { for (var n = 0; n < REGISTER_COUNT; n++) idxs.push(n); }
      idxs.forEach(function (i) {
        var counts = entry.registers && entry.registers[i];
        if (!counts) return;
        var r = computeRegister(counts, entry.base);
        if (!r.hasCount || r.dropCents <= 0) return;
        out.push({ at: entry.at, registerIndex: i, dropCents: r.dropCents, drop: r.drop });
      });
    });
    out.sort(function (a, b) { return a.at - b.at; });
    return out;
  }

  function svgBars(items, color) {
    var max = 1;
    items.forEach(function (it) { if (it.v > max) max = it.v; });
    var w = 640, h = 200, padL = 8, padR = 8, padB = 28, padT = 10;
    var n = Math.max(items.length, 1);
    var gap = 4;
    var bw = Math.max(6, (w - padL - padR) / n - gap);
    var s = '<svg viewBox="0 0 ' + w + " " + h + '" class="chart" role="img">';
    items.forEach(function (it, i) {
      var bh = ((h - padB - padT) * it.v) / max;
      var x = padL + i * ((w - padL - padR) / n);
      var y = h - padB - bh;
      s += '<rect x="' + x.toFixed(1) + '" y="' + y.toFixed(1) + '" width="' + bw.toFixed(1) + '" height="' + Math.max(0, bh).toFixed(1) + '" rx="3" fill="' + color + '"/>';
      s += '<text x="' + (x + bw / 2).toFixed(1) + '" y="' + (h - 8) + '" text-anchor="middle" font-size="10" fill="var(--muted)">' + escapeHtml(it.label) + "</text>";
    });
    return s + "</svg>";
  }

  function svgArea(items) {
    var max = 1;
    items.forEach(function (it) { if (it.v > max) max = it.v; });
    var w = 640, h = 200, padB = 28, padT = 12, padX = 12;
    var n = Math.max(items.length, 1);
    var pts = items.map(function (it, i) {
      var x = padX + (i * (w - padX * 2)) / Math.max(n - 1, 1);
      var y = padT + (h - padB - padT) * (1 - it.v / max);
      return x.toFixed(1) + "," + y.toFixed(1);
    });
    var last = items.length ? padX + ((n - 1) * (w - padX * 2)) / Math.max(n - 1, 1) : padX;
    var s = '<svg viewBox="0 0 ' + w + " " + h + '" class="chart" role="img">';
    s += '<polyline fill="none" stroke="var(--navy-mid)" stroke-width="2" points="' + pts.join(" ") + '"/>';
    s += '<polygon fill="var(--navy-mid)" fill-opacity="0.22" points="' + padX + "," + (h - padB) + " " + pts.join(" ") + " " + last + "," + (h - padB) + '"/>';
    items.forEach(function (it, i) {
      if (n > 10 && i !== 0 && i !== n - 1 && i % Math.ceil(n / 6) !== 0) return;
      var x = padX + (i * (w - padX * 2)) / Math.max(n - 1, 1);
      s += '<text x="' + x.toFixed(1) + '" y="' + (h - 8) + '" text-anchor="middle" font-size="10" fill="var(--muted)">' + escapeHtml(it.label) + "</text>";
    });
    return s + "</svg>";
  }

  function statsHtml() {
    var events = dropEvents(state.history);
    var h = '<article class="card panel stats-page"><header class="card-h" style="display:block;padding:20px 24px"><p style="margin:0;font-size:12px;opacity:.7">File</p><h2 style="margin:4px 0 0;font-size:24px">Stats for Nerds</h2><p style="margin:8px 0 0;opacity:.85">Drop is the cash pulled so each drawer resets to base. Sample fills stay out of History, so they stay out of these charts.</p></header>';
    if (!events.length) {
      return h + '<div class="body"><p class="muted">History is empty. Count a till, then Clear. Each real clear becomes a point on the charts.</p></div></article>';
    }
    var total = 0;
    var byTill = {};
    var byDay = {};
    var byWd = [0, 0, 0, 0, 0, 0, 0];
    var byDenom = {};
    events.forEach(function (e) {
      total += e.dropCents;
      byTill[e.registerIndex] = (byTill[e.registerIndex] || 0) + e.dropCents;
      var k = dateKey(e.at);
      byDay[k] = (byDay[k] || 0) + e.dropCents;
      byWd[new Date(e.at).getDay()] += e.dropCents;
      DENOMS.forEach(function (d) {
        var c = e.drop[d.key] * d.cents;
        if (c) byDenom[d.key] = (byDenom[d.key] || 0) + c;
      });
    });
    var avg = Math.round(total / events.length);
    var tillItems = Object.keys(byTill).map(function (i) {
      return { label: state.names[Number(i)] || ("R" + (Number(i) + 1)), v: byTill[i] };
    }).sort(function (a, b) { return b.v - a.v; });
    var dayKeys = Object.keys(byDay).sort();
    var dayItems = dayKeys.map(function (k) {
      var p = k.split("-");
      var d = new Date(Number(p[0]), Number(p[1]) - 1, Number(p[2]));
      return { label: d.toLocaleDateString("en-US", { month: "short", day: "numeric" }), v: byDay[k] };
    });
    var wd = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
    var wdItems = wd.map(function (label, i) { return { label: label, v: byWd[i] }; });
    var denomItems = DENOMS.map(function (d) {
      var label = d.kind === "bill" ? "$" + d.label : (d.rollLetter ? d.rollLetter + "-roll" : d.label);
      return { label: label, v: byDenom[d.key] || 0 };
    }).filter(function (it) { return it.v > 0; }).sort(function (a, b) { return b.v - a.v; }).slice(0, 8);
    var top = tillItems[0];
    var topDay = wdItems.slice().sort(function (a, b) { return b.v - a.v; })[0];
    h += '<div class="stat-tiles"><div><span>Drop in History</span><strong>' + formatMoney(total) + "</strong></div>";
    h += "<div><span>Clears</span><strong>" + events.length + "</strong></div>";
    h += "<div><span>Average drop</span><strong>" + formatMoney(avg) + "</strong></div>";
    h += "<div><span>Tills</span><strong>" + tillItems.length + "</strong></div></div>";
    h += '<div class="body">';
    if (top) {
      h += '<div class="block"><h3>What the drops say</h3><ul><li>' + escapeHtml(top.label) + " is the heavy till.</li>";
      if (topDay && topDay.v) h += "<li>" + topDay.label + " is the big day for cash pulled.</li>";
      h += "</ul></div>";
    }
    h += '<div class="charts">';
    h += '<section class="block"><h3>Drop by day</h3>' + svgArea(dayItems) + "</section>";
    h += '<section class="block"><h3>Drop by till</h3>' + svgBars(tillItems, "var(--navy)") + "</section>";
    h += '<section class="block"><h3>Weekday mix</h3>' + svgBars(wdItems, "var(--navy-mid)") + "</section>";
    h += '<section class="block"><h3>What you pull</h3>' + svgBars(denomItems, "var(--input-edge)") + "</section>";
    h += "</div></div></article>";
    return h;
  }
  function escapeHtml(s) {
    var map = {
      "&": "&" + "amp;",
      "<": "&" + "lt;",
      ">": "&" + "gt;",
      '"': "&" + "quot;",
      "'": "&#39;",
    };
    return String(s).replace(/[&<>"']/g, function (c) { return map[c]; });
  }

  var SKEY = "rdc:v1";
  var OKEY = "rdc:options:v1";
  var HKEY = "rdc:history:v1";
  var NKEY = "rdc:names:v1";

  var state = {
    base: 400,
    registers: emptyRegisters(),
    names: defaultNames(),
    history: [],
    darkMode: false,
    active: 0,
    sheet: "counter",
    confirmClear: false,
    editingTill: -1,
    slipOpen: -1,
    bag: "",
    initials: "",
    histDate: "",
    histSel: null,
    status: "",
    fileOpen: false,
    sampleScratch: false,
  };

  function loadJson(key, fallback) {
    try {
      var raw = localStorage.getItem(key);
      if (!raw) return fallback;
      return JSON.parse(raw);
    } catch (e) { return fallback; }
  }
  function save() {
    try {
      localStorage.setItem(SKEY, JSON.stringify({
        base: state.base,
        registers: state.registers,
        sampleScratch: !!state.sampleScratch,
      }));
      localStorage.setItem(OKEY, JSON.stringify({ darkMode: state.darkMode }));
      localStorage.setItem(HKEY, JSON.stringify(state.history));
      localStorage.setItem(NKEY, JSON.stringify(state.names));
    } catch (e) {}
  }
  function hydrate() {
    var snap = loadJson(SKEY, null);
    if (snap && typeof snap === "object") {
      if (BASE_OPTIONS.indexOf(snap.base) >= 0) state.base = snap.base;
      if (Array.isArray(snap.registers)) {
        state.registers = emptyRegisters().map(function (z, i) {
          return Object.assign(z, snap.registers[i] || {});
        });
      }
      state.sampleScratch = !!snap.sampleScratch;
    }
    var opt = loadJson(OKEY, {});
    state.darkMode = !!opt.darkMode;
    var hist = loadJson(HKEY, []);
    if (Array.isArray(hist)) state.history = hist.slice(0, HISTORY_LIMIT);
    var names = loadJson(NKEY, null);
    if (Array.isArray(names)) {
      state.names = defaultNames().map(function (n, i) { return sanitizeName(names[i], i); });
    }
    applyTheme();
  }
  function applyTheme() {
    document.documentElement.classList.toggle("dark", state.darkMode);
    document.documentElement.style.colorScheme = state.darkMode ? "dark" : "light";
  }

  function results() {
    return state.registers.map(function (c) { return computeRegister(c, state.base); });
  }
  function pushHistory(kind, registerIndex) {
    if (kind === "register" && !hasCount(state.registers[registerIndex])) return;
    if (kind === "all" && !state.registers.some(hasCount)) return;
    state.history.unshift({
      id: newId(), at: Date.now(), kind: kind,
      registerIndex: kind === "register" ? registerIndex : null,
      base: state.base, registers: cloneRegs(state.registers),
    });
    if (state.history.length > HISTORY_LIMIT) state.history.length = HISTORY_LIMIT;
  }

  function copyText(text) {
    if (navigator.clipboard && navigator.clipboard.writeText) {
      return navigator.clipboard.writeText(text).then(function () { return true; }).catch(function () { return false; });
    }
    var ta = document.createElement("textarea");
    ta.value = text;
    document.body.appendChild(ta);
    ta.select();
    var ok = false;
    try { ok = document.execCommand("copy"); } catch (e) {}
    document.body.removeChild(ta);
    return Promise.resolve(ok);
  }

  function encodeHistory(entries) {
    var lines = ["RDCH1", String(entries.length)];
    entries.forEach(function (e) {
      var kind = e.kind === "register" ? 0 : 1;
      var reg = e.kind === "register" ? (e.registerIndex == null ? -1 : e.registerIndex) : -1;
      lines.push(Math.floor((e.at > 1e12 ? e.at : e.at * 1000) / 1000) + " " + kind + " " + reg + " " + e.base);
      for (var r = 0; r < REGISTER_COUNT; r++) {
        var c = e.registers[r] || emptyCounts();
        lines.push(DENOMS.map(function (d) { return String(c[d.key] || 0); }).join(","));
      }
    });
    return lines.join("\n") + "\n";
  }

  var app = document.getElementById("app");
  var lastFocus = "c-0-penny";

  function stamp() {
    return new Date().toLocaleDateString("en-US", { weekday: "long", month: "long", day: "numeric", year: "numeric" });
  }

  function paint() {
    var rs = results();
    var counted = rs.filter(function (r) { return r.hasCount; }).length;
    var balanced = rs.filter(function (r) { return r.hasCount && r.balanced; }).length;
    var off = rs.filter(function (r) { return r.hasCount && !r.balanced; }).length;
    var html = "";
    html += '<div class="app">';
    html += headerHtml();
    html += tabsHtml();
    html += '<div class="wrap">';
    if (state.sheet === "options") html += optionsHtml();
    else if (state.sheet === "about") html += aboutHtml();
    else if (state.sheet === "history") html += historyHtml(rs);
    else if (state.sheet === "stats") html += statsHtml();
    else html += counterHtml(rs, counted, balanced, off);
    html += "</div>";
    if (state.sheet === "counter") html += padHtml();
    if (state.slipOpen >= 0) html += slipHtml(rs[state.slipOpen], state.slipOpen);
    html += "</div>";
    app.innerHTML = html;
    bind();
    var el = document.getElementById(lastFocus);
    if (el && state.sheet === "counter") { /* keep last id; don't steal focus on full paint unless till edit */ }
    if (state.editingTill >= 0) {
      var te = document.getElementById("till-" + state.editingTill);
      if (te) { te.focus(); te.select(); }
    }
  }

  function headerHtml() {
    var last = state.history[0];
    var undoLabel = "Undo clear";
    if (last && last.kind === "register") undoLabel = "Undo " + (state.names[last.registerIndex] || ("R" + (last.registerIndex + 1)));
    var h = '<header class="top"><div class="top-inner">';
    h += '<div><h1>Register Drop Counter<span class="ver">v' + VERSION + '</span></h1>';
    h += '<p class="sub">Count the drawer. The drop is calculated so each register resets to the base.</p></div>';
    h += '<label class="base-box">Register Base <select id="base">';
    BASE_OPTIONS.forEach(function (n) {
      h += '<option value="' + n + '"' + (n === state.base ? " selected" : "") + ">" + formatMoney(n * 100) + "</option>";
    });
    h += "</select></label>";
    h += '<div class="top-actions">';
    h += '<div class="file-menu">';
    h += '<button type="button" class="btn btn-ghost" data-act="file-toggle" aria-haspopup="menu" aria-expanded="' + (state.fileOpen ? "true" : "false") + '">File</button>';
    if (state.fileOpen) {
      h += '<div class="file-pop" role="menu"><button type="button" role="menuitem" data-act="sample">Load Sample Drops</button><button type="button" role="menuitem" data-act="stats">Stats for Nerds</button><div class="sep" role="separator"></div><button type="button" role="menuitem" data-act="options">Options</button><button type="button" role="menuitem" data-act="about">About</button></div>';
    }
    h += "</div>";
    if (state.confirmClear) {
      h += '<button type="button" class="btn btn-danger" data-act="clear-all-yes">Confirm clear</button>';
      h += '<button type="button" class="btn btn-ghost" data-act="clear-all-no">Cancel</button>';
    } else {
      h += '<button type="button" class="btn btn-ghost" data-act="clear-all">Clear all</button>';
    }
    h += '<button type="button" class="btn btn-ghost" data-act="undo"' + (last ? "" : " disabled") + ">" + escapeHtml(undoLabel) + "</button>";
    h += "</div></div></header>";
    return h;
  }

  function tabsHtml() {
    function tab(id, label) {
      var extra = "";
      if (id === "history" && state.history.length) extra = '<span class="badge">' + state.history.length + "</span>";
      return '<button type="button" class="tab" role="tab" data-sheet="' + id + '" aria-selected="' + (state.sheet === id) + '">' + label + extra + "</button>";
    }
    return '<div class="tabs" role="tablist">' +
      tab("counter", "Counter") + tab("history", "History") +
      '<span style="flex:1"></span>' +
      '<span class="stamp">' + escapeHtml(stamp()) + "</span></div>";
  }

  function counterHtml(rs, counted, balanced, off) {
    var h = '<div class="meta">';
    h += '<span class="chip chip-navy">' + escapeHtml(stamp()) + "</span>";
    h += '<span class="chip chip-paper">' + counted + " counted</span>";
    h += '<span class="chip chip-ok">' + balanced + " on base</span>";
    if (off) h += '<span class="chip chip-bad">' + off + " off base</span>";
    h += '<span class="hint">Yellow cells are counts. Tab, Enter, or arrow keys move to the next field. After $100 it wraps back to pennies on this till. On a phone, use the number pad.</span></div>';
    h += '<div class="reg-chips">';
    rs.forEach(function (r, i) {
      var cls = "reg-chip";
      if (i === state.active) cls += " on";
      else if (r.hasCount && r.balanced) cls += " ok";
      else if (r.hasCount) cls += " bad";
      h += '<button type="button" class="' + cls + '" data-act="active" data-i="' + i + '">' + escapeHtml(state.names[i]) + "</button>";
    });
    h += "</div>";
    h += '<div class="layout"><div class="regs">';
    rs.forEach(function (r, i) { h += registerCard(r, i); });
    h += "</div>" + cashLogHtml(rs) + "</div>";
    return h;
  }

  function registerCard(r, i) {
    var unused = !r.hasCount;
    var st = unused ? "Empty" : r.balanced ? "Balanced" : "Off base";
    var stCls = unused ? "" : r.balanced ? " ok" : " bad";
    var h = '<section class="card' + (i === state.active ? " show" : "") + '" aria-labelledby="reg-' + i + '">';
    h += '<header class="card-h"><h2>';
    if (state.editingTill === i) {
      h += '<input class="till-edit" id="till-' + i + '" maxlength="' + TILL_NAME_MAX + '" value="' + escapeHtml(state.names[i]) + '" data-till="' + i + '" />';
    } else {
      h += '<button type="button" class="till-btn" data-act="rename" data-i="' + i + '">' + escapeHtml(state.names[i]) + "</button>";
    }
    h += '</h2><div class="row">';
    h += '<button type="button" class="btn btn-ghost" data-act="slip" data-i="' + i + '">Drop slip</button>';
    h += '<button type="button" class="btn btn-ghost" data-act="clear-one" data-i="' + i + '"' + (unused ? " disabled" : "") + ">Clear</button>";
    h += '<span class="status' + stCls + '">' + st + "</span></div></header>";
    h += '<div class="table-wrap"><table class="sheet"><thead><tr>';
    h += "<th></th><th>Count</th><th>Denom</th><th>Amount</th><th>Drop</th><th>Left</th></tr></thead><tbody>";
    DENOMS.forEach(function (d) {
      var drop = r.drop[d.key];
      var amt = r.counts[d.key] * d.cents;
      var id = "c-" + i + "-" + d.key;
      h += "<tr>";
      h += '<td class="roll-lab">' + (d.key === "pRoll" ? '<span style="font-size:9px">ROLLS</span> ' : "") + (d.rollLetter || "") + "</td>";
      h += "<td><input class=\"count-in\" id=\"" + id + "\" inputmode=\"numeric\" autocomplete=\"off\" value=\"" + (r.counts[d.key] ? r.counts[d.key] : "") + "\" data-reg=\"" + i + "\" data-key=\"" + d.key + "\" aria-label=\"Register " + (i + 1) + " " + d.label + " count\"></td>";
      h += '<td class="denom">' + d.label + "</td>";
      h += '<td class="amt">' + (amt ? formatMoney(amt) : "$0.00") + "</td>";
      h += '<td class="drop' + (drop > 0 ? " hit" : "") + '">' + drop + "</td>";
      h += '<td class="leftc">' + r.left[d.key] + "</td>";
      h += "</tr>";
    });
    var leftCls = unused ? " empty" : r.balanced ? " ok" : " bad";
    h += '</tbody><tfoot class="foot"><tr><td colspan="3">Total</td><td>' + formatMoney(r.amountCents) + "</td><td>" + formatMoney(r.dropCents) + '</td><td class="left-tot' + leftCls + '">' + formatMoney(r.leftCents) + "</td></tr></tfoot></table></div></section>";
    return h;
  }

  function cashLogHtml(rs) {
    var dep = rs.map(function (r, i) { return buildDeposit(i + 1, r); });
    var eod = rs.map(function (r, i) { return buildEod(i + 1, r); });
    var rst = rs.map(function (r, i) { return buildReset(i + 1, r, state.base); });
    var h = '<aside class="card log"><div class="card-h"><h2>Cash Log Information</h2><div class="row">';
    h += '<button type="button" class="btn btn-light" data-copy="' + encodeURIComponent(areaTsv(dep)) + '">Copy Cash Log</button>';
    h += '<button type="button" class="btn btn-light" data-copy="' + encodeURIComponent(areaTsv(eod)) + '">Copy EOD</button>';
    h += '<button type="button" class="btn btn-light" data-copy="' + encodeURIComponent(areaTsv(rst)) + '">Copy Reset</button>';
    h += "</div></div>";
    h += logTable("Deposit Tab", "Coin", dep, null, areaTsv(dep));
    h += logTable("EOD Drawer Count", "Coins", eod, rs, areaTsv(eod));
    h += logTable("Drawer Reset Count", "Coins", rst, rs, areaTsv(rst));
    h += "</aside>";
    return h;
  }
  function logTable(title, coinLabel, rows, tones, tsv) {
    var tot = sumLog(rows);
    var h = "<section><div style=\"display:flex;justify-content:space-between;gap:8px;align-items:center\"><h3>" + title + "</h3>";
    h += '<button type="button" class="btn btn-navy" style="height:28px;font-size:11px" data-copy="' + encodeURIComponent(oneTsv(tot)) + '">Copy Totals</button></div>';
    h += '<p class="note">Copy copies every cell in this table. R1–R10 copy that register row.</p>';
    h += "<div class=\"table-wrap\"><table><thead><tr><th></th><th>$1</th><th>$2</th><th>$5</th><th>$10</th><th>$20</th><th>$50</th><th>$100</th><th>" + coinLabel + "</th><th>Total</th></tr></thead><tbody>";
    rows.forEach(function (r, i) {
      var used = r.totalCents !== 0;
      var cls = !used ? " empty" : "";
      if (tones && used && tones[i].hasCount) cls = tones[i].balanced ? " ok" : " bad";
      h += '<tr class="' + cls + '"><td class="lab"><button type="button" class="copy-mini" data-copy="' + encodeURIComponent(oneTsv(r)) + '">R' + r.register + "</button></td>";
      [r.ones, r.twos, r.fives, r.tens, r.twenties, r.fifties, r.hundreds, formatPlain(r.coinCents), formatMoney(r.totalCents)].forEach(function (c) {
        h += "<td>" + c + "</td>";
      });
      h += "</tr>";
    });
    h += '</tbody><tfoot><tr><td>Totals</td>';
    [tot.ones, tot.twos, tot.fives, tot.tens, tot.twenties, tot.fifties, tot.hundreds, formatPlain(tot.coinCents), formatMoney(tot.totalCents)].forEach(function (c) {
      h += "<td>" + c + "</td>";
    });
    h += "</tr></tfoot></table></div></section>";
    return h;
  }

  function optionsHtml() {
    return '<article class="card panel"><header class="card-h" style="display:block;padding:20px 24px"><p style="margin:0;font-size:12px;opacity:.7;text-transform:uppercase">Options</p><h2 style="margin:4px 0 0;font-size:24px">Options</h2></header><div class="body">' +
      '<div class="block"><h3>Appearance</h3><label class="check"><input type="checkbox" id="dark"' + (state.darkMode ? " checked" : "") + "> <span>Dark mode</span></label>" +
      '<p class="muted">Dark mode uses a night theme: teal chrome, carbon cards, and amber count cells. Drop slips still print black on white.</p></div>' +
      '<div class="block"><h3>History file</h3><p class="muted">Save snapshots as RegisterDropCounter.history — the same file Windows uses for OneDrive.</p>' +
      '<div class="tools" style="border:0;padding:8px 0"><button type="button" class="btn btn-navy" data-act="hist-save">Save history file</button>' +
      '<label class="btn btn-light" style="height:32px">Load history file<input type="file" id="hist-file" accept=".history,.txt,text/plain" hidden></label></div>' +
      (state.status ? '<p class="muted">' + escapeHtml(state.status) + "</p>" : "") +
      "</div></div></article>";
  }

  function aboutHtml() {
    return '<article class="card panel"><header class="card-h" style="display:block;padding:20px 24px"><p style="margin:0;font-size:12px;opacity:.7">Designed by Ronald Robbins Jr and SuperGrok</p><h2 style="margin:4px 0 0;font-size:24px">Register Drop Counter</h2><p style="margin:8px 0 0;opacity:.85">Version ' + VERSION + " (" + RELEASE + ")</p></header><div class='body'>" +
      '<p class="muted">Count each drawer, drop down to the register base, and copy tab-separated rows into the national cash log.</p>' +
      '<div class="block"><h3>Count a drawer</h3><p>Set the register base. Open a till (R1–R10). Tap the name to rename it. Type counts in the yellow cells. After $100, Tab wraps back to pennies on the same till. Amount, Drop, and Left fill in. Left turns green when it equals the base.</p></div>' +
      '<div class="block"><h3>The drop</h3><p>$100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies. Loose coins drop before rolls.</p></div>' +
      '<div class="block"><h3>Changelog</h3>' +
      "<p>v2.29.0  Options and About live under File.</p>" +
      "<p>v2.28.0  File > Stats for Nerds on the Windows desktop app.</p>" +
      "<p>v2.27.0  Current release on web, Windows, iPhone, Android, and C23.</p>" +
      "<p>v2.26.0  Dark mode is a night theme: carbon, teal, and amber.</p>" +
      "<p>v2.25.0  File > Stats for Nerds charts drop trends from History.</p>" +
      "<p>v2.24.0  Dark mode on desktop: gold count cells, readable amount columns, navy copy buttons.</p>" +
      "<p>v2.23.0  Counting bar keeps File, Clear all, and Undo. Downloads live on About in the hosted app.</p>" +
      "<p>v2.22.0  Tab after $100 wraps to pennies on the same till.</p>" +
      "<p>v2.21.0  Current release on web, Windows, iPhone, Android, and C23.</p>" +
      "<p>v2.20.0  Sample drops do not write History snapshots.</p>" +
      "<p>v2.19.0  Sample drops stay under $6,000 per till.</p>" +
      "<p>v2.18.0  File > Load Sample Drops fills every till with random test counts.</p>" +
      "<p>v2.17.0  Drop slip shows drop total and left in drawer at the top.</p>" +
      "<p>v2.16.0  Web project — unzip and open index.html in any browser.</p>" +
      "<p>v2.15.0  Dark mode polish — count and amount cells go dark.</p>" +
      "<p>v2.14.1  Visual Studio build — History till selection and till-name editor.</p>" +
      "<p>v2.14.0  Dark mode on the Options tab.</p>" +
      "<p>v2.13.0  Star TSC100 drop slip — 80 mm / 42-column receipt.</p>" +
      "<p>v2.12.0  Android app (Kotlin + Jetpack Compose).</p>" +
      "<p>v2.11.0  ISO C23 console app.</p>" +
      "<p>v2.10.0  iPhone app (SwiftUI).</p></div></div></article>";
  }

  function historyHtml() {
    var filtered = state.histDate ? state.history.filter(function (e) { return dateKey(e.at) === state.histDate; }) : state.history;
    var sel = filtered.filter(function (e) { return e.id === state.histSel; })[0] || filtered[0] || null;
    var h = '<div class="hist-layout"><section class="card hist-list"><header class="card-h"><h2>Clear history</h2></header>';
    h += '<div class="tools"><input type="date" id="hist-date" value="' + escapeHtml(state.histDate) + '">';
    h += '<button type="button" class="btn btn-light" data-act="hist-today">Today</button>';
    if (state.history.length) h += '<button type="button" class="btn btn-danger" data-act="hist-wipe">Clear history</button>';
    h += "</div>";
    if (!filtered.length) h += '<p class="body muted">History is empty. Use Clear on a register or Clear all after a count.</p>';
    filtered.forEach(function (e) {
      var label = e.kind === "all" ? "All registers" : (state.names[e.registerIndex] || ("R" + ((e.registerIndex || 0) + 1)));
      var when = new Date(e.at).toLocaleString();
      h += '<button type="button" class="hist-item' + (sel && sel.id === e.id ? " on" : "") + '" data-act="hist-sel" data-id="' + e.id + '"><strong>' + escapeHtml(label) + '</strong><div class="when">' + escapeHtml(when) + "</div></button>";
    });
    h += "</section>";
    h += '<section class="card" style="flex:1"><header class="card-h"><h2>Snapshot</h2></header><div class="body">';
    if (!sel) h += '<p class="muted">No snapshot selected.</p>';
    else {
      h += '<p>Base ' + formatMoney(sel.base * 100) + ". Restore puts these counts back on the counter.</p>";
      h += '<p><button type="button" class="btn btn-navy" data-act="hist-restore" data-id="' + sel.id + '">Restore</button></p>';
      h += '<div class="table-wrap"><table class="sheet"><thead><tr><th>Till</th><th>Counted</th><th>Drop</th><th>Left</th></tr></thead><tbody>';
      sel.registers.forEach(function (c, i) {
        var r = computeRegister(c, sel.base);
        h += "<tr><td>" + escapeHtml(state.names[i]) + "</td><td class=\"amt\">" + formatMoney(r.amountCents) + "</td><td class=\"amt\">" + formatMoney(r.dropCents) + "</td><td class=\"amt\">" + formatMoney(r.leftCents) + "</td></tr>";
      });
      h += "</tbody></table></div>";
    }
    h += "</div></section></div>";
    return h;
  }

  function padHtml() {
    var keys = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "Clear", "0", "Enter"];
    var h = '<div class="pad" role="group" aria-label="Count number pad"><div class="pad-grid">';
    keys.forEach(function (k) {
      var cls = k === "Clear" ? "clear" : k === "Enter" ? "enter" : "";
      h += '<button type="button" class="' + cls + '" data-pad="' + k + '">' + k + "</button>";
    });
    return h + "</div></div>";
  }

  function slipHtml(r, i) {
    var text = dropSlipText({
      registerIndex: i, result: r, base: state.base,
      bag: state.bag, initials: state.initials, till: state.names[i],
    });
    return '<div class="overlay" data-act="slip-close"><div class="dialog drop-slip-print" role="dialog">' +
      '<header class="card-h no-print" style="display:block;padding:16px 20px"><p style="margin:0;font-size:11px;opacity:.75">Star TSC100  ·  80 mm receipt</p><h2 style="margin:4px 0 0">Register Drop Slip</h2></header>' +
      '<div class="body"><div class="no-print" style="display:grid;grid-template-columns:1fr 1fr;gap:12px">' +
      '<label class="field">Bag / seal #<input id="slip-bag" value="' + escapeHtml(state.bag) + '"></label>' +
      '<label class="field">Initials<input id="slip-in" maxlength="8" value="' + escapeHtml(state.initials) + '"></label></div>' +
      '<div class="no-print" style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin:12px 0">' +
      '<div style="background:var(--navy);color:var(--on-navy);border-radius:6px;padding:8px 12px"><div style="font-size:11px;opacity:.75;text-transform:uppercase">Drop</div><div style="font-family:var(--mono);font-size:18px;font-weight:700">' + formatMoney(r.dropCents) + "</div></div>" +
      '<div class="' + (r.hasCount ? (r.balanced ? "ok" : "bad") : "") + '" style="background:' + (!r.hasCount ? "var(--computed)" : r.balanced ? "var(--ok)" : "var(--bad)") + ";color:" + (!r.hasCount ? "var(--muted)" : r.balanced ? "var(--ok-ink)" : "var(--bad-ink)") + ';border-radius:6px;padding:8px 12px"><div style="font-size:11px;opacity:.8;text-transform:uppercase">Left</div><div style="font-family:var(--mono);font-size:18px;font-weight:700">' + formatMoney(r.leftCents) + "</div></div></div>" +
      '<pre class="receipt">' + escapeHtml(text) + "</pre>" +
      '<div class="no-print tools" style="border:0"><button type="button" class="btn btn-navy" data-act="slip-print">Print</button>' +
      '<button type="button" class="btn btn-light" data-act="slip-close">Close</button></div></div></div></div>';
  }

  function nextCount(reg, key, dir) {
    var idx = KEYS.indexOf(key);
    idx += dir;
    if (idx >= KEYS.length) idx = 0;
    if (idx < 0) idx = KEYS.length - 1;
    return "c-" + reg + "-" + KEYS[idx];
  }

  function bind() {
    app.querySelectorAll("[data-sheet]").forEach(function (b) {
      b.addEventListener("click", function () { state.sheet = b.getAttribute("data-sheet"); state.confirmClear = false; paint(); });
    });
    var base = document.getElementById("base");
    if (base) base.addEventListener("change", function () {
      state.base = Number(base.value) || 400; save(); paint();
    });
    app.querySelectorAll("[data-act]").forEach(function (b) {
      b.addEventListener("click", function (ev) {
        var act = b.getAttribute("data-act");
        var i = Number(b.getAttribute("data-i"));
        if (act === "slip-close" && ev.target !== b && !b.classList.contains("btn")) return;
        runAct(act, i, b.getAttribute("data-id"));
      });
    });
    app.querySelectorAll("[data-copy]").forEach(function (b) {
      b.addEventListener("click", function () {
        var t = decodeURIComponent(b.getAttribute("data-copy") || "");
        copyText(t).then(function (ok) {
          if (!ok) return;
          var old = b.textContent;
          b.textContent = "Copied";
          setTimeout(function () { b.textContent = old; }, 1200);
        });
      });
    });
    app.querySelectorAll(".count-in").forEach(function (inp) {
      inp.addEventListener("focus", function () { inp.select(); lastFocus = inp.id; state.active = Number(inp.getAttribute("data-reg")); });
      inp.addEventListener("input", function () {
        var v = inp.value.replace(/[^\d]/g, "").slice(0, 5);
        inp.value = v;
        var reg = Number(inp.getAttribute("data-reg"));
        var key = inp.getAttribute("data-key");
        state.registers[reg][key] = parseCount(v);
        lastFocus = inp.id;
        save();
        paint();
        var again = document.getElementById(lastFocus);
        if (again) { again.focus(); again.value = v; again.setSelectionRange(v.length, v.length); }
      });
      inp.addEventListener("keydown", function (e) {
        var dir = 0;
        if (e.key === "Enter" || e.key === "Tab" || e.key === "ArrowDown") dir = e.shiftKey ? -1 : 1;
        else if (e.key === "ArrowUp") dir = -1;
        if (!dir) return;
        e.preventDefault();
        var reg = Number(inp.getAttribute("data-reg"));
        var key = inp.getAttribute("data-key");
        state.registers[reg][key] = parseCount(inp.value);
        save();
        lastFocus = nextCount(reg, key, dir);
        state.active = Number(lastFocus.split("-")[1]);
        paint();
        var n = document.getElementById(lastFocus);
        if (n) { n.focus(); n.select(); }
      });
    });
    var till = app.querySelector(".till-edit");
    if (till) {
      till.addEventListener("blur", commitTill);
      till.addEventListener("keydown", function (e) {
        if (e.key === "Enter" || e.key === "Tab") { e.preventDefault(); commitTill(); }
        if (e.key === "Escape") { state.editingTill = -1; paint(); }
      });
    }
    var dark = document.getElementById("dark");
    if (dark) dark.addEventListener("change", function () {
      state.darkMode = dark.checked; applyTheme(); save();
    });
    var hd = document.getElementById("hist-date");
    if (hd) hd.addEventListener("change", function () { state.histDate = hd.value; paint(); });
    var hf = document.getElementById("hist-file");
    if (hf) hf.addEventListener("change", function () {
      var file = hf.files && hf.files[0];
      if (!file) return;
      var reader = new FileReader();
      reader.onload = function () {
        try {
          state.history = decodeHistory(String(reader.result || ""));
          state.status = "Loaded " + state.history.length + " snapshot(s).";
          save(); paint();
        } catch (err) {
          state.status = err.message || "Could not read that file.";
          paint();
        }
      };
      reader.readAsText(file);
    });
    var bag = document.getElementById("slip-bag");
    if (bag) bag.addEventListener("input", function () { state.bag = bag.value; });
    var ini = document.getElementById("slip-in");
    if (ini) ini.addEventListener("input", function () { state.initials = ini.value; });
    app.querySelectorAll("[data-pad]").forEach(function (b) {
      b.addEventListener("pointerdown", function (e) { e.preventDefault(); });
      b.addEventListener("click", function () { padPress(b.getAttribute("data-pad")); });
    });
  }

  function commitTill() {
    var till = document.querySelector(".till-edit");
    if (!till) { state.editingTill = -1; return; }
    var i = Number(till.getAttribute("data-till"));
    state.names[i] = sanitizeName(till.value, i);
    state.editingTill = -1;
    save();
    paint();
  }

  function padPress(k) {
    var el = document.activeElement;
    if (!(el instanceof HTMLInputElement) || !el.classList.contains("count-in")) {
      el = document.getElementById(lastFocus) || document.querySelector(".count-in");
      if (el) el.focus();
    }
    if (!(el instanceof HTMLInputElement)) return;
    var reg = Number(el.getAttribute("data-reg"));
    var key = el.getAttribute("data-key");
    if (k === "Clear") {
      el.value = "";
      state.registers[reg][key] = 0;
    } else if (k === "Enter") {
      state.registers[reg][key] = parseCount(el.value);
      lastFocus = nextCount(reg, key, 1);
      state.active = Number(lastFocus.split("-")[1]);
      save(); paint();
      var n = document.getElementById(lastFocus);
      if (n) { n.focus(); n.select(); }
      return;
    } else {
      var next = (el.value + k).replace(/[^\d]/g, "").slice(0, 5);
      el.value = next;
      state.registers[reg][key] = parseCount(next);
    }
    lastFocus = el.id;
    save(); paint();
    var again = document.getElementById(lastFocus);
    if (again) { again.focus(); again.value = el.value; }
  }

  function runAct(act, i, id) {
    if (act === "file-toggle") { state.fileOpen = !state.fileOpen; paint(); return; }
    if (act === "stats") { state.sheet = "stats"; state.fileOpen = false; paint(); return; }
    if (act === "options") { state.sheet = "options"; state.fileOpen = false; paint(); return; }
    if (act === "about") { state.sheet = "about"; state.fileOpen = false; paint(); return; }
    if (act === "sample") {
      var cap = 600000;
      state.registers = emptyRegisters();
      for (var ri = 0; ri < REGISTER_COUNT; ri++) {
        DENOMS.forEach(function (d) {
          var hi = d.kind === "roll" ? 10 : 100;
          state.registers[ri][d.key] = Math.floor(Math.random() * (hi + 1));
        });
        for (;;) {
          var drop = computeRegister(state.registers[ri], state.base).dropCents;
          if (drop < cap) break;
          var need = drop - (cap - 1);
          var removed = false;
          for (var oi = 0; oi < DROP_ORDER.length; oi++) {
            var key = DROP_ORDER[oi];
            if (state.registers[ri][key] <= 0) continue;
            var cents = BY_KEY[key].cents;
            var take = Math.min(state.registers[ri][key], Math.ceil(need / cents));
            if (take > 0) {
              state.registers[ri][key] -= take;
              removed = true;
              break;
            }
          }
          if (!removed) break;
        }
      }
      state.fileOpen = false;
      state.sampleScratch = true;
      save(); paint(); return;
    }
    if (act === "clear-all") { state.confirmClear = true; paint(); return; }
    if (act === "clear-all-no") { state.confirmClear = false; paint(); return; }
    if (act === "clear-all-yes") {
      if (!state.sampleScratch) pushHistory("all", null);
      state.registers = emptyRegisters();
      state.sampleScratch = false;
      state.confirmClear = false;
      save(); paint(); return;
    }
    if (act === "clear-one") {
      if (!state.sampleScratch) pushHistory("register", i);
      state.registers[i] = emptyCounts();
      save(); paint(); return;
    }
    if (act === "undo") {
      var last = state.history[0];
      if (!last) return;
      state.registers = cloneRegs(last.registers);
      state.base = last.base;
      state.history = state.history.slice(1);
      save(); paint(); return;
    }
    if (act === "active") { state.active = i; lastFocus = "c-" + i + "-penny"; paint(); return; }
    if (act === "rename") { state.editingTill = i; paint(); return; }
    if (act === "slip") { state.slipOpen = i; paint(); return; }
    if (act === "slip-close") { state.slipOpen = -1; paint(); return; }
    if (act === "slip-print") { window.print(); return; }
    if (act === "hist-today") { state.histDate = todayKey(); paint(); return; }
    if (act === "hist-sel") { state.histSel = id; paint(); return; }
    if (act === "hist-wipe") { state.history = []; save(); paint(); return; }
    if (act === "hist-restore") {
      var e = state.history.filter(function (x) { return x.id === id; })[0];
      if (!e) return;
      state.registers = cloneRegs(e.registers);
      state.base = e.base;
      state.sheet = "counter";
      save(); paint(); return;
    }
    if (act === "hist-save") {
      var blob = new Blob([encodeHistory(state.history)], { type: "text/plain;charset=utf-8" });
      var a = document.createElement("a");
      a.href = URL.createObjectURL(blob);
      a.download = "RegisterDropCounter.history";
      a.click();
      URL.revokeObjectURL(a.href);
      return;
    }
  }

  function decodeHistory(text) {
    var raw = text.replace(/^\uFEFF/, "").replace(/\r\n/g, "\n").trim();
    var lines = raw.split("\n");
    if (lines[0] !== "RDCH1") throw new Error("Not a Register Drop Counter history file.");
    var n = Math.min(HISTORY_LIMIT, Math.max(0, parseInt(lines[1] || "0", 10) || 0));
    var out = [];
    var i = 2;
    for (var e = 0; e < n; e++) {
      var header = (lines[i++] || "").trim().split(/\s+/);
      var at = Number(header[0]);
      if (!isFinite(at)) break;
      var kind = Number(header[1]) === 0 ? "register" : "all";
      var reg = Number(header[2]);
      var base = Number(header[3]);
      var registers = [];
      for (var r = 0; r < REGISTER_COUNT; r++) {
        var c = emptyCounts();
        var row = (lines[i++] || "").split(",");
        DENOMS.forEach(function (d, di) { c[d.key] = parseCount(row[di]); });
        registers.push(c);
      }
      out.push({
        id: newId(), at: at > 1e12 ? at : at * 1000, kind: kind,
        registerIndex: kind === "register" ? reg : null,
        base: BASE_OPTIONS.indexOf(base) >= 0 ? base : 400,
        registers: registers,
      });
    }
    return out;
  }

  hydrate();
  paint();

  var sample0 = computeRegister(Object.assign(emptyCounts(), SAMPLE[0]), 400);
  if (sample0.leftCents !== 40000) {
    console.warn("Drop math check failed", sample0.leftCents);
  }
})();
