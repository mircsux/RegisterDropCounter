package com.ronaldrobbins.registerdropcounter

import androidx.compose.ui.graphics.Color

data class RdcPalette(
    val id: String,
    val name: String,
    val dark: Boolean,
    val navy: Color,
    val navyDeep: Color,
    val navyMid: Color,
    val onNavy: Color,
    val navyFg: Color,
    val sheet: Color,
    val paper: Color,
    val ink: Color,
    val muted: Color,
    val grid: Color,
    val input: Color,
    val computed: Color,
    val ok: Color,
    val okInk: Color,
    val bad: Color,
    val badInk: Color,
    val cellInk: Color,
    val dropHit: Color,
)

private fun c(hex: Long) = Color(hex)

private val amberIn = c(0xFFFFF4C2)
private val amberNight = c(0xFFF5C542)
private val cellDay = c(0xFF1A242E)
private val cellNight = c(0xFF1C1406)
private val okDay = c(0xFFC6EFCE)
private val okInkDay = c(0xFF006100)
private val badDay = c(0xFFFFC7CE)
private val badInkDay = c(0xFF9C0006)
private val hitDay = c(0xFFD6EADF)
private val okNight = c(0xFF0D3F35)
private val okInkNight = c(0xFF6EF0C8)
private val badNight = c(0xFF4D1F28)
private val badInkNight = c(0xFFFFB0BC)
private val hitNight = c(0xFF15564A)

private fun day(
    id: String, name: String,
    navy: Long, mid: Long, deep: Long, fg: Long, on: Long,
    sheet: Long, paper: Long, ink: Long, muted: Long, grid: Long, computed: Long,
) = RdcPalette(
    id, name, false, c(navy), c(deep), c(mid), c(on), c(fg), c(sheet), c(paper), c(ink), c(muted),
    c(grid), amberIn, c(computed), okDay, okInkDay, badDay, badInkDay, cellDay, hitDay,
)

private fun night(
    id: String, name: String,
    navy: Long, mid: Long, deep: Long, fg: Long, on: Long,
    sheet: Long, paper: Long, ink: Long, muted: Long, grid: Long, computed: Long,
) = RdcPalette(
    id, name, true, c(navy), c(deep), c(mid), c(on), c(fg), c(sheet), c(paper), c(ink), c(muted),
    c(grid), amberNight, c(computed), okNight, okInkNight, badNight, badInkNight, cellNight, hitNight,
)

object RdcThemes {
    val all = listOf(
        day("classic", "Classic Navy", 0xFF1F4E79, 0xFF2E75B6, 0xFF163A5F, 0xFF1F4E79, 0xFFF7FBFF, 0xFFEEF1F4, 0xFFFFFFFF, 0xFF1A242E, 0xFF5C6B7A, 0xFFC5D0DB, 0xFFF8E4D4),
        night("cobalt", "Cobalt Night", 0xFF1A3352, 0xFF2E5580, 0xFF152844, 0xFF6EE7D4, 0xFFE7F6F2, 0xFF1B2A3D, 0xFF24364C, 0xFFE7F6F2, 0xFF8FB3AE, 0xFF3C5570, 0xFF1E3044),
        day("forest", "Evergreen", 0xFF1B4D3E, 0xFF2F7D62, 0xFF14382E, 0xFF1B4D3E, 0xFFF4FBF7, 0xFFEEF6F1, 0xFFFFFFFF, 0xFF1A2A24, 0xFF5C7268, 0xFFC3D9CC, 0xFFF3E6D4),
        night("pine", "Pine Night", 0xFF1A3D32, 0xFF2F6B56, 0xFF132E26, 0xFF9EE6C8, 0xFFE7F6EE, 0xFF14241E, 0xFF1D332A, 0xFFE7F6EE, 0xFF9CB8AA, 0xFF3D5C4E, 0xFF24382F),
        day("burgundy", "Burgundy", 0xFF7A2438, 0xFFA33D56, 0xFF5C1A2A, 0xFF7A2438, 0xFFFFF7F8, 0xFFF7F0F2, 0xFFFFFFFF, 0xFF2A1C22, 0xFF7A6570, 0xFFE4CFD6, 0xFFF8E4D4),
        night("wine", "Wine Night", 0xFF4A2030, 0xFF7A3A50, 0xFF341622, 0xFFF0B4C4, 0xFFFBEFF2, 0xFF24161C, 0xFF322028, 0xFFFBEFF2, 0xFFC4A8B0, 0xFF5C3A48, 0xFF3A2830),
        day("ocean", "Lagoon", 0xFF0E6B7A, 0xFF1A9AAB, 0xFF0A4E59, 0xFF0E6B7A, 0xFFF3FBFC, 0xFFEEF7F8, 0xFFFFFFFF, 0xFF163034, 0xFF5A7278, 0xFFC5DDE2, 0xFFF6E6D6),
        night("harbor", "Harbor Night", 0xFF0E3D48, 0xFF1A6A78, 0xFF0A2C34, 0xFF7EE0E8, 0xFFE7F7F8, 0xFF10282E, 0xFF183840, 0xFFE7F7F8, 0xFF9EC4C8, 0xFF2F5C66, 0xFF1C4048),
        day("plum", "Plum", 0xFF5C3D7A, 0xFF7D5AA3, 0xFF432C5C, 0xFF5C3D7A, 0xFFFAF7FC, 0xFFF4F0F8, 0xFFFFFFFF, 0xFF261C30, 0xFF6E6578, 0xFFD8CFE6, 0xFFF6E4D8),
        night("ink", "Violet Ink", 0xFF3A2A58, 0xFF5C4588, 0xFF281C40, 0xFFD4C4F5, 0xFFF4EFFC, 0xFF1C1628, 0xFF282036, 0xFFF4EFFC, 0xFFB8A8CC, 0xFF4A3C64, 0xFF322848),
        day("sunrise", "Sunrise", 0xFF9A4A1C, 0xFFC46A32, 0xFF6E3414, 0xFF9A4A1C, 0xFFFFF8F3, 0xFFFBF4EC, 0xFFFFFDF8, 0xFF2C2218, 0xFF7A685C, 0xFFE6D4C4, 0xFFF8E0C8),
        day("slate", "Slate", 0xFF3D4C5C, 0xFF5A7084, 0xFF2C3844, 0xFF3D4C5C, 0xFFF7F9FB, 0xFFE8ECEF, 0xFFFFFFFF, 0xFF1E262E, 0xFF66707A, 0xFFC5CED6, 0xFFF3E4D6),
    )

    fun byId(id: String?, dark: Boolean = false): RdcPalette {
        val found = all.firstOrNull { it.id == id }
        if (found != null) return found
        return all.first { it.id == if (dark) "cobalt" else "classic" }
    }
}

object RdcColor {
    var themeId: String = "classic"

    private val p: RdcPalette get() = RdcThemes.byId(themeId)

    val navy: Color get() = p.navy
    val navyDeep: Color get() = p.navyDeep
    val navyMid: Color get() = p.navyMid
    val onNavy: Color get() = p.onNavy
    val navyFg: Color get() = p.navyFg
    val sheet: Color get() = p.sheet
    val paper: Color get() = p.paper
    val ink: Color get() = p.ink
    val muted: Color get() = p.muted
    val grid: Color get() = p.grid
    val input: Color get() = p.input
    val computed: Color get() = p.computed
    val ok: Color get() = p.ok
    val okInk: Color get() = p.okInk
    val bad: Color get() = p.bad
    val badInk: Color get() = p.badInk
    val cellInk: Color get() = p.cellInk
    val dropHit: Color get() = p.dropHit
}
