import SwiftUI

@main
struct RegisterDropCounterApp: App {
    @StateObject private var model = AppModel()

    var body: some Scene {
        WindowGroup {
            let palette = ThemeCatalog.select(model.theme, dark: model.darkMode)
            ContentView()
                .id(palette.id)
                .environmentObject(model)
                .tint(Theme.navy)
                .preferredColorScheme(palette.dark ? .dark : .light)
        }
    }
}
