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
                    LabeledContent("Drop total", value: DropEngine.money(result.dropCents))
                    LabeledContent("Left in drawer", value: DropEngine.money(result.leftCents))
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
        DropEngine.dropSlipText(
            till: model.names[index],
            baseDollars: model.base,
            result: result,
            bag: model.bag,
            initials: model.initials
        )
    }

    private func copySlip() {
        UIPasteboard.general.string = slipText()
        model.saveSlipFields()
    }

    private func printSlip() {
        model.saveSlipFields()
        let text = slipText()
        let info = UIPrintInfo.printInfo()
        info.jobName = "Drop Slip \(model.names[index])"
        info.outputType = .grayscale
        info.orientation = .portrait

        let fmt = UISimpleTextPrintFormatter(text: text)
        fmt.font = UIFont(name: "Menlo-Regular", size: 9)
            ?? UIFont.monospacedSystemFont(ofSize: 9, weight: .regular)
        fmt.color = .black
        fmt.perPageContentInsets = UIEdgeInsets(top: 8, left: 8, bottom: 31, right: 8)

        let renderer = UIPrintPageRenderer()
        renderer.addPrintFormatter(fmt, startingAtPageAt: 0)
        let paperW: CGFloat = 80.0 / 25.4 * 72.0
        let paperH: CGFloat = 200.0 / 25.4 * 72.0
        let paper = CGRect(x: 0, y: 0, width: paperW, height: paperH)
        let printable = CGRect(x: 8, y: 8, width: paperW - 16, height: paperH - 39)
        renderer.setValue(paper, forKey: "paperRect")
        renderer.setValue(printable, forKey: "printableRect")

        let ctrl = UIPrintInteractionController.shared
        ctrl.printInfo = info
        ctrl.printPageRenderer = renderer
        ctrl.present(animated: true)
    }
}
