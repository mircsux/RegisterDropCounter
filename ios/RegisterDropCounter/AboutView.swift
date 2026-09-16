import SwiftUI

struct AboutView: View {
    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    VStack(alignment: .leading, spacing: 6) {
                        Text("Register Drop Counter")
                            .font(.title2.weight(.semibold))
                            .foregroundStyle(.white)
                        Text("Designed by Ronald Robbins Jr and SuperGrok")
                            .foregroundStyle(.white)
                        Text("Version \(AppModel.version)  (\(AppModel.releaseDate))")
                            .font(.caption.monospaced())
                            .foregroundStyle(.white.opacity(0.85))
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding()
                    .background(Theme.navy)

                    Group {
                        Text("Count a drawer")
                            .font(.headline)
                        Text("Set the register base. Open a till (R1–R10). Tap the name to rename it. Type counts in the yellow cells with the number pad. Amount, Drop, and Left fill in. Left turns green when it equals the base.")
                        Text("Drop slip prints a bag slip. Undo clear brings back the last drawer. History stores every clear.")
                        Text("The drop")
                            .font(.headline)
                        Text("$100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies. Loose coins drop before rolls.")
                        Text("Changelog")
                            .font(.headline)
                        Text("v2.11.0  ISO C23 console app with the same drop math. No C++.")
                        Text("v2.10.0  iPhone app (SwiftUI). Same drop math as Windows and the web counter.")
                        Text("v2.9.0  Named tills — tap R1–R10 to rename.")
                        Text("v2.8.0  Undo clear and printable drop slip.")
                        Text("v2.7.0  Phone number pad and arrow-key navigation.")
                    }
                    .padding(.horizontal)
                }
                .padding(.bottom, 24)
            }
            .background(Theme.sheet)
            .navigationTitle("About")
        }
    }
}
