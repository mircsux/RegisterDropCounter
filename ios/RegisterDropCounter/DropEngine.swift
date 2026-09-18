import Foundation

enum Denom: Int, CaseIterable, Codable {
    case penny, nickel, dime, quarter, pRoll, nRoll, dRoll, qRoll
    case one, two, five, ten, twenty, fifty, hundred

    static let count = 15

    var cents: Int {
        [1, 5, 10, 25, 50, 200, 500, 1000, 100, 200, 500, 1000, 2000, 5000, 10000][rawValue]
    }

    var label: String {
        ["0.01", "0.05", "0.10", "0.25", "0.50", "2.00", "5.00", "10.00",
         "1", "2", "5", "10", "20", "50", "100"][rawValue]
    }

    var rollLetter: String? {
        [nil, nil, nil, nil, "P", "N", "D", "Q", nil, nil, nil, nil, nil, nil, nil][rawValue]
    }

    var slipName: String {
        ["Pennies", "Nickels", "Dimes", "Quarters",
         "P roll (0.50)", "N roll (2.00)", "D roll (5.00)", "Q roll (10.00)",
         "$1", "$2", "$5", "$10", "$20", "$50", "$100"][rawValue]
    }

    var kind: Kind {
        if rawValue <= 3 { return .coin }
        if rawValue <= 7 { return .roll }
        return .bill
    }

    enum Kind { case coin, roll, bill }

    static let dropOrder: [Denom] = [
        .hundred, .fifty, .twenty, .ten, .five, .two, .one,
        .quarter, .dime, .nickel, .qRoll, .dRoll, .nRoll, .pRoll, .penny,
    ]
}

struct Counts: Equatable, Codable, Hashable {
    var n: [Int]

    init() { n = Array(repeating: 0, count: Denom.count) }

    init(n: [Int]) {
        var v = n
        if v.count < Denom.count { v += Array(repeating: 0, count: Denom.count - v.count) }
        if v.count > Denom.count { v = Array(v.prefix(Denom.count)) }
        self.n = v.map { Counts.clamp($0) }
    }

    subscript(_ d: Denom) -> Int {
        get { n[d.rawValue] }
        set { n[d.rawValue] = Counts.clamp(newValue) }
    }

    var hasCount: Bool { n.contains { $0 > 0 } }

    static func clamp(_ v: Int) -> Int { min(99999, max(0, v)) }
}

struct RegisterResult {
    var counts = Counts()
    var drop = Counts()
    var left = Counts()
    var amountCents = 0
    var dropCents = 0
    var leftCents = 0
    var balanced = false
    var hasCount = false
}

enum DropEngine {
    static let registerCount = 10
    static let baseOptions = [100, 200, 300, 400, 500]

    static func compute(_ input: Counts, baseDollars: Int) -> RegisterResult {
        var r = RegisterResult()
        r.counts = Counts(n: input.n)
        let baseCents = baseDollars * 100
        for d in Denom.allCases {
            r.amountCents += r.counts[d] * d.cents
            if r.counts[d] > 0 { r.hasCount = true }
        }
        var remaining = max(0, r.amountCents - baseCents)
        for d in Denom.dropOrder {
            let need = remaining / d.cents
            let drop = max(0, min(r.counts[d], need))
            r.drop[d] = drop
            r.left[d] = r.counts[d] - drop
            remaining -= drop * d.cents
        }
        for d in Denom.allCases {
            r.dropCents += r.drop[d] * d.cents
            r.leftCents += r.left[d] * d.cents
        }
        r.balanced = r.leftCents == baseCents
        return r
    }

    static func money(_ cents: Int) -> String {
        let n = Double(cents) / 100.0
        let f = NumberFormatter()
        f.numberStyle = .currency
        f.currencyCode = "USD"
        f.maximumFractionDigits = 2
        f.minimumFractionDigits = 2
        return f.string(from: NSNumber(value: n)) ?? String(format: "$%.2f", n)
    }

    static func looseCoinCents(_ c: Counts) -> Int {
        c[.penny] * 1 + c[.nickel] * 5 + c[.dime] * 10 + c[.quarter] * 25
    }

    static func coinAndRollCents(_ c: Counts) -> Int {
        looseCoinCents(c)
            + c[.pRoll] * 50 + c[.nRoll] * 200 + c[.dRoll] * 500 + c[.qRoll] * 1000
    }

    static func billCents(_ c: Counts) -> Int {
        c[.one] * 100 + c[.two] * 200 + c[.five] * 500 + c[.ten] * 1000
            + c[.twenty] * 2000 + c[.fifty] * 5000 + c[.hundred] * 10000
    }

    static func oneCashLogTsv(_ r: RegisterResult, drop: Bool) -> String {
        let c = drop ? r.drop : r.counts
        let coin = drop ? looseCoinCents(c) : coinAndRollCents(c)
        return [
            "\(c[.one])", "\(c[.two])", "\(c[.five])", "\(c[.ten])",
            "\(c[.twenty])", "\(c[.fifty])", "\(c[.hundred])",
            String(format: "%.2f", Double(coin) / 100.0),
        ].joined(separator: "\t")
    }

    static let receiptCols = 42

    static func dropSlipText(
        till: String,
        baseDollars: Int,
        result: RegisterResult,
        bag: String,
        initials: String,
        now: Date = Date()
    ) -> String {
        func clip(_ s: String, _ n: Int) -> String {
            if s.count <= n { return s }
            return String(s.prefix(n))
        }
        func padR(_ s: String, _ n: Int) -> String {
            let t = clip(s, n)
            return t + String(repeating: " ", count: n - t.count)
        }
        func padL(_ s: String, _ n: Int) -> String {
            let t = clip(s, n)
            return String(repeating: " ", count: n - t.count) + t
        }
        func center(_ s: String) -> String {
            let t = clip(s, receiptCols)
            let left = (receiptCols - t.count) / 2
            return String(repeating: " ", count: left) + t
        }
        func rule() -> String { String(repeating: "-", count: receiptCols) }
        func kv(_ label: String, _ value: String) -> String {
            let v = value.isEmpty ? "________" : value
            let line = label + v
            if line.count <= receiptCols { return line }
            return label + "\n" + clip(v, receiptCols)
        }
        func itemRow(_ name: String, _ count: Int, _ cents: Int) -> String {
            padR(name, 22) + padL("\(count)", 6) + " " + padL(money(cents), 13)
        }
        let df = DateFormatter()
        df.locale = Locale(identifier: "en_US")
        df.dateFormat = "EEE, MMM d, yyyy"
        let tf = DateFormatter()
        tf.locale = Locale(identifier: "en_US")
        tf.dateFormat = "h:mm a"
        let bagV = bag.trimmingCharacters(in: .whitespacesAndNewlines)
        let initV = initials.trimmingCharacters(in: .whitespacesAndNewlines)
        let balanced: String
        if !result.hasCount { balanced = "Empty" }
        else if result.balanced { balanced = "Yes" }
        else { balanced = "No - off base" }
        var rows: [String] = []
        for d in Denom.allCases {
            let n = result.drop[d]
            if n > 0 { rows.append(itemRow(d.slipName, n, n * d.cents)) }
        }
        if rows.isEmpty { rows = ["(nothing to drop)"] }
        var out: [String] = [
            center("DROP SLIP"),
            rule(),
            df.string(from: now),
            tf.string(from: now),
            kv("Till: ", till),
            kv("Base: ", money(baseDollars * 100)),
            kv("Bag #: ", bagV),
            kv("Initials: ", initV),
            rule(),
            padR("DROP TOTAL", 29) + padL(money(result.dropCents), 13),
            padR("LEFT IN DRAWER", 29) + padL(money(result.leftCents), 13),
            rule(),
            padR("ITEM", 22) + padL("QTY", 6) + " " + padL("AMOUNT", 13),
        ]
        out.append(contentsOf: rows)
        out.append(rule())
        out.append(padR("DROP TOTAL", 29) + padL(money(result.dropCents), 13))
        out.append(padR("LEFT IN DRAWER", 29) + padL(money(result.leftCents), 13))
        out.append(padR("COUNTED", 29) + padL(money(result.amountCents), 13))
        out.append("Balanced: \(balanced)")
        out.append(rule())
        out.append(center("Star 80mm receipt"))
        return out.joined(separator: "\n")
    }
}

struct HistoryKind: Codable, Equatable {
    static let register = "register"
    static let all = "all"
}

struct HistoryEntry: Codable, Identifiable, Equatable {
    var id: String
    var at: TimeInterval
    var kind: String
    var registerIndex: Int?
    var base: Int
    var registers: [Counts]
}
