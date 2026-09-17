package com.ronaldrobbins.registerdropcounter

import androidx.compose.ui.graphics.Color

object RdcColor {
    var dark: Boolean = false

    val navy = Color(0xFF1F4E79)
    val navyDeep = Color(0xFF163A5F)
    val navyMid = Color(0xFF2E75B6)
    val input = Color(0xFFFFF4C2)
    val computed = Color(0xFFF8E4D4)
    val ok = Color(0xFFC6EFCE)
    val okInk = Color(0xFF006100)
    val bad = Color(0xFFFFC7CE)
    val badInk = Color(0xFF9C0006)
    val onNavy = Color(0xFFF7FBFF)
    val cellInk = Color(0xFF1A242E)

    val navyFg: Color get() = if (dark) Color(0xFF9CC7EC) else Color(0xFF1F4E79)
    val sheet: Color get() = if (dark) Color(0xFF121820) else Color(0xFFEEF1F4)
    val paper: Color get() = if (dark) Color(0xFF1C2530) else Color(0xFFFFFFFF)
    val ink: Color get() = if (dark) Color(0xFFE6EEF6) else Color(0xFF1A242E)
    val muted: Color get() = if (dark) Color(0xFF9AABBA) else Color(0xFF5C6B7A)
    val grid: Color get() = if (dark) Color(0xFF3A4A5A) else Color(0xFFC5D0DB)
}
