import SwiftUI
import UIKit

enum Theme {
    static let navy = Color(red: 31 / 255, green: 78 / 255, blue: 121 / 255)
    static let navyDeep = Color(red: 22 / 255, green: 58 / 255, blue: 95 / 255)
    static let navyMid = Color(red: 46 / 255, green: 117 / 255, blue: 182 / 255)
    static let navyFg = adaptive(
        light: (31 / 255, 78 / 255, 121 / 255),
        dark: (156 / 255, 199 / 255, 236 / 255)
    )
    static let input = adaptive(
        light: (255 / 255, 244 / 255, 194 / 255),
        dark: (74 / 255, 63 / 255, 24 / 255)
    )
    static let computed = adaptive(
        light: (248 / 255, 228 / 255, 212 / 255),
        dark: (36 / 255, 48 / 255, 60 / 255)
    )
    static let ok = adaptive(
        light: (198 / 255, 239 / 255, 206 / 255),
        dark: (22 / 255, 58 / 255, 40 / 255)
    )
    static let okInk = adaptive(
        light: (20 / 255, 90 / 255, 50 / 255),
        dark: (158 / 255, 235 / 255, 192 / 255)
    )
    static let bad = adaptive(
        light: (255 / 255, 199 / 255, 206 / 255),
        dark: (74 / 255, 30 / 255, 36 / 255)
    )
    static let badInk = adaptive(
        light: (140 / 255, 30 / 255, 40 / 255),
        dark: (255 / 255, 176 / 255, 184 / 255)
    )
    static let cellInk = adaptive(
        light: (26 / 255, 36 / 255, 46 / 255),
        dark: (255 / 255, 232 / 255, 160 / 255)
    )

    static let sheet = adaptive(
        light: (238 / 255, 241 / 255, 244 / 255),
        dark: (14 / 255, 20 / 255, 27 / 255)
    )
    static let paper = adaptive(
        light: (1, 1, 1),
        dark: (24 / 255, 34 / 255, 44 / 255)
    )
    static let ink = adaptive(
        light: (26 / 255, 36 / 255, 46 / 255),
        dark: (230 / 255, 238 / 255, 246 / 255)
    )
    static let muted = adaptive(
        light: (90 / 255, 102 / 255, 114 / 255),
        dark: (154 / 255, 171 / 255, 186 / 255)
    )
    static let grid = adaptive(
        light: (197 / 255, 208 / 255, 219 / 255),
        dark: (58 / 255, 74 / 255, 90 / 255)
    )

    private static func adaptive(
        light: (CGFloat, CGFloat, CGFloat),
        dark: (CGFloat, CGFloat, CGFloat)
    ) -> Color {
        Color(uiColor: UIColor { trait in
            let c = trait.userInterfaceStyle == .dark ? dark : light
            return UIColor(red: c.0, green: c.1, blue: c.2, alpha: 1)
        })
    }
}
