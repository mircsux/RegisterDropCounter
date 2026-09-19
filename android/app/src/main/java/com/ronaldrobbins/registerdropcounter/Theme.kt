package com.ronaldrobbins.registerdropcounter

import androidx.compose.ui.graphics.Color

object RdcColor {
    var dark: Boolean = false

    val navy: Color get() = if (dark) Color(0xFF102C32) else Color(0xFF1F4E79)
    val navyDeep: Color get() = if (dark) Color(0xFF163E46) else Color(0xFF163A5F)
    val navyMid: Color get() = if (dark) Color(0xFF1E5A64) else Color(0xFF2E75B6)
    val onNavy: Color get() = if (dark) Color(0xFFE7F6F2) else Color(0xFFF7FBFF)

    val navyFg: Color get() = if (dark) Color(0xFF6EE7D4) else Color(0xFF1F4E79)
    val sheet: Color get() = if (dark) Color(0xFF060809) else Color(0xFFEEF1F4)
    val paper: Color get() = if (dark) Color(0xFF10171A) else Color(0xFFFFFFFF)
    val ink: Color get() = if (dark) Color(0xFFE7F6F2) else Color(0xFF1A242E)
    val muted: Color get() = if (dark) Color(0xFF8FB3AE) else Color(0xFF5C6B7A)
    val grid: Color get() = if (dark) Color(0xFF2C454B) else Color(0xFFC5D0DB)
    val input: Color get() = if (dark) Color(0xFFF5C542) else Color(0xFFFFF4C2)
    val computed: Color get() = if (dark) Color(0xFF1A272B) else Color(0xFFF8E4D4)
    val ok: Color get() = if (dark) Color(0xFF0D3F35) else Color(0xFFC6EFCE)
    val okInk: Color get() = if (dark) Color(0xFF6EF0C8) else Color(0xFF006100)
    val bad: Color get() = if (dark) Color(0xFF4D1F28) else Color(0xFFFFC7CE)
    val badInk: Color get() = if (dark) Color(0xFFFFB0BC) else Color(0xFF9C0006)
    val cellInk: Color get() = if (dark) Color(0xFF1C1406) else Color(0xFF1A242E)
    val dropHit: Color get() = if (dark) Color(0xFF15564A) else Color(0xFFD6EADF)
}