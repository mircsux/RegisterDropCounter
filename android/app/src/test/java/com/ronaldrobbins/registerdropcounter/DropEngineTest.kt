package com.ronaldrobbins.registerdropcounter

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class DropEngineTest {
    @Test
    fun register1MatchesWorkbook() {
        val c = Counts.fromList(listOf(0, 6, 48, 27, 0, 0, 0, 0, 109, 0, 5, 10, 103, 6, 12))
        val r = DropEngine.compute(c, 400)
        assertEquals(380585, r.amountCents)
        assertEquals(340585, r.dropCents)
        assertEquals(40000, r.leftCents)
        assertTrue(r.balanced)
        assertEquals(95, r.drop[Denom.Twenty])
        assertEquals(12, r.drop[Denom.Hundred])
        assertEquals(109, r.left[Denom.One])
        assertEquals("$3,805.85", DropEngine.money(r.amountCents))
    }

    @Test
    fun twoDollarBillsDrop() {
        val c = Counts()
        c[Denom.One] = 400
        c[Denom.Two] = 3
        c[Denom.Five] = 1
        val r = DropEngine.compute(c, 400)
        assertEquals(3, r.drop[Denom.Two])
        assertEquals(400, r.left[Denom.One])
    }

    @Test
    fun dropSlipFitsStarTsc100() {
        val c = Counts()
        c[Denom.Twenty] = 25
        val r = DropEngine.compute(c, 400)
        val text = DropEngine.dropSlipText(
            till = "Drive-thru lane",
            baseDollars = 400,
            r = r,
            bag = "SEAL-998877",
            initials = "RRJR",
        )
        assertTrue(text.contains("DROP SLIP"))
        assertTrue(text.contains("Till: Drive-thru lane"))
        assertTrue(text.contains("DROP TOTAL"))
        assertTrue(text.contains("LEFT IN DRAWER"))
        assertTrue(text.indexOf("DROP TOTAL") < text.indexOf("ITEM"))
        assertEquals(42, DropEngine.RECEIPT_COLS)
        for (line in text.split("\n")) {
            assertTrue("line too wide: $line", line.length <= DropEngine.RECEIPT_COLS)
        }
    }
}
