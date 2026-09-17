import SwiftUI

@main
struct RegisterDropCounterApp: App {
    @StateObject private var model = AppModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(model)
                .tint(Theme.navy)
                .preferredColorScheme(model.darkMode ? .dark : .light)
        }
    }
}
