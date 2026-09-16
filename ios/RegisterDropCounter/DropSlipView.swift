import SwiftUI
import UIKit

struct DropSlipView: View {
    @EnvironmentObject var model: AppModel
    let index: Int
    @Environment(\.dismiss) private var dismiss

    var result: RegisterResult { model.result(index) }

    var lines: [(String, Int, Int)] {
        Denom.allCases.compactMap { d in
            let n = result.drop[d]
            guard n > 0 else { return nil }
            return (d.slipName, n, n * d.cents)
        }
    }

    var body: some View {
        NavigationStack {
            Form {
                Section("Register Drop Slip") {
                    LabeledContent("Date", value: Date.now.formatted(date: .complete, time: .shortened))
                    LabeledContent("Register", value: model.names[index])
                    LabeledContent("Register base", value: DropEngine.money(model.base * 100))
                    LabeledContent("Balanced", value: result.hasCount ? (result.balanced ? "Yes" : "No - off base") : "Empty")
                }
                Section("Bag") {
                    TextField("Bag / seal #", text: $model.bag)
                    TextField("Initials", text: $model.initials)
                        .textInputAutocapitalization(.characters)
                }
                Section("Drop") {
                    if lines.isEmpty {
                        Text("Nothing to drop.")
                    } else {
                        ForEach(lines, id: \.0) { line in
                            HStack {
                                Text(line.0)
                                Spacer()
                                Text("\(line.1)").monospaced()
                                Text(DropEngine.money(line.2)).monospaced()
                            }
                        }
                    }
                    LabeledContent("Drop total", value: DropEngine.money(result.dropCents))
                    LabeledContent("Left in drawer", value: DropEngine.money(result.leftCents))
                    LabeledContent("Drawer counted", value: DropEngine.money(result.amountCents))
                }
            }
            .navigationTitle("Drop slip")
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Close") {
                        model.saveSlipFields()
                        dismiss()
                    }
                }
                ToolbarItem(placement: .primaryAction) {
                    Button("Print") { printSlip() }
                }
                ToolbarItem(placement: .topBarTrailing) {
                    Button("Copy") { copySlip() }
                }
            }
        }
    }

    private func slipText() -> String {
        var s = "REGISTER DROP SLIP\n"
        s += "Date: \(Date.now.formatted(date: .complete, time: .shortened))\n"
        s += "Register: \(model.names[index])\n"
        s += "Register base: \(DropEngine.money(model.base * 100))\n"
        s += "Bag / seal #: \(model.bag.isEmpty ? "________" : model.bag)\n"
        s += "Initials: \(model.initials.isEmpty ? "________" : model.initials)\n\n"
        for line in lines {
            s += "\(line.0)  \(line.1)  \(DropEngine.money(line.2))\n"
        }
        s += "\nDrop total: \(DropEngine.money(result.dropCents))\n"
        s += "Left in drawer: \(DropEngine.money(result.leftCents))\n"
        return s
    }

    private func copySlip() {
        UIPasteboard.general.string = slipText()
        model.saveSlipFields()
    }

    private func printSlip() {
        model.saveSlipFields()
        let info = UIPrintInfo.printInfo()
        info.jobName = "Register Drop Slip \(model.names[index])"
        info.outputType = .grayscale
        let ctrl = UIPrintInteractionController.shared
        ctrl.printInfo = info
        let fmt = UIMarkupTextPrintFormatter(markupText: "<pre>\(slipText())</pre>")
        ctrl.printFormatter = fmt
        ctrl.present(animated: true)
    }
}
