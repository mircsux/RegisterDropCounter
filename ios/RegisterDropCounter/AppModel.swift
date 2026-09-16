import Foundation
import SwiftUI
import Combine

@MainActor
final class AppModel: ObservableObject {
    @Published var base = 400
    @Published var registers: [Counts]
    @Published var names: [String]
    @Published var history: [HistoryEntry] = []
    @Published var active = 0
    @Published var bag = ""
    @Published var initials = ""

    static let version = "2.12.0"
    static let releaseDate = "2026-09-15"

    private let historyLimit = 200

    init() {
        registers = (0..<DropEngine.registerCount).map { _ in Counts() }
        names = (0..<DropEngine.registerCount).map { "R\($0 + 1)" }
        load()
        #if DEBUG
        DropEngineSelfTest.run()
        #endif
    }

    func result(_ i: Int) -> RegisterResult {
        DropEngine.compute(registers[i], baseDollars: base)
    }

    var results: [RegisterResult] {
        registers.map { DropEngine.compute($0, baseDollars: base) }
    }

    func setCount(register: Int, denom: Denom, value: Int) {
        registers[register][denom] = value
        persistLive()
    }

    func setName(register: Int, name: String) {
        let t = name.trimmingCharacters(in: .whitespacesAndNewlines)
        names[register] = t.isEmpty ? "R\(register + 1)" : String(t.prefix(20))
        persistNames()
    }

    func clearRegister(_ i: Int) {
        snapshot(kind: HistoryKind.register, index: i)
        registers[i] = Counts()
        persistLive()
    }

    func clearAll() {
        snapshot(kind: HistoryKind.all, index: nil)
        registers = (0..<DropEngine.registerCount).map { _ in Counts() }
        persistLive()
    }

    func undoClear() {
        guard let last = history.first else { return }
        if last.kind == HistoryKind.register, let i = last.registerIndex, i >= 0, i < last.registers.count {
            registers[i] = last.registers[i]
            active = i
        } else {
            base = last.base
            registers = last.registers
        }
        persistLive()
    }

    func restore(_ id: String) {
        guard let e = history.first(where: { $0.id == id }) else { return }
        base = e.base
        registers = e.registers
        persistLive()
    }

    func clearHistory() {
        history = []
        persistHistory()
    }

    func loadSample() {
        base = 400
        registers = SampleData.registers
        persistLive()
    }

    private func snapshot(kind: String, index: Int?) {
        if kind == HistoryKind.register {
            guard let i = index, registers[i].hasCount else { return }
        } else if !registers.contains(where: { $0.hasCount }) {
            return
        }
        let e = HistoryEntry(
            id: UUID().uuidString,
            at: Date().timeIntervalSince1970,
            kind: kind,
            registerIndex: kind == HistoryKind.register ? index : nil,
            base: base,
            registers: registers
        )
        history.insert(e, at: 0)
        if history.count > historyLimit { history = Array(history.prefix(historyLimit)) }
        persistHistory()
    }

    private func docs() -> URL {
        FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }

    private func persistLive() {
        let payload: [String: Any] = [
            "base": base,
            "registers": registers.map(\.n),
        ]
        UserDefaults.standard.set(payload, forKey: "rdc.live")
        try? JSONSerialization.data(withJSONObject: payload).write(to: docs().appendingPathComponent("RegisterDropCounter.state.json"))
    }

    private func persistNames() {
        UserDefaults.standard.set(names, forKey: "rdc.names")
    }

    private func persistHistory() {
        if let data = try? JSONEncoder().encode(history) {
            UserDefaults.standard.set(data, forKey: "rdc.history")
            try? data.write(to: docs().appendingPathComponent("RegisterDropCounter.history.json"))
        }
    }

    private func persistSlip() {
        UserDefaults.standard.set(bag, forKey: "rdc.bag")
        UserDefaults.standard.set(initials, forKey: "rdc.initials")
    }

    func saveSlipFields() { persistSlip() }

    private func load() {
        if let n = UserDefaults.standard.array(forKey: "rdc.names") as? [String] {
            names = (0..<DropEngine.registerCount).map { i in
                let t = i < n.count ? n[i].trimmingCharacters(in: .whitespaces) : ""
                return t.isEmpty ? "R\(i + 1)" : String(t.prefix(20))
            }
        }
        if let live = UserDefaults.standard.dictionary(forKey: "rdc.live") {
            if let b = live["base"] as? Int, DropEngine.baseOptions.contains(b) { base = b }
            if let rows = live["registers"] as? [[Int]] {
                registers = (0..<DropEngine.registerCount).map { i in
                    Counts(n: i < rows.count ? rows[i] : [])
                }
            }
        }
        if let data = UserDefaults.standard.data(forKey: "rdc.history"),
           let h = try? JSONDecoder().decode([HistoryEntry].self, from: data) {
            history = h
        }
        bag = UserDefaults.standard.string(forKey: "rdc.bag") ?? ""
        initials = UserDefaults.standard.string(forKey: "rdc.initials") ?? ""
    }
}

enum SampleData {
    static var registers: [Counts] {
        var out = (0..<DropEngine.registerCount).map { _ in Counts() }
        out[0] = Counts(n: [0, 6, 48, 27, 0, 0, 0, 0, 109, 0, 5, 10, 103, 6, 12])
        out[1] = Counts(n: [0, 51, 18, 16, 0, 0, 0, 0, 100, 0, 2, 1, 98, 3, 5])
        out[2] = Counts(n: [5, 63, 40, 0, 0, 0, 0, 0, 89, 0, 19, 25, 154, 2, 13])
        return out
    }
}

enum DropEngineSelfTest {
    static func run() {
        var c = Counts()
        c[.nickel] = 6; c[.dime] = 48; c[.quarter] = 27
        c[.one] = 109; c[.five] = 5; c[.ten] = 10
        c[.twenty] = 103; c[.fifty] = 6; c[.hundred] = 12
        let r = DropEngine.compute(c, baseDollars: 400)
        assert(r.amountCents == 380585)
        assert(r.dropCents == 340585)
        assert(r.leftCents == 40000)
        assert(r.balanced)
        assert(r.drop[.twenty] == 95)
        assert(r.drop[.hundred] == 12)
        assert(r.left[.one] == 109)
    }
}
