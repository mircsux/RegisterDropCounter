/* Register Drop Counter web — same integer-cent math as Windows / Android / iOS / C23. */
(function () {
  "use strict";

  const VERSION = "2.15.0";
  const RELEASE = "2026-09-17";
  const REGISTER_COUNT = 10;
  const BASES = [100, 200, 300, 400, 500];
  const MAX_HISTORY = 200;

  const D = [
    { id: "Penny", cents: 1, label: "0.01", slip: "Pennies", kind: "coin" },
    { id: "Nickel", cents: 5, label: "0.05", slip: "Nickels", kind: "coin" },
    { id: "Dime", cents: 10, label: "0.10", slip: "Dimes", kind: "coin" },
    { id: "Quarter", cents: 25, label: "0.25", slip: "Quarters", kind: "coin" },
    { id: "PRoll", cents: 50, label: "0.50", slip: "P roll (0.50)", kind: "roll", letter: "P" },
    { id: "NRoll", cents: 200, label: "2.00", slip: "N roll (2.00)", kind: "roll", letter: "N" },
    { id: "DRoll", cents: 500, label: "5.00", slip: "D roll (5.00)", kind: "roll", letter: "D" },
    { id: "QRoll", cents: 1000, label: "10.00", slip: "Q roll (10.00)", kind: "roll", letter: "Q" },
    { id: "One", cents: 100, label: "1", slip: "$1", kind: "bill" },
    { id: "Two", cents: 200, label: "2", slip: "$2", kind: "bill" },
    { id: "Five", cents: 500, label: "5", slip: "$5", kind: "bill" },
    { id: "Ten", cents: 1000, label: "10", slip: "$10", kind: "bill" },
    { id: "Twenty", cents: 2000, label: "20", slip: "$20", kind: "bill" },
    { id: "Fifty", cents: 5000, label: "50", slip: "$50", kind: "bill" },
    { id: "Hundred", cents: 10000, label: "100", slip: "$100", kind: "bill" },
  ];
  const IDX = Object.fromEntries(D.map((d, i) => [d.id, i]));
  const DROP_ORDER = [
    "Hundred", "Fifty", "Twenty", "Ten", "Five", "Two", "One",
    "Quarter", "Dime", "Nickel", "QRoll", "DRoll", "NRoll", "PRoll", "Penny",
  ].map((id) => IDX[id]);

  function emptyCounts() {
    return new Array(D.length).fill(0);
  }
  function clamp(v) {
    v = parseInt(v, 10);
    if (!Number.isFinite(v) || v < 0) return 0;
    if (v > 99999) return 99999;
    return v;
  }
  function money(cents) {
    const n = Math.abs(cents);
    const s = "$" + Math.floor(n / 100) + "." + String(n % 100).padStart(2, "0");
    return cents < 0 ? "-" + s : s;
  }
  function plainMoney(cents) {
    const n = Math.abs(cents);
    const s = Math.floor(n / 100) + "." + String(n % 100).padStart(2, "0");
    return cents < 0 ? "-" + s : s;
  }
  function hasCount(c) {
    return c.some((n) => n > 0);
  }
  function compute(countsIn, baseDollars) {
    const counts = countsIn.map(clamp);
    const base = BASES.includes(baseDollars) ? baseDollars : 400;
    const baseCents = base * 100;
    let amount = 0;
    for (let i = 0; i < D.length; i++) amount += counts[i] * D[i].cents;
    let remaining = Math.max(0, amount - baseCents);
    const drop = emptyCounts();
    const left = emptyCounts();
    for (const i of DROP_ORDER) {
      const need = Math.floor(remaining / D[i].cents);
      const take = Math.max(0, Math.min(counts[i], need));
      drop[i] = take;
      left[i] = counts[i] - take;
      remaining -= take * D[i].cents;
    }
    let dropCents = 0;
    let leftCents = 0;
    for (let i = 0; i < D.length; i++) {
      dropCents += drop[i] * D[i].cents;
      leftCents += left[i] * D[i].cents;
    }
    return {
      counts, drop, left, amountCents: amount, dropCents, leftCents,
      balanced: leftCents === baseCents, hasCount: hasCount(counts),
    };
  }
  function looseCoin(c) {
    return c[IDX.Penny] + c[IDX.Nickel] * 5 + c[IDX.Dime] * 10 + c[IDX.Quarter] * 25;
  }
  function coinAndRoll(c) {
    return looseCoin(c) + c[IDX.PRoll] * 50 + c[IDX.NRoll] * 200 + c[IDX.DRoll] * 500 + c[IDX.QRoll] * 1000;
  }
  function billCents(c) {
    return c[IDX.One] * 100 + c[IDX.Two] * 200 + c[IDX.Five] * 500 + c[IDX.Ten] * 1000 +
      c[IDX.Twenty] * 2000 + c[IDX.Fifty] * 5000 + c[IDX.Hundred] * 10000;
  }
  function depositRow(r) {
    return {
      ones: r.drop[IDX.One], twos: r.drop[IDX.Two], fives: r.drop[IDX.Five], tens: r.drop[IDX.Ten],
      twenties: r.drop[IDX.Twenty], fifties: r.drop[IDX.Fifty], hundreds: r.drop[IDX.Hundred],
      coinCents: looseCoin(r.drop), totalCents: billCents(r.drop) + looseCoin(r.drop),
    };
  }
  function eodRow(r) {
    return {
      ones: r.counts[IDX.One], twos: r.counts[IDX.Two], fives: r.counts[IDX.Five], tens: r.counts[IDX.Ten],
      twenties: r.counts[IDX.Twenty], fifties: r.counts[IDX.Fifty], hundreds: r.counts[IDX.Hundred],
      coinCents: coinAndRoll(r.counts), totalCents: billCents(r.counts) + coinAndRoll(r.counts),
    };
  }
  function resetRow(r, baseDollars) {
    return {
      ones: r.left[IDX.One], twos: r.left[IDX.Two], fives: r.left[IDX.Five], tens: r.left[IDX.Ten],
      twenties: r.left[IDX.Twenty], fifties: r.left[IDX.Fifty], hundreds: r.left[IDX.Hundred],
      coinCents: r.hasCount ? baseDollars * 100 - billCents(r.left) : 0,
      totalCents: r.leftCents,
    };
  }
  function rowTsv(row) {
    return [row.ones, row.twos, row.fives, row.tens, row.twenties, row.fifties, row.hundreds, plainMoney(row.coinCents)].join("\t");
  }
  function areaRowTsv(label, row) {
    return [label, row.ones, row.twos, row.fives, row.tens, row.twenties, row.fifties, row.hundreds, plainMoney(row.coinCents), plainMoney(row.totalCents)].join("\t");
  }
  function sumRows(rows) {
    const t = { ones: 0, twos: 0, fives: 0, tens: 0, twenties: 0, fifties: 0, hundreds: 0, coinCents: 0, totalCents: 0 };
    for (const r of rows) {
      t.ones += r.ones; t.twos += r.twos; t.fives += r.fives; t.tens += r.tens;
      t.twenties += r.twenties; t.fifties += r.fifties; t.hundreds += r.hundreds;
      t.coinCents += r.coinCents; t.totalCents += r.totalCents;
    }
    return t;
  }
  function areaTsv(rows, names) {
    const lines = rows.map((r, i) => areaRowTsv(names[i] || ("R" + (i + 1)), r));
    lines.push(areaRowTsv("Totals", sumRows(rows)));
    return lines.join("\r\n");
  }
  function sampleRegisters() {
    const regs = Array.from({ length: REGISTER_COUNT }, emptyCounts);
    regs[0] = [0, 6, 48, 27, 0, 0, 0, 0, 109, 0, 5, 10, 103, 6, 12];
    regs[1] = [0, 51, 18, 16, 0, 0, 0, 0, 100, 0, 2, 1, 98, 3, 5];
    regs[2] = [5, 63, 40, 0, 0, 0, 0, 0, 89, 0, 19, 25, 154, 2, 13];
    return regs;
  }
  function padR(s, n) { s = String(s).slice(0, n); return s + " ".repeat(n - s.length); }
  function padL(s, n) { s = String(s).slice(0, n); return " ".repeat(n - s.length) + s; }
  function center(s, n) { s = String(s).slice(0, n); return " ".repeat(Math.floor((n - s.length) / 2)) + s; }
  function dropSlip(till, base, r, bag, initials) {
    const COLS = 42;
    const now = new Date();
    const date = now.toLocaleDateString("en-US", { weekday: "short", month: "short", day: "numeric", year: "numeric" });
    const time = now.toLocaleTimeString("en-US", { hour: "numeric", minute: "2-digit" });
    const bal = !r.hasCount ? "Empty" : r.balanced ? "Yes" : "No - off base";
    const lines = [
      center("DROP SLIP", COLS),
      "-".repeat(COLS),
      date, time,
      "Till: " + (till || "R1"),
      "Base: " + money(base * 100),
      "Bag #: " + (bag.trim() || "________"),
      "Initials: " + (initials.trim() || "________"),
      "-".repeat(COLS),
      padR("ITEM", 22) + padL("QTY", 6) + " " + padL("AMOUNT", 13),
    ];
    let any = false;
    for (let i = 0; i < D.length; i++) {
      if (r.drop[i] <= 0) continue;
      any = true;
      lines.push(padR(D[i].slip, 22) + padL(String(r.drop[i]), 6) + " " + padL(money(r.drop[i] * D[i].cents), 13));
    }
    if (!any) lines.push("(nothing to drop)");
    lines.push("-".repeat(COLS));
    lines.push(padR("DROP TOTAL", 29) + padL(money(r.dropCents), 13));
    lines.push(padR("LEFT IN DRAWER", 29) + padL(money(r.leftCents), 13));
    lines.push(padR("COUNTED", 29) + padL(money(r.amountCents), 13));
    lines.push("Balanced: " + bal);
    lines.push("-".repeat(COLS));
    lines.push(center("Star 80mm receipt", COLS));
    return lines.join("\n");
  }

  function selfTest() {
    const c = [0, 6, 48, 27, 0, 0, 0, 0, 109, 0, 5, 10, 103, 6, 12];
    const r = compute(c, 400);
    const ok =
      r.amountCents === 380585 &&
      r.dropCents === 340585 &&
      r.leftCents === 40000 &&
      r.balanced &&
      r.drop[IDX.Twenty] === 95 &&
      r.drop[IDX.Hundred] === 12 &&
      r.left[IDX.One] === 109;
    return ok;
  }

  const KEY = "rdc-web-v1";
  const state = {
    base: 400,
    active: 0,
    bag: "",
    initials: "",
    dark: false,
    registers: Array.from({ length: REGISTER_COUNT }, emptyCounts),
    names: Array.from({ length: REGISTER_COUNT }, (_, i) => "R" + (i + 1)),
    history: [],
  };

  function load() {
    try {
      const raw = localStorage.getItem(KEY);
      if (!raw) return;
      const s = JSON.parse(raw);
      if (BASES.includes(s.base)) state.base = s.base;
      if (Array.isArray(s.registers)) {
        for (let i = 0; i < REGISTER_COUNT; i++) {
          const row = s.registers[i] || [];
          state.registers[i] = D.map((_, di) => clamp(row[di] || 0));
        }
      }
      if (Array.isArray(s.names)) {
        for (let i = 0; i < REGISTER_COUNT; i++) {
          const t = String(s.names[i] || "").trim().slice(0, 20);
          state.names[i] = t || ("R" + (i + 1));
        }
      }
      if (Array.isArray(s.history)) state.history = s.history.slice(0, MAX_HISTORY);
      state.bag = String(s.bag || "");
      state.initials = String(s.initials || "");
      state.dark = !!s.dark;
      if (typeof s.active === "number") state.active = Math.max(0, Math.min(REGISTER_COUNT - 1, s.active));
    } catch (_) { /* keep defaults */ }
  }
  function save() {
    localStorage.setItem(KEY, JSON.stringify({
      base: state.base, active: state.active, bag: state.bag, initials: state.initials,
      dark: state.dark, registers: state.registers, names: state.names, history: state.history,
    }));
  }
  function snapshot(kind, index) {
    if (kind === "register") {
      if (!hasCount(state.registers[index])) return;
    } else if (!state.registers.some(hasCount)) return;
    state.history.unshift({
      id: crypto.randomUUID ? crypto.randomUUID() : String(Date.now()),
      at: Date.now(),
      kind,
      registerIndex: kind === "register" ? index : null,
      base: state.base,
      registers: state.registers.map((r) => r.slice()),
    });
    while (state.history.length > MAX_HISTORY) state.history.pop();
  }

  async function copyText(text) {
    try {
      await navigator.clipboard.writeText(text);
      toast("Copied");
    } catch (_) {
      const ta = document.createElement("textarea");
      ta.value = text;
      document.body.appendChild(ta);
      ta.select();
      document.execCommand("copy");
      ta.remove();
      toast("Copied");
    }
  }
  function toast(msg) {
    const el = document.getElementById("toast");
    el.textContent = msg;
    el.classList.add("show");
    clearTimeout(toast._t);
    toast._t = setTimeout(() => el.classList.remove("show"), 1400);
  }

  function results() {
    return state.registers.map((c) => compute(c, state.base));
  }

  function render() {
    document.documentElement.classList.toggle("dark", state.dark);
    document.getElementById("version").textContent = VERSION + "  (" + RELEASE + ")";
    document.getElementById("today").textContent = new Date().toLocaleDateString("en-US", { weekday: "short", month: "short", day: "numeric", year: "numeric" });
    const rs = results();
    const tabs = document.getElementById("tabs");
    tabs.innerHTML = "";
    for (let i = 0; i < REGISTER_COUNT; i++) {
      const b = document.createElement("button");
      b.className = "tab" + (i === state.active ? " on" : "") + (rs[i].hasCount ? (rs[i].balanced ? " ok" : " off") : "");
      b.textContent = state.names[i];
      b.title = "Click to select, double-click to rename";
      b.onclick = () => { state.active = i; save(); render(); };
      b.ondblclick = () => rename(i);
      tabs.appendChild(b);
    }
    document.getElementById("base").value = String(state.base);
    document.getElementById("bag").value = state.bag;
    document.getElementById("initials").value = state.initials;

    const i = state.active;
    const r = rs[i];
    document.getElementById("tillTitle").textContent = state.names[i];
    document.getElementById("counted").textContent = money(r.amountCents);
    document.getElementById("dropTot").textContent = money(r.dropCents);
    const leftEl = document.getElementById("leftTot");
    leftEl.textContent = money(r.leftCents);
    leftEl.className = "stat-val " + (!r.hasCount ? "" : r.balanced ? "good" : "bad");
    document.getElementById("balanced").textContent = !r.hasCount ? "Empty" : r.balanced ? "Yes" : "No — off base";
    document.getElementById("balanced").className = "stat-val " + (!r.hasCount ? "" : r.balanced ? "good" : "bad");

    const body = document.getElementById("gridBody");
    body.innerHTML = "";
    D.forEach((d, di) => {
      const tr = document.createElement("tr");
      tr.className = d.kind;
      tr.innerHTML =
        "<td class='lab'>" + (d.letter ? "<span class='roll'>" + d.letter + "</span> " : "") + d.label + "</td>" +
        "<td><input class='cnt' inputmode='numeric' data-di='" + di + "' value='" + (r.counts[di] || "") + "' placeholder='0'></td>" +
        "<td class='num drop'>" + (r.drop[di] ? r.drop[di] : "") + "</td>" +
        "<td class='num left'>" + (r.hasCount ? r.left[di] : "") + "</td>" +
        "<td class='num amt'>" + (r.counts[di] ? money(r.counts[di] * d.cents) : "") + "</td>";
      body.appendChild(tr);
    });
    body.querySelectorAll("input.cnt").forEach((inp) => {
      inp.addEventListener("change", onCount);
      inp.addEventListener("focus", () => inp.select());
      inp.addEventListener("keydown", navGrid);
    });

    const hist = document.getElementById("histList");
    const q = (document.getElementById("histQ").value || "").toLowerCase();
    hist.innerHTML = "";
    const items = state.history.filter((h) => {
      if (!q) return true;
      const dt = new Date(h.at).toLocaleString();
      return dt.toLowerCase().includes(q) || (h.kind || "").includes(q);
    });
    if (!items.length) {
      hist.innerHTML = "<p class='muted'>No snapshots yet. Clear a drawer to save one.</p>";
    } else {
      items.forEach((h) => {
        const row = document.createElement("div");
        row.className = "hist-row";
        const who = h.kind === "register" ? (state.names[h.registerIndex] || ("R" + (h.registerIndex + 1))) : "All tills";
        row.innerHTML = "<div><strong>" + who + "</strong> · base $" + h.base + "<br><span class='muted'>" + new Date(h.at).toLocaleString() + "</span></div>";
        const btn = document.createElement("button");
        btn.textContent = "Restore";
        btn.onclick = () => restore(h.id);
        row.appendChild(btn);
        hist.appendChild(row);
      });
    }
  }

  function onCount(e) {
    const di = parseInt(e.target.dataset.di, 10);
    state.registers[state.active][di] = clamp(e.target.value);
    save();
    render();
    const next = document.querySelector("input.cnt[data-di='" + di + "']");
    if (next) next.focus();
  }
  function navGrid(e) {
    const di = parseInt(e.target.dataset.di, 10);
    let n = di;
    if (e.key === "Enter" || e.key === "ArrowDown") n = Math.min(D.length - 1, di + 1);
    else if (e.key === "ArrowUp") n = Math.max(0, di - 1);
    else return;
    e.preventDefault();
    const el = document.querySelector("input.cnt[data-di='" + n + "']");
    if (el) { el.focus(); el.select(); }
  }
  function rename(i) {
    const next = prompt("Till name", state.names[i]);
    if (next == null) return;
    const t = next.trim().replace(/\s+/g, " ").slice(0, 20);
    state.names[i] = t || ("R" + (i + 1));
    save();
    render();
  }
  function restore(id) {
    const h = state.history.find((x) => x.id === id);
    if (!h) return;
    state.base = BASES.includes(h.base) ? h.base : state.base;
    state.registers = h.registers.map((r) => D.map((_, di) => clamp((r && r[di]) || 0)));
    save();
    render();
    toast("Restored");
  }

  function bind() {
    document.getElementById("base").onchange = (e) => {
      const v = parseInt(e.target.value, 10);
      if (BASES.includes(v)) { state.base = v; save(); render(); }
    };
    document.getElementById("bag").oninput = (e) => { state.bag = e.target.value; save(); };
    document.getElementById("initials").oninput = (e) => { state.initials = e.target.value; save(); };
    document.getElementById("dark").onclick = () => { state.dark = !state.dark; save(); render(); };
    document.getElementById("rename").onclick = () => rename(state.active);
    document.getElementById("clearOne").onclick = () => {
      snapshot("register", state.active);
      state.registers[state.active] = emptyCounts();
      save(); render(); toast("Cleared " + state.names[state.active]);
    };
    document.getElementById("clearAll").onclick = () => {
      if (!confirm("Clear all 10 tills?")) return;
      snapshot("all", null);
      state.registers = Array.from({ length: REGISTER_COUNT }, emptyCounts);
      save(); render(); toast("Cleared all");
    };
    document.getElementById("undo").onclick = () => {
      const last = state.history[0];
      if (!last) { toast("Nothing to undo"); return; }
      if (last.kind === "register" && last.registerIndex != null) {
        state.registers[last.registerIndex] = (last.registers[last.registerIndex] || emptyCounts()).slice();
        state.active = last.registerIndex;
      } else {
        state.base = BASES.includes(last.base) ? last.base : state.base;
        state.registers = last.registers.map((r) => D.map((_, di) => clamp((r && r[di]) || 0)));
      }
      save(); render(); toast("Undo clear");
    };
    document.getElementById("sample").onclick = () => {
      state.base = 400;
      state.registers = sampleRegisters();
      save(); render(); toast("Sample loaded");
    };
    document.getElementById("copyOne").onclick = () => {
      const r = compute(state.registers[state.active], state.base);
      copyText(rowTsv(depositRow(r)));
    };
    document.getElementById("copyLog").onclick = () => {
      copyText(areaTsv(results().map(depositRow), state.names));
    };
    document.getElementById("copyEod").onclick = () => {
      copyText(areaTsv(results().map(eodRow), state.names));
    };
    document.getElementById("copyReset").onclick = () => {
      copyText(areaTsv(results().map((r) => resetRow(r, state.base)), state.names));
    };
    document.getElementById("slip").onclick = () => {
      const r = compute(state.registers[state.active], state.base);
      const text = dropSlip(state.names[state.active], state.base, r, state.bag, state.initials);
      const w = window.open("", "_blank", "width=420,height=720");
      w.document.write("<pre style='font:14px/1.3 ui-monospace,Consolas,monospace;padding:16px;white-space:pre'>" +
        text.replace(/[&<>]/g, (c) => ({ "&": "&", "<": "<", ">": ">" }[c])) + "</pre>");
      w.document.title = "Drop slip — " + state.names[state.active];
      w.document.close();
      w.print();
    };
    document.getElementById("histQ").oninput = render;
    document.getElementById("clearHist").onclick = () => {
      if (!confirm("Erase history on this browser?")) return;
      state.history = [];
      save(); render();
    };
    document.getElementById("about").onclick = () => document.getElementById("aboutDlg").showModal();
  }

  load();
  bind();
  render();
  const test = selfTest();
  document.getElementById("test").textContent = test ? "Drop math OK" : "Drop math FAILED";
  document.getElementById("test").className = "pill " + (test ? "ok" : "bad");
})();
