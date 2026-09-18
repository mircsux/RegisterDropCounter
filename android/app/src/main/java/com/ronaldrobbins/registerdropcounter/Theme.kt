package com.ronaldrobbins.registerdropcounter

import androidx.compose.ui.graphics.Color

object RdcColor {
    var dark: Boolean = false

    val navy = Color(0xFF1F4E79)
    val navyDeep = Color(0xFF163A5F)
    val navyMid = Color(0xFF2E75B6)
    val onNavy = Color(0xFFF7FBFF)

    val navyFg: Color get() = if (dark) Color(0xFF9CC7EC) else Color(0xFF1F4E79)
    val sheet: Color get() = if (dark) Color(0xFF0E141B) else Color(0xFFEEF1F4)
    val paper: Color get() = if (dark) Color(0xFF18222C) else Color(0xFFFFFFFF)
    val ink: Color get() = if (dark) Color(0xFFE6EEF6) else Color(0xFF1A242E)
    val muted: Color get() = if (dark) Color(0xFF9AABBA) else Color(0xFF5C6B7A)
    val grid: Color get() = if (dark) Color(0xFF3D4F61) else Color(0xFFC5D0DB)
    val input: Color get() = if (dark) Color(0xFF4A3F18) else Color(0xFFFFF4C2)
    val computed: Color get() = if (dark) Color(0xFF24303C) else Color(0xFFF8E4D4)
    val ok: Color get() = if (dark) Color(0xFF163A28) else Color(0xFFC6EFCE)
    val okInk: Color get() = if (dark) Color(0xFF9EEBC0) else Color(0xFF006100)
    val bad: Color get() = if (dark) Color(0xFF4A1E24) else Color(0xFFFFC7CE)
    val badInk: Color get() = if (dark) Color(0xFFFFB0B8) else Color(0xFF9C0006)
    val cellInk: Color get() = if (dark) Color(0xFFFFE8A0) else Color(0xFF1A242E)
    val dropHit: Color get() = if (dark) Color(0xFF1B3D30) else Color(0xFFD6EADF)
}
