import SwiftUI
import UIKit

struct OptionsView: View {
    @EnvironmentObject var model: AppModel

    var body: some View {
        NavigationStack {
            Form {
                Section("Appearance") {
                    Toggle("Dark mode", isOn: Binding(
                        get: { model.darkMode },
                        set: { model.setDarkMode($0) }
                    ))
                    Text("Count cells stay gold so you can still find them. The rest of the sheet goes dark.")
                        .font(.caption)
                        .foregroundStyle(Theme.muted)
                }
                Section("Register base") {
                    Picker("Base", selection: $model.base) {
                        ForEach(DropEngine.baseOptions, id: \.self) { b in
                            Text(DropEngine.money(b * 100)).tag(b)
                        }
                    }
                }
                Section("Cash log") {
                    NavigationLink("Cash log tables") {
                        CashLogView()
                    }
                }
                Section("Data") {
                    Text("Counts, names, and history save on this iPhone. They also write JSON files you can share from the Files app.")
                        .font(.caption)
                        .foregroundStyle(Theme.muted)
                }
            }
            .navigationTitle("Options")
        }
    }
}

struct CashLogView: View {
    @EnvironmentObject var model: AppModel

    var body: some View {
        List {
            Section("Deposit (what you drop)") {
                ForEach(0..<DropEngine.registerCount, id: \.self) { i in
                    cashRow(i, drop: true)
                }
            }
            Section("EOD drawer") {
                ForEach(0..<DropEngine.registerCount, id: \.self) { i in
                    cashRow(i, drop: false)
                }
            }
        }
        .navigationTitle("Cash log")
    }

    private func cashRow(_ i: Int, drop: Bool) -> some View {
        let r = model.result(i)
        let text = DropEngine.oneCashLogTsv(r, drop: drop)
        return Button {
            UIPasteboard.general.string = text
        } label: {
            HStack {
                Text(model.names[i])
                Spacer()
                Text("Copy")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(Theme.navyFg)
            }
        }
    }
}
