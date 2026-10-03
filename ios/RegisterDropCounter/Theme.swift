import SwiftUI
import UIKit

enum Theme {
    static let navy = adaptive(
        light: (31 / 255, 78 / 255, 121 / 255),
        dark: (26 / 255, 51 / 255, 82 / 255)
    )
    static let navyDeep = adaptive(
        light: (22 / 255, 58 / 255, 95 / 255),
        dark: (21 / 255, 40 / 255, 68 / 255)
    )
    static let navyMid = adaptive(
        light: (46 / 255, 117 / 255, 182 / 255),
        dark: (46 / 255, 85 / 255, 128 / 255)
    )
    static let navyFg = adaptive(
        light: (31 / 255, 78 / 255, 121 / 255),
        dark: (110 / 255, 231 / 255, 212 / 255)
    )
    static let input = adaptive(
        light: (255 / 255, 244 / 255, 194 / 255),
        dark: (245 / 255, 197 / 255, 66 / 255)
    )
    static let computed = adaptive(
        light: (248 / 255, 228 / 255, 212 / 255),
        dark: (30 / 255, 48 / 255, 68 / 255)
    )
    static let ok = adaptive(
        light: (198 / 255, 239 / 255, 206 / 255),
        dark: (13 / 255, 63 / 255, 53 / 255)
    )
    static let okInk = adaptive(
        light: (20 / 255, 90 / 255, 50 / 255),
        dark: (110 / 255, 240 / 255, 200 / 255)
    )
    static let bad = adaptive(
        light: (255 / 255, 199 / 255, 206 / 255),
        dark: (77 / 255, 31 / 255, 40 / 255)
    )
    static let badInk = adaptive(
        light: (140 / 255, 30 / 255, 40 / 255),
        dark: (255 / 255, 176 / 255, 188 / 255)
    )
    static let cellInk = adaptive(
        light: (26 / 255, 36 / 255, 46 / 255),
        dark: (28 / 255, 20 / 255, 6 / 255)
    )

    static let sheet = adaptive(
        light: (238 / 255, 241 / 255, 244 / 255),
        dark: (27 / 255, 42 / 255, 61 / 255)
    )
    static let paper = adaptive(
        light: (1, 1, 1),
        dark: (36 / 255, 54 / 255, 76 / 255)
    )
    static let ink = adaptive(
        light: (26 / 255, 36 / 255, 46 / 255),
        dark: (231 / 255, 246 / 255, 242 / 255)
    )
    static let muted = adaptive(
        light: (90 / 255, 102 / 255, 114 / 255),
        dark: (143 / 255, 179 / 255, 174 / 255)
    )
    static let grid = adaptive(
        light: (197 / 255, 208 / 255, 219 / 255),
        dark: (60 / 255, 85 / 255, 112 / 255)
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