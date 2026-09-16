import SwiftUI

struct CounterView: View {
    @EnvironmentObject var model: AppModel
    @State private var confirmClearAll = false
    @State private var slipOpen = false
    @FocusState private var focused: FocusKey?

    struct FocusKey: Hashable {
        var register: Int
        var denom: Denom
    }

    var body: some View {
        NavigationStack {
            ZStack(alignment: .bottom) {
                Theme.sheet.ignoresSafeArea()
                VStack(spacing: 0) {
                    header
                    chips
                    ScrollView {
                        RegisterCardView(index: model.active, focused: $focused)
                            .padding(.horizontal, 12)
                            .padding(.top, 8)
                            .padding(.bottom, 220)
                    }
                }
                NumberPadView(focused: $focused)
            }
            .navigationBarHidden(true)
            .sheet(isPresented: $slipOpen) {
                DropSlipView(index: model.active)
            }
            .onAppear {
                if focused == nil {
                    focused = FocusKey(register: model.active, denom: .penny)
                }
            }
        }
    }

    private var header: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Register Drop Counter")
                        .font(.headline)
                        .foregroundStyle(.white)
                    Text("v\(AppModel.version)")
                        .font(.caption.monospaced())
                        .foregroundStyle(.white.opacity(0.75))
                }
                Spacer()
                Picker("Base", selection: $model.base) {
                    ForEach(DropEngine.baseOptions, id: \.self) { b in
                        Text(DropEngine.money(b * 100)).tag(b)
                    }
                }
                .pickerStyle(.menu)
                .tint(.white)
            }
            HStack(spacing: 8) {
                Button("Sample") { model.loadSample() }
                    .buttonStyle(NavyChip())
                Button("Clear all") { confirmClearAll = true }
                    .buttonStyle(NavyChip())
                Button(undoLabel) { model.undoClear() }
                    .buttonStyle(NavyChip())
                    .disabled(model.history.isEmpty)
                Button("Drop slip") { slipOpen = true }
                    .buttonStyle(NavyChip())
            }
        }
        .padding(.horizontal, 12)
        .padding(.top, 12)
        .padding(.bottom, 10)
        .background(Theme.navy)
        .confirmationDialog("Clear all ten registers?", isPresented: $confirmClearAll, titleVisibility: .visible) {
            Button("Clear all", role: .destructive) { model.clearAll() }
            Button("Cancel", role: .cancel) {}
        }
    }

    private var undoLabel: String {
        guard let last = model.history.first else { return "Undo" }
        if last.kind == HistoryKind.register, let i = last.registerIndex {
            return "Undo \(model.names[i])"
        }
        return "Undo"
    }

    private var chips: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 6) {
                ForEach(0..<DropEngine.registerCount, id: \.self) { i in
                    let r = model.result(i)
                    Button {
                        model.active = i
                    } label: {
                        Text(model.names[i])
                            .font(.caption.weight(.semibold))
                            .padding(.horizontal, 12)
                            .padding(.vertical, 8)
                            .background(chipColor(i, r))
                            .foregroundStyle(model.active == i ? Color.white : Theme.ink)
                            .clipShape(RoundedRectangle(cornerRadius: 6))
                    }
                }
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 8)
        }
        .background(Theme.paper)
    }

    private func chipColor(_ i: Int, _ r: RegisterResult) -> Color {
        if model.active == i { return Theme.navy }
        if r.hasCount && r.balanced { return Theme.ok }
        if r.hasCount && !r.balanced { return Theme.bad }
        return Theme.sheet
    }
}

struct NavyChip: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.caption.weight(.semibold))
            .foregroundStyle(.white)
            .padding(.horizontal, 10)
            .padding(.vertical, 8)
            .background(Theme.navyDeep.opacity(configuration.isPressed ? 0.7 : 1))
            .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

struct RegisterCardView: View {
    @EnvironmentObject var model: AppModel
    let index: Int
    var focused: FocusState<CounterView.FocusKey?>.Binding

    var body: some View {
        let r = model.result(index)
        VStack(spacing: 0) {
            HStack {
                TextField("R\(index + 1)", text: nameBinding)
                    .font(.subheadline.weight(.semibold))
                    .foregroundStyle(.white)
                    .padding(6)
                Spacer()
                Button("Clear") { model.clearRegister(index) }
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(.white)
                    .disabled(!r.hasCount)
                Text(r.hasCount ? (r.balanced ? "Balanced" : "Off base") : "Empty")
                    .font(.caption2.monospaced())
                    .padding(.horizontal, 8)
                    .padding(.vertical, 4)
                    .background(r.hasCount ? (r.balanced ? Theme.ok : Theme.bad) : Theme.navyDeep)
                    .foregroundStyle(r.hasCount ? (r.balanced ? Theme.okInk : Theme.badInk) : .white.opacity(0.8))
                    .clipShape(RoundedRectangle(cornerRadius: 4))
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 8)
            .background(Theme.navy)

            headerRow
            ForEach(Denom.allCases, id: \.rawValue) { d in
                denomRow(d, r)
            }
            totalRow(r)
        }
        .background(Theme.paper)
        .clipShape(RoundedRectangle(cornerRadius: 12))
        .overlay(RoundedRectangle(cornerRadius: 12).stroke(Theme.grid))
    }

    private var nameBinding: Binding<String> {
        Binding(
            get: { model.names[index] },
            set: { model.setName(register: index, name: $0) }
        )
    }

    private var headerRow: some View {
        HStack {
            Text("").frame(width: 28)
            Text("Count").frame(maxWidth: .infinity)
            Text("Denom").frame(width: 52, alignment: .trailing)
            Text("Amount").frame(width: 72, alignment: .trailing)
            Text("Drop").frame(width: 52, alignment: .trailing)
            Text("Left").frame(width: 64, alignment: .trailing)
        }
        .font(.caption2.weight(.semibold))
        .foregroundStyle(.white)
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(Theme.navyMid)
    }

    private func denomRow(_ d: Denom, _ r: RegisterResult) -> some View {
        let isRoll = d.kind == .roll
        return HStack(spacing: 4) {
            Text(d.rollLetter ?? "")
                .font(.caption.monospaced())
                .frame(width: 28)
            Button {
                focused.wrappedValue = CounterView.FocusKey(register: index, denom: d)
            } label: {
                Text(r.counts[d] == 0 ? " " : "\(r.counts[d])")
                    .font(.body.monospaced())
                    .foregroundStyle(Theme.ink)
                    .frame(maxWidth: .infinity, minHeight: 40)
                    .background(Theme.input)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
                    .overlay(
                        RoundedRectangle(cornerRadius: 6)
                            .stroke(focused.wrappedValue?.denom == d && focused.wrappedValue?.register == index ? Theme.navyMid : Color.clear, lineWidth: 2)
                    )
            }
            .buttonStyle(.plain)
            Text(d.label)
                .font(.caption.monospaced())
                .frame(width: 52, alignment: .trailing)
            Text(moneyOrBlank(r.counts[d] * d.cents))
                .font(.caption.monospaced())
                .frame(width: 72, alignment: .trailing)
                .padding(.vertical, 6)
                .background(Theme.computed)
            Text(r.drop[d] == 0 ? "" : "\(r.drop[d])")
                .font(.caption.monospaced())
                .frame(width: 52, alignment: .trailing)
            Text(moneyOrBlank(r.left[d] * d.cents))
                .font(.caption.monospaced())
                .frame(width: 64, alignment: .trailing)
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 2)
        .background(isRoll ? Theme.paper : Theme.paper)
        .overlay(alignment: .top) {
            if d == .pRoll { Rectangle().fill(Theme.ink).frame(height: 2) }
        }
        .overlay(alignment: .bottom) {
            if d == .qRoll { Rectangle().fill(Theme.ink).frame(height: 2) }
        }
    }

    private func totalRow(_ r: RegisterResult) -> some View {
        HStack {
            Text("Total").font(.caption.weight(.semibold)).foregroundStyle(.white)
            Spacer()
            Text(DropEngine.money(r.amountCents)).foregroundStyle(.white)
            Text(DropEngine.money(r.dropCents)).frame(width: 52, alignment: .trailing).foregroundStyle(.white)
            Text(DropEngine.money(r.leftCents))
                .frame(width: 64, alignment: .trailing)
                .padding(4)
                .background(r.hasCount ? (r.balanced ? Theme.ok : Theme.bad) : Theme.navyDeep)
                .foregroundStyle(r.hasCount ? (r.balanced ? Theme.okInk : Theme.badInk) : .white)
        }
        .font(.caption.monospaced())
        .padding(8)
        .background(Theme.navyMid)
    }

    private func moneyOrBlank(_ cents: Int) -> String {
        cents == 0 ? "" : DropEngine.money(cents)
    }
}

struct NumberPadView: View {
    var focused: FocusState<CounterView.FocusKey?>.Binding
    @EnvironmentObject var model: AppModel

    private let keys = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "Clear", "0", "Enter"]

    var body: some View {
        VStack(spacing: 6) {
            LazyVGrid(columns: Array(repeating: GridItem(.flexible(), spacing: 6), count: 3), spacing: 6) {
                ForEach(keys, id: \.self) { k in
                    Button {
                        press(k)
                    } label: {
                        Text(k)
                            .font(k == "Clear" || k == "Enter" ? .subheadline.weight(.semibold) : .title2.monospaced())
                            .frame(maxWidth: .infinity, minHeight: 48)
                            .background(k == "Enter" ? Theme.navy : (k == "Clear" ? Theme.sheet : Theme.paper))
                            .foregroundStyle(k == "Enter" ? Color.white : Theme.ink)
                            .clipShape(RoundedRectangle(cornerRadius: 8))
                    }
                }
            }
        }
        .padding(8)
        .padding(.bottom, 8)
        .background(Theme.navyDeep)
    }

    private func press(_ key: String) {
        if focused.wrappedValue == nil {
            focused.wrappedValue = CounterView.FocusKey(register: model.active, denom: .penny)
        }
        guard let f = focused.wrappedValue else { return }
        var counts = model.registers[f.register]
        if key == "Clear" {
            counts[f.denom] = 0
            model.registers[f.register] = counts
            model.setCount(register: f.register, denom: f.denom, value: 0)
            return
        }
        if key == "Enter" {
            advance(from: f)
            return
        }
        let cur = counts[f.denom]
        let next = Int("\(cur == 0 ? "" : "\(cur)")\(key)") ?? 0
        model.setCount(register: f.register, denom: f.denom, value: next)
    }

    private func advance(from f: CounterView.FocusKey) {
        let all = Denom.allCases
        if let i = all.firstIndex(of: f.denom), i + 1 < all.count {
            focused.wrappedValue = CounterView.FocusKey(register: f.register, denom: all[i + 1])
            return
        }
        if f.register + 1 < DropEngine.registerCount {
            model.active = f.register + 1
            focused.wrappedValue = CounterView.FocusKey(register: f.register + 1, denom: .penny)
        }
    }
}
