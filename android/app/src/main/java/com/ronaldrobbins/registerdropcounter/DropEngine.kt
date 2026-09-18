package com.ronaldrobbins.registerdropcounter

import java.text.NumberFormat
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import kotlin.math.abs

enum class DenomKind { Coin, Roll, Bill }

enum class Denom(val cents: Int, val label: String, val rollLetter: String?, val slipName: String) {
    Penny(1, "0.01", null, "Pennies"),
    Nickel(5, "0.05", null, "Nickels"),
    Dime(10, "0.10", null, "Dimes"),
    Quarter(25, "0.25", null, "Quarters"),
    PRoll(50, "0.50", "P", "P roll (0.50)"),
    NRoll(200, "2.00", "N", "N roll (2.00)"),
    DRoll(500, "5.00", "D", "D roll (5.00)"),
    QRoll(1000, "10.00", "Q", "Q roll (10.00)"),
    One(100, "1", null, "$1"),
    Two(200, "2", null, "$2"),
    Five(500, "5", null, "$5"),
    Ten(1000, "10", null, "$10"),
    Twenty(2000, "20", null, "$20"),
    Fifty(5000, "50", null, "$50"),
    Hundred(10000, "100", null, "$100");

    val kind: DenomKind
        get() = when {
            ordinal <= 3 -> DenomKind.Coin
            ordinal <= 7 -> DenomKind.Roll
            else -> DenomKind.Bill
        }

    companion object {
        val dropOrder: List<Denom> = listOf(
            Hundred, Fifty, Twenty, Ten, Five, Two, One,
            Quarter, Dime, Nickel, QRoll, DRoll, NRoll, PRoll, Penny,
        )
    }
}

data class Counts(val n: IntArray = IntArray(Denom.entries.size)) {
    operator fun get(d: Denom): Int = n[d.ordinal]
    operator fun set(d: Denom, value: Int) {
        n[d.ordinal] = clamp(value)
    }

    val hasCount: Boolean get() = n.any { it > 0 }

    fun copyOf(): Counts = Counts(n.copyOf())

    override fun equals(other: Any?): Boolean = other is Counts && n.contentEquals(other.n)
    override fun hashCode(): Int = n.contentHashCode()

    companion object {
        fun clamp(v: Int): Int = v.coerceIn(0, 99_999)
        fun fromList(values: List<Int>): Counts {
            val c = Counts()
            Denom.entries.forEachIndexed { i, _ ->
                c.n[i] = clamp(values.getOrElse(i) { 0 })
            }
            return c
        }
    }
}

data class RegisterResult(
    val counts: Counts = Counts(),
    val drop: Counts = Counts(),
    val left: Counts = Counts(),
    val amountCents: Int = 0,
    val dropCents: Int = 0,
    val leftCents: Int = 0,
    val balanced: Boolean = false,
    val hasCount: Boolean = false,
)

object DropEngine {
    const val REGISTER_COUNT = 10
    val baseOptions = listOf(100, 200, 300, 400, 500)

    fun compute(input: Counts, baseDollars: Int): RegisterResult {
        val counts = Counts(input.n.copyOf().also { arr -> arr.forEachIndexed { i, v -> arr[i] = Counts.clamp(v) } })
        val base = if (baseDollars in baseOptions) baseDollars else 400
        val baseCents = base * 100
        var amount = 0
        var has = false
        for (d in Denom.entries) {
            amount += counts[d] * d.cents
            if (counts[d] > 0) has = true
        }
        var remaining = (amount - baseCents).coerceAtLeast(0)
        val drop = Counts()
        val left = Counts()
        for (d in Denom.dropOrder) {
            val need = remaining / d.cents
            val take = counts[d].coerceAtMost(need).coerceAtLeast(0)
            drop[d] = take
            left[d] = counts[d] - take
            remaining -= take * d.cents
        }
        var dropCents = 0
        var leftCents = 0
        for (d in Denom.entries) {
            dropCents += drop[d] * d.cents
            leftCents += left[d] * d.cents
        }
        return RegisterResult(counts, drop, left, amount, dropCents, leftCents, leftCents == baseCents, has)
    }

    fun money(cents: Int): String {
        val nf = NumberFormat.getCurrencyInstance(Locale.US)
        nf.maximumFractionDigits = 2
        nf.minimumFractionDigits = 2
        return nf.format(cents / 100.0)
    }

    fun looseCoinCents(c: Counts): Int =
        c[Denom.Penny] + c[Denom.Nickel] * 5 + c[Denom.Dime] * 10 + c[Denom.Quarter] * 25

    fun coinAndRollCents(c: Counts): Int =
        looseCoinCents(c) + c[Denom.PRoll] * 50 + c[Denom.NRoll] * 200 +
            c[Denom.DRoll] * 500 + c[Denom.QRoll] * 1000

    fun billCents(c: Counts): Int =
        c[Denom.One] * 100 + c[Denom.Two] * 200 + c[Denom.Five] * 500 + c[Denom.Ten] * 1000 +
            c[Denom.Twenty] * 2000 + c[Denom.Fifty] * 5000 + c[Denom.Hundred] * 10000

    fun oneCashLogTsv(r: RegisterResult, drop: Boolean): String {
        val c = if (drop) r.drop else r.counts
        val coin = if (drop) looseCoinCents(c) else coinAndRollCents(c)
        return listOf(
            c[Denom.One], c[Denom.Two], c[Denom.Five], c[Denom.Ten],
            c[Denom.Twenty], c[Denom.Fifty], c[Denom.Hundred],
            String.format(Locale.US, "%.2f", coin / 100.0),
        ).joinToString("\t")
    }

    const val RECEIPT_COLS = 42

    fun dropSlipText(
        till: String,
        baseDollars: Int,
        r: RegisterResult,
        bag: String,
        initials: String,
        now: Date = Date(),
    ): String {
        fun clip(s: String, n: Int) = if (s.length <= n) s else s.substring(0, n)
        fun padR(s: String, n: Int): String {
            val t = clip(s, n)
            return t + " ".repeat(n - t.length)
        }
        fun padL(s: String, n: Int): String {
            val t = clip(s, n)
            return " ".repeat(n - t.length) + t
        }
        fun center(s: String): String {
            val t = clip(s, RECEIPT_COLS)
            val left = (RECEIPT_COLS - t.length) / 2
            return " ".repeat(left) + t
        }
        fun rule() = "-".repeat(RECEIPT_COLS)
        fun kv(label: String, value: String): String {
            val v = value.ifBlank { "________" }
            val line = label + v
            return if (line.length <= RECEIPT_COLS) line else label + "\n" + clip(v, RECEIPT_COLS)
        }
        fun itemRow(name: String, count: Int, cents: Int) =
            padR(name, 22) + padL(count.toString(), 6) + " " + padL(money(cents), 13)
        val df = SimpleDateFormat("EEE, MMM d, yyyy", Locale.US)
        val tf = SimpleDateFormat("h:mm a", Locale.US)
        val balanced = when {
            !r.hasCount -> "Empty"
            r.balanced -> "Yes"
            else -> "No - off base"
        }
        val rows = Denom.entries.mapNotNull { d ->
            val n = r.drop[d]
            if (n > 0) itemRow(d.slipName, n, n * d.cents) else null
        }.ifEmpty { listOf("(nothing to drop)") }
        return buildList {
            add(center("DROP SLIP"))
            add(rule())
            add(df.format(now))
            add(tf.format(now))
            add(kv("Till: ", till))
            add(kv("Base: ", money(baseDollars * 100)))
            add(kv("Bag #: ", bag.trim()))
            add(kv("Initials: ", initials.trim()))
            add(rule())
            add(padR("DROP TOTAL", 29) + padL(money(r.dropCents), 13))
            add(padR("LEFT IN DRAWER", 29) + padL(money(r.leftCents), 13))
            add(rule())
            add(padR("ITEM", 22) + padL("QTY", 6) + " " + padL("AMOUNT", 13))
            addAll(rows)
            add(rule())
            add(padR("DROP TOTAL", 29) + padL(money(r.dropCents), 13))
            add(padR("LEFT IN DRAWER", 29) + padL(money(r.leftCents), 13))
            add(padR("COUNTED", 29) + padL(money(r.amountCents), 13))
            add("Balanced: $balanced")
            add(rule())
            add(center("Star 80mm receipt"))
        }.joinToString("\n")
    }
}

data class HistoryEntry(
    val id: String,
    val at: Long,
    val kind: String,
    val registerIndex: Int?,
    val base: Int,
    val registers: List<Counts>,
) {
    companion object {
        const val REGISTER = "register"
        const val ALL = "all"
    }
}

object SampleData {
    fun registers(): List<Counts> {
        val out = MutableList(DropEngine.REGISTER_COUNT) { Counts() }
        out[0] = Counts.fromList(listOf(0, 6, 48, 27, 0, 0, 0, 0, 109, 0, 5, 10, 103, 6, 12))
        out[1] = Counts.fromList(listOf(0, 51, 18, 16, 0, 0, 0, 0, 100, 0, 2, 1, 98, 3, 5))
        out[2] = Counts.fromList(listOf(5, 63, 40, 0, 0, 0, 0, 0, 89, 0, 19, 25, 154, 2, 13))
        return out
    }

    fun randomRegisters(baseDollars: Int = 400): List<Counts> {
        return List(DropEngine.REGISTER_COUNT) {
            val c = Counts()
            for (d in Denom.entries) {
                c[d] = if (d.kind == DenomKind.Roll) (0..10).random() else (0..100).random()
            }
            capSampleDrop(c, baseDollars)
        }
    }

    private const val SAMPLE_DROP_CAP_CENTS = 600_000

    private fun capSampleDrop(counts: Counts, baseDollars: Int): Counts {
        val c = counts.copyOf()
        while (true) {
            val drop = DropEngine.compute(c, baseDollars).dropCents
            if (drop < SAMPLE_DROP_CAP_CENTS) return c
            val need = drop - (SAMPLE_DROP_CAP_CENTS - 1)
            var removed = false
            for (d in Denom.dropOrder) {
                if (c[d] <= 0) continue
                var take = need / d.cents
                if (need % d.cents != 0) take += 1
                take = minOf(c[d], take)
                if (take > 0) {
                    c[d] = c[d] - take
                    removed = true
                    break
                }
            }
            if (!removed) return c
        }
    }
}

object DropEngineSelfTest {
    fun run() {
        val c = Counts()
        c[Denom.Nickel] = 6; c[Denom.Dime] = 48; c[Denom.Quarter] = 27
        c[Denom.One] = 109; c[Denom.Five] = 5; c[Denom.Ten] = 10
        c[Denom.Twenty] = 103; c[Denom.Fifty] = 6; c[Denom.Hundred] = 12
        val r = DropEngine.compute(c, 400)
        check(r.amountCents == 380585)
        check(r.dropCents == 340585)
        check(r.leftCents == 40000)
        check(r.balanced)
        check(r.drop[Denom.Twenty] == 95)
        check(r.drop[Denom.Hundred] == 12)
        check(r.left[Denom.One] == 109)
        check(abs(r.amountCents) >= 0)
    }
}
