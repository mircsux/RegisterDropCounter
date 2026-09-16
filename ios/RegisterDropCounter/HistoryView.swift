import SwiftUI

struct HistoryView: View {
    @EnvironmentObject var model: AppModel
    @State private var date = ""
    @State private var selected: String?

    var filtered: [HistoryEntry] {
        if date.isEmpty { return model.history }
        let f = DateFormatter()
        f.dateFormat = "yyyy-MM-dd"
        return model.history.filter { e in
            f.string(from: Date(timeIntervalSince1970: e.at)) == date
        }
    }

    var body: some View {
        NavigationStack {
            List {
                Section {
                    DatePicker("Search by date", selection: dateBinding, displayedComponents: .date)
                    Button("All dates") { date = "" }
                }
                if filtered.isEmpty {
                    Text("No snapshots. Clear a register on Counter and it will show up here.")
                        .foregroundStyle(Theme.muted)
                }
                ForEach(filtered) { e in
                    Button {
                        selected = e.id
                    } label: {
                        VStack(alignment: .leading, spacing: 4) {
                            Text(when(e.at))
                                .font(.subheadline.weight(.semibold))
                            Text(label(e))
                                .font(.caption)
                                .foregroundStyle(Theme.muted)
                        }
                    }
                }
            }
            .navigationTitle("History")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Clear history", role: .destructive) { model.clearHistory() }
                        .disabled(model.history.isEmpty)
                }
            }
            .sheet(item: selectedBinding) { e in
                SnapshotDetail(entry: e)
            }
        }
    }

    private var dateBinding: Binding<Date> {
        Binding(
            get: {
                let f = DateFormatter(); f.dateFormat = "yyyy-MM-dd"
                return f.date(from: date) ?? Date()
            },
            set: {
                let f = DateFormatter(); f.dateFormat = "yyyy-MM-dd"
                date = f.string(from: $0)
            }
        )
    }

    private var selectedBinding: Binding<HistoryEntry?> {
        Binding(
            get: { model.history.first { $0.id == selected } },
            set: { selected = $0?.id }
        )
    }

    private func when(_ t: TimeInterval) -> String {
        let d = Date(timeIntervalSince1970: t)
        return d.formatted(date: .abbreviated, time: .shortened)
    }

    private func label(_ e: HistoryEntry) -> String {
        if e.kind == HistoryKind.all { return "Cleared all registers" }
        let i = e.registerIndex ?? 0
        return "Cleared \(model.names.indices.contains(i) ? model.names[i] : "R\(i + 1)")"
    }
}

struct SnapshotDetail: View {
    @EnvironmentObject var model: AppModel
    let entry: HistoryEntry
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            List {
                ForEach(entry.registers.indices, id: \.self) { i in
                    let r = DropEngine.compute(entry.registers[i], baseDollars: entry.base)
                    HStack {
                        Text(model.names.indices.contains(i) ? model.names[i] : "R\(i + 1)")
                        Spacer()
                        Text(DropEngine.money(r.amountCents)).font(.caption.monospaced())
                        Text(r.hasCount ? (r.balanced ? "OK" : "Off") : "Empty")
                            .font(.caption2)
                            .foregroundStyle(r.hasCount ? (r.balanced ? Theme.okInk : Theme.badInk) : Theme.muted)
                    }
                }
            }
            .navigationTitle("Snapshot")
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Restore") {
                        model.restore(entry.id)
                        dismiss()
                    }
                }
                ToolbarItem(placement: .cancellationAction) {
                    Button("Close") { dismiss() }
                }
            }
        }
    }
}

extension HistoryEntry: Hashable {}
