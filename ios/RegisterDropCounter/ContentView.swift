import SwiftUI

struct ContentView: View {
    @EnvironmentObject var model: AppModel

    var body: some View {
        TabView {
            CounterView()
                .tabItem { Label("Counter", systemImage: "tray.full") }
            HistoryView()
                .tabItem { Label("History", systemImage: "clock") }
            OptionsView()
                .tabItem { Label("Options", systemImage: "gearshape") }
            AboutView()
                .tabItem { Label("About", systemImage: "info.circle") }
        }
    }
}
