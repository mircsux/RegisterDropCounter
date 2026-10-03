import SwiftUI
import UIKit

struct ThemePalette {
    var id: String
    var name: String
    var dark: Bool
    var navy: (CGFloat, CGFloat, CGFloat)
    var navyDeep: (CGFloat, CGFloat, CGFloat)
    var navyMid: (CGFloat, CGFloat, CGFloat)
    var navyFg: (CGFloat, CGFloat, CGFloat)
    var onNavy: (CGFloat, CGFloat, CGFloat)
    var sheet: (CGFloat, CGFloat, CGFloat)
    var paper: (CGFloat, CGFloat, CGFloat)
    var ink: (CGFloat, CGFloat, CGFloat)
    var muted: (CGFloat, CGFloat, CGFloat)
    var grid: (CGFloat, CGFloat, CGFloat)
    var input: (CGFloat, CGFloat, CGFloat)
    var computed: (CGFloat, CGFloat, CGFloat)
    var ok: (CGFloat, CGFloat, CGFloat)
    var okInk: (CGFloat, CGFloat, CGFloat)
    var bad: (CGFloat, CGFloat, CGFloat)
    var badInk: (CGFloat, CGFloat, CGFloat)
    var cellInk: (CGFloat, CGFloat, CGFloat)
}

private func rgb(_ r: CGFloat, _ g: CGFloat, _ b: CGFloat) -> (CGFloat, CGFloat, CGFloat) {
    (r / 255, g / 255, b / 255)
}

enum ThemeCatalog {
    static var currentId = "classic"

    static let all: [ThemePalette] = [
        day("classic", "Classic Navy", 31, 78, 121, 46, 117, 182, 22, 58, 95, 31, 78, 121, 247, 251, 255, 238, 241, 244, 255, 255, 255, 26, 36, 46, 92, 107, 122, 197, 208, 219, 248, 228, 212),
        night("cobalt", "Cobalt Night", 26, 51, 82, 46, 85, 128, 21, 40, 68, 110, 231, 212, 231, 246, 242, 27, 42, 61, 36, 54, 76, 231, 246, 242, 143, 179, 174, 60, 85, 112, 30, 48, 68),
        day("forest", "Evergreen", 27, 77, 62, 47, 125, 98, 20, 56, 46, 27, 77, 62, 244, 251, 247, 238, 246, 241, 255, 255, 255, 26, 42, 36, 92, 114, 104, 195, 217, 204, 243, 230, 212),
        night("pine", "Pine Night", 26, 61, 50, 47, 107, 86, 19, 46, 38, 158, 230, 200, 231, 246, 238, 20, 36, 30, 29, 51, 42, 231, 246, 238, 156, 184, 170, 61, 92, 78, 36, 56, 47),
        day("burgundy", "Burgundy", 122, 36, 56, 163, 61, 86, 92, 26, 42, 122, 36, 56, 255, 247, 248, 247, 240, 242, 255, 255, 255, 42, 28, 34, 122, 101, 112, 228, 207, 214, 248, 228, 212),
        night("wine", "Wine Night", 74, 32, 48, 122, 58, 80, 52, 22, 34, 240, 180, 196, 251, 239, 242, 36, 22, 28, 50, 32, 40, 251, 239, 242, 196, 168, 176, 92, 58, 72, 58, 40, 48),
        day("ocean", "Lagoon", 14, 107, 122, 26, 154, 171, 10, 78, 89, 14, 107, 122, 243, 251, 252, 238, 247, 248, 255, 255, 255, 22, 48, 52, 90, 114, 120, 197, 221, 226, 246, 230, 214),
        night("harbor", "Harbor Night", 14, 61, 72, 26, 106, 120, 10, 44, 52, 126, 224, 232, 231, 247, 248, 16, 40, 46, 24, 56, 64, 231, 247, 248, 158, 196, 200, 47, 92, 102, 28, 64, 72),
        day("plum", "Plum", 92, 61, 122, 125, 90, 163, 67, 44, 92, 92, 61, 122, 250, 247, 252, 244, 240, 248, 255, 255, 255, 38, 28, 48, 110, 101, 120, 216, 207, 230, 246, 228, 216),
        night("ink", "Violet Ink", 58, 42, 88, 92, 69, 136, 40, 28, 64, 212, 196, 245, 244, 239, 252, 28, 22, 40, 40, 32, 54, 244, 239, 252, 184, 168, 204, 74, 60, 100, 50, 40, 72),
        day("sunrise", "Sunrise", 154, 74, 28, 196, 106, 50, 110, 52, 20, 154, 74, 28, 255, 248, 243, 251, 244, 236, 255, 253, 248, 44, 34, 24, 122, 104, 92, 230, 212, 196, 248, 224, 200),
        day("slate", "Slate", 61, 76, 92, 90, 112, 132, 44, 56, 68, 61, 76, 92, 247, 249, 251, 232, 236, 239, 255, 255, 255, 30, 38, 46, 102, 112, 122, 197, 206, 214, 243, 228, 214),
    ]

    static var current: ThemePalette { byId(currentId) }

    static func byId(_ id: String, dark: Bool = false) -> ThemePalette {
        if let found = all.first(where: { $0.id == id }) { return found }
        let fallback = dark ? "cobalt" : "classic"
        return all.first { $0.id == fallback } ?? all[0]
    }

    @discardableResult
    static func select(_ id: String, dark: Bool = false) -> ThemePalette {
        let p = byId(id, dark: dark)
        currentId = p.id
        return p
    }

    private static func day(_ id: String, _ name: String, _ n: CGFloat...) -> ThemePalette {
        pack(id, name, false, n)
    }
    private static func night(_ id: String, _ name: String, _ n: CGFloat...) -> ThemePalette {
        pack(id, name, true, n)
    }
    private static func pack(_ id: String, _ name: String, _ dark: Bool, _ n: [CGFloat]) -> ThemePalette {
        func t(_ i: Int) -> (CGFloat, CGFloat, CGFloat) { rgb(n[i], n[i + 1], n[i + 2]) }
        let input = dark ? rgb(245, 197, 66) : rgb(255, 244, 194)
        let cell = dark ? rgb(28, 20, 6) : rgb(26, 36, 46)
        let ok = dark ? rgb(13, 63, 53) : rgb(198, 239, 206)
        let okInk = dark ? rgb(110, 240, 200) : rgb(0, 97, 0)
        let bad = dark ? rgb(77, 31, 40) : rgb(255, 199, 206)
        let badInk = dark ? rgb(255, 176, 188) : rgb(156, 0, 6)
        return ThemePalette(
            id: id, name: name, dark: dark,
            navy: t(0), navyMid: t(3), navyDeep: t(6), navyFg: t(9), onNavy: t(12),
            sheet: t(15), paper: t(18), ink: t(21), muted: t(24), grid: t(27), computed: t(30),
            input: input, ok: ok, okInk: okInk, bad: bad, badInk: badInk, cellInk: cell
        )
    }
}

enum Theme {
    private static func color(_ c: (CGFloat, CGFloat, CGFloat)) -> Color {
        Color(red: c.0, green: c.1, blue: c.2)
    }
    static var navy: Color { color(ThemeCatalog.current.navy) }
    static var navyDeep: Color { color(ThemeCatalog.current.navyDeep) }
    static var navyMid: Color { color(ThemeCatalog.current.navyMid) }
    static var navyFg: Color { color(ThemeCatalog.current.navyFg) }
    static var input: Color { color(ThemeCatalog.current.input) }
    static var computed: Color { color(ThemeCatalog.current.computed) }
    static var ok: Color { color(ThemeCatalog.current.ok) }
    static var okInk: Color { color(ThemeCatalog.current.okInk) }
    static var bad: Color { color(ThemeCatalog.current.bad) }
    static var badInk: Color { color(ThemeCatalog.current.badInk) }
    static var cellInk: Color { color(ThemeCatalog.current.cellInk) }
    static var sheet: Color { color(ThemeCatalog.current.sheet) }
    static var paper: Color { color(ThemeCatalog.current.paper) }
    static var ink: Color { color(ThemeCatalog.current.ink) }
    static var muted: Color { color(ThemeCatalog.current.muted) }
    static var grid: Color { color(ThemeCatalog.current.grid) }
}
