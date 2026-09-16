package com.ronaldrobbins.registerdropcounter

import android.app.Application
import android.content.Context
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import org.json.JSONArray
import org.json.JSONObject
import java.util.UUID

class AppModel(app: Application) : AndroidViewModel(app) {
    var base by mutableIntStateOf(400)
        private set
    var active by mutableIntStateOf(0)
        private set
    var bag by mutableStateOf("")
    var initials by mutableStateOf("")

    val registers = mutableStateListOf<Counts>().apply {
        repeat(DropEngine.REGISTER_COUNT) { add(Counts()) }
    }
    val names = mutableStateListOf<String>().apply {
        repeat(DropEngine.REGISTER_COUNT) { add("R${it + 1}") }
    }
    val history = mutableStateListOf<HistoryEntry>()

    init {
        load()
        DropEngineSelfTest.run()
    }

    fun result(i: Int): RegisterResult = DropEngine.compute(registers[i], base)

    fun setActive(i: Int) {
        active = i.coerceIn(0, DropEngine.REGISTER_COUNT - 1)
    }

    fun setBase(v: Int) {
        if (v in DropEngine.baseOptions) {
            base = v
            persistLive()
        }
    }

    fun setCount(register: Int, denom: Denom, value: Int) {
        val next = registers[register].copyOf()
        next[denom] = value
        registers[register] = next
        persistLive()
    }

    fun setName(register: Int, name: String) {
        val t = name.trim().replace(Regex("\\s+"), " ").take(20)
        names[register] = t.ifEmpty { "R${register + 1}" }
        persistNames()
    }

    fun clearRegister(i: Int) {
        snapshot(HistoryEntry.REGISTER, i)
        registers[i] = Counts()
        persistLive()
    }

    fun clearAll() {
        snapshot(HistoryEntry.ALL, null)
        for (i in 0 until DropEngine.REGISTER_COUNT) registers[i] = Counts()
        persistLive()
    }

    fun undoClear(): Boolean {
        val last = history.firstOrNull() ?: return false
        if (last.kind == HistoryEntry.REGISTER) {
            val i = last.registerIndex ?: return false
            if (i in last.registers.indices) {
                registers[i] = last.registers[i].copyOf()
                active = i
            }
        } else {
            base = last.base
            for (i in 0 until DropEngine.REGISTER_COUNT) {
                registers[i] = last.registers.getOrElse(i) { Counts() }.copyOf()
            }
        }
        persistLive()
        return true
    }

    fun restore(id: String): Boolean {
        val e = history.firstOrNull { it.id == id } ?: return false
        base = e.base
        for (i in 0 until DropEngine.REGISTER_COUNT) {
            registers[i] = e.registers.getOrElse(i) { Counts() }.copyOf()
        }
        persistLive()
        return true
    }

    fun clearHistory() {
        history.clear()
        persistHistory()
    }

    fun loadSample() {
        base = 400
        val sample = SampleData.registers()
        for (i in 0 until DropEngine.REGISTER_COUNT) registers[i] = sample[i]
        persistLive()
    }

    fun saveSlipFields() {
        prefs().edit().putString("bag", bag).putString("initials", initials).apply()
    }

    private fun snapshot(kind: String, index: Int?) {
        if (kind == HistoryEntry.REGISTER) {
            val i = index ?: return
            if (!registers[i].hasCount) return
        } else if (registers.none { it.hasCount }) {
            return
        }
        val e = HistoryEntry(
            id = UUID.randomUUID().toString(),
            at = System.currentTimeMillis(),
            kind = kind,
            registerIndex = if (kind == HistoryEntry.REGISTER) index else null,
            base = base,
            registers = registers.map { it.copyOf() },
        )
        history.add(0, e)
        while (history.size > 200) history.removeAt(history.lastIndex)
        persistHistory()
    }

    private fun prefs() = getApplication<Application>().getSharedPreferences("rdc", Context.MODE_PRIVATE)

    private fun persistLive() {
        val arr = JSONArray()
        registers.forEach { c ->
            val row = JSONArray()
            c.n.forEach { row.put(it) }
            arr.put(row)
        }
        prefs().edit().putInt("base", base).putString("registers", arr.toString()).apply()
    }

    private fun persistNames() {
        prefs().edit().putString("names", JSONArray(names.toList()).toString()).apply()
    }

    private fun persistHistory() {
        val arr = JSONArray()
        history.forEach { e ->
            val o = JSONObject()
            o.put("id", e.id)
            o.put("at", e.at)
            o.put("kind", e.kind)
            o.put("registerIndex", e.registerIndex ?: -1)
            o.put("base", e.base)
            val regs = JSONArray()
            e.registers.forEach { c ->
                val row = JSONArray()
                c.n.forEach { row.put(it) }
                regs.put(row)
            }
            o.put("registers", regs)
            arr.put(o)
        }
        prefs().edit().putString("history", arr.toString()).apply()
    }

    private fun load() {
        val p = prefs()
        val b = p.getInt("base", 400)
        if (b in DropEngine.baseOptions) base = b
        p.getString("registers", null)?.let { raw ->
            val arr = JSONArray(raw)
            for (i in 0 until DropEngine.REGISTER_COUNT) {
                val row = if (i < arr.length()) arr.getJSONArray(i) else JSONArray()
                val vals = List(Denom.entries.size) { di -> if (di < row.length()) row.optInt(di) else 0 }
                registers[i] = Counts.fromList(vals)
            }
        }
        p.getString("names", null)?.let { raw ->
            val arr = JSONArray(raw)
            for (i in 0 until DropEngine.REGISTER_COUNT) {
                val t = if (i < arr.length()) arr.optString(i).trim() else ""
                names[i] = t.ifEmpty { "R${i + 1}" }.take(20)
            }
        }
        p.getString("history", null)?.let { raw ->
            val arr = JSONArray(raw)
            history.clear()
            for (i in 0 until arr.length()) {
                val o = arr.getJSONObject(i)
                val regsArr = o.optJSONArray("registers") ?: JSONArray()
                val regs = MutableList(DropEngine.REGISTER_COUNT) { Counts() }
                for (r in 0 until DropEngine.REGISTER_COUNT) {
                    val row = if (r < regsArr.length()) regsArr.getJSONArray(r) else JSONArray()
                    val vals = List(Denom.entries.size) { di -> if (di < row.length()) row.optInt(di) else 0 }
                    regs[r] = Counts.fromList(vals)
                }
                val idx = o.optInt("registerIndex", -1)
                history.add(
                    HistoryEntry(
                        id = o.optString("id"),
                        at = o.optLong("at"),
                        kind = o.optString("kind"),
                        registerIndex = if (idx >= 0) idx else null,
                        base = o.optInt("base", 400),
                        registers = regs,
                    ),
                )
            }
        }
        bag = p.getString("bag", "") ?: ""
        initials = p.getString("initials", "") ?: ""
    }
}
