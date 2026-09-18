package com.ronaldrobbins.registerdropcounter

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.TextField
import androidx.compose.material3.TextFieldDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@Composable
fun CounterScreen(model: AppModel, modifier: Modifier = Modifier) {
    var confirmClearAll by remember { mutableStateOf(false) }
    var slipOpen by remember { mutableStateOf(false) }
    var focused by remember { mutableStateOf(Denom.Penny) }
    val r = model.result(model.active)

    Column(modifier.fillMaxSize().background(RdcColor.sheet)) {
        Column(
            Modifier
                .fillMaxWidth()
                .background(RdcColor.navy)
                .padding(12.dp),
        ) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Column(Modifier.weight(1f)) {
                    Text("Register Drop Counter", color = RdcColor.onNavy, fontWeight = FontWeight.SemiBold)
                    Text("v2.21.0", color = RdcColor.onNavy.copy(alpha = 0.75f), fontSize = 12.sp, fontFamily = FontFamily.Monospace)
                }
                var baseOpen by remember { mutableStateOf(false) }
                Box {
                    Text(
                        DropEngine.money(model.base * 100),
                        color = RdcColor.onNavy,
                        modifier = Modifier
                            .clip(RoundedCornerShape(6.dp))
                            .background(RdcColor.navyDeep)
                            .clickable { baseOpen = true }
                            .padding(horizontal = 10.dp, vertical = 8.dp),
                    )
                    DropdownMenu(expanded = baseOpen, onDismissRequest = { baseOpen = false }) {
                        DropEngine.baseOptions.forEach { b ->
                            DropdownMenuItem(
                                text = { Text(DropEngine.money(b * 100)) },
                                onClick = { model.setBase(b); baseOpen = false },
                            )
                        }
                    }
                }
            }
            Spacer(Modifier.height(8.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                var fileOpen by remember { mutableStateOf(false) }
                Box {
                    NavyChip("File") { fileOpen = true }
                    DropdownMenu(expanded = fileOpen, onDismissRequest = { fileOpen = false }) {
                        DropdownMenuItem(
                            text = { Text("Load Sample Drops") },
                            onClick = { model.loadSample(); fileOpen = false },
                        )
                    }
                }
                NavyChip("Clear all") { confirmClearAll = true }
                val last = model.history.firstOrNull()
                val undoLabel = if (last?.kind == HistoryEntry.REGISTER && last.registerIndex != null)
                    "Undo ${model.names[last.registerIndex]}"
                else "Undo"
                NavyChip(undoLabel, enabled = last != null) { model.undoClear() }
                NavyChip("Drop slip") { slipOpen = true }
            }
        }

        Row(
            Modifier
                .horizontalScroll(rememberScrollState())
                .background(RdcColor.paper)
                .padding(8.dp),
            horizontalArrangement = Arrangement.spacedBy(6.dp),
        ) {
            repeat(DropEngine.REGISTER_COUNT) { i ->
                val rr = model.result(i)
                val bg = when {
                    model.active == i -> RdcColor.navy
                    rr.hasCount && rr.balanced -> RdcColor.ok
                    rr.hasCount && !rr.balanced -> RdcColor.bad
                    else -> RdcColor.sheet
                }
                val fg = when {
                    model.active == i -> RdcColor.onNavy
                    rr.hasCount && rr.balanced -> RdcColor.okInk
                    rr.hasCount && !rr.balanced -> RdcColor.badInk
                    else -> RdcColor.ink
                }
                Text(
                    model.names[i],
                    color = fg,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.SemiBold,
                    modifier = Modifier
                        .clip(RoundedCornerShape(6.dp))
                        .background(bg)
                        .clickable { model.setActive(i); focused = Denom.Penny }
                        .padding(horizontal = 12.dp, vertical = 8.dp),
                )
            }
        }

        Column(
            Modifier
                .weight(1f)
                .verticalScroll(rememberScrollState())
                .padding(12.dp),
        ) {
            RegisterCard(
                model = model,
                index = model.active,
                result = r,
                focused = focused,
                onFocus = { focused = it },
            )
            Spacer(Modifier.height(12.dp))
        }

        NumberPad(
            onDigit = { digit ->
                val cur = model.registers[model.active][focused]
                val next = ((if (cur == 0) "" else "$cur") + digit).toIntOrNull() ?: 0
                model.setCount(model.active, focused, next)
            },
            onClear = { model.setCount(model.active, focused, 0) },
            onEnter = {
                val all = Denom.entries
                val i = all.indexOf(focused)
                if (i + 1 < all.size) {
                    focused = all[i + 1]
                } else if (model.active + 1 < DropEngine.REGISTER_COUNT) {
                    model.setActive(model.active + 1)
                    focused = Denom.Penny
                }
            },
        )
    }

    if (confirmClearAll) {
        AlertDialog(
            onDismissRequest = { confirmClearAll = false },
            title = { Text("Clear all ten registers?") },
            confirmButton = {
                TextButton(onClick = { model.clearAll(); confirmClearAll = false }) { Text("Clear all") }
            },
            dismissButton = {
                TextButton(onClick = { confirmClearAll = false }) { Text("Cancel") }
            },
        )
    }
    if (slipOpen) {
        DropSlipDialog(model = model, index = model.active, onClose = { slipOpen = false })
    }
}

@Composable
private fun NavyChip(label: String, enabled: Boolean = true, onClick: () -> Unit) {
    Text(
        label,
        color = if (enabled) RdcColor.onNavy else RdcColor.onNavy.copy(alpha = 0.4f),
        fontSize = 12.sp,
        fontWeight = FontWeight.SemiBold,
        modifier = Modifier
            .clip(RoundedCornerShape(6.dp))
            .background(RdcColor.navyDeep)
            .clickable(enabled = enabled, onClick = onClick)
            .padding(horizontal = 10.dp, vertical = 8.dp),
    )
}

@Composable
private fun RegisterCard(
    model: AppModel,
    index: Int,
    result: RegisterResult,
    focused: Denom,
    onFocus: (Denom) -> Unit,
) {
    Column(
        Modifier
            .clip(RoundedCornerShape(12.dp))
            .border(1.dp, RdcColor.grid, RoundedCornerShape(12.dp))
            .background(RdcColor.paper),
    ) {
        Row(
            Modifier
                .fillMaxWidth()
                .background(RdcColor.navy)
                .padding(10.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            var draft by remember(index, model.names[index]) { mutableStateOf(model.names[index]) }
            TextField(
                value = draft,
                onValueChange = {
                    draft = it
                    model.setName(index, it)
                },
                singleLine = true,
                colors = TextFieldDefaults.colors(
                    focusedContainerColor = Color.Transparent,
                    unfocusedContainerColor = Color.Transparent,
                    focusedTextColor = RdcColor.onNavy,
                    unfocusedTextColor = RdcColor.onNavy,
                    cursorColor = RdcColor.onNavy,
                    focusedIndicatorColor = Color.Transparent,
                    unfocusedIndicatorColor = Color.Transparent,
                ),
                modifier = Modifier.weight(1f),
            )
            Text(
                "Clear",
                color = RdcColor.onNavy,
                fontSize = 12.sp,
                modifier = Modifier
                    .clickable(enabled = result.hasCount) { model.clearRegister(index) }
                    .padding(8.dp),
            )
            val status = if (!result.hasCount) "Empty" else if (result.balanced) "Balanced" else "Off base"
            val bg = if (!result.hasCount) RdcColor.navyDeep else if (result.balanced) RdcColor.ok else RdcColor.bad
            val fg = if (!result.hasCount) RdcColor.onNavy.copy(alpha = 0.8f) else if (result.balanced) RdcColor.okInk else RdcColor.badInk
            Text(
                status,
                color = fg,
                fontSize = 11.sp,
                fontFamily = FontFamily.Monospace,
                modifier = Modifier
                    .clip(RoundedCornerShape(4.dp))
                    .background(bg)
                    .padding(horizontal = 8.dp, vertical = 4.dp),
            )
        }
        Row(
            Modifier
                .fillMaxWidth()
                .background(RdcColor.navyMid)
                .padding(horizontal = 8.dp, vertical = 6.dp),
        ) {
            listOf("" to 28, "Count" to 0, "Denom" to 52, "Amount" to 72, "Drop" to 52, "Left" to 64).forEach { (h, w) ->
                Text(
                    h,
                    color = RdcColor.onNavy,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    modifier = if (w == 0) Modifier.weight(1f) else Modifier.width(w.dp),
                    textAlign = if (h == "Count") TextAlign.Center else TextAlign.End,
                )
            }
        }
        Denom.entries.forEach { d ->
            val isFocus = focused == d
            Row(
                Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 8.dp, vertical = 2.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(d.rollLetter ?: "", color = RdcColor.ink, fontFamily = FontFamily.Monospace, fontSize = 12.sp, modifier = Modifier.width(28.dp))
                val qty = result.counts[d]
                Text(
                    if (qty == 0) " " else "$qty",
                    fontFamily = FontFamily.Monospace,
                    color = RdcColor.cellInk,
                    textAlign = TextAlign.Center,
                    modifier = Modifier
                        .weight(1f)
                        .height(40.dp)
                        .clip(RoundedCornerShape(6.dp))
                        .background(RdcColor.input)
                        .border(if (isFocus) 2.dp else 0.dp, if (isFocus) RdcColor.navyMid else Color.Transparent, RoundedCornerShape(6.dp))
                        .clickable { onFocus(d) }
                        .padding(vertical = 8.dp),
                )
                Text(d.label, color = RdcColor.ink, fontFamily = FontFamily.Monospace, fontSize = 12.sp, textAlign = TextAlign.End, modifier = Modifier.width(52.dp))
                Text(
                    if (qty == 0) "" else DropEngine.money(qty * d.cents),
                    fontFamily = FontFamily.Monospace,
                    fontSize = 12.sp,
                    color = RdcColor.ink,
                        .padding(vertical = 6.dp),
                )
                Text(
                    if (result.drop[d] == 0) "" else "${result.drop[d]}",
                    fontFamily = FontFamily.Monospace,
                    fontSize = 12.sp,
                    textAlign = TextAlign.End,
                    modifier = Modifier.width(52.dp),
                )
                Text(
                    if (result.left[d] == 0) "" else DropEngine.money(result.left[d] * d.cents),
                    fontFamily = FontFamily.Monospace,
                    fontSize = 12.sp,
                    textAlign = TextAlign.End,
                    modifier = Modifier.width(64.dp),
                )
            }
        }
        Row(
            Modifier
                .fillMaxWidth()
                .background(RdcColor.navyMid)
                .padding(8.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Text("Total", color = RdcColor.onNavy, fontSize = 12.sp, fontWeight = FontWeight.SemiBold, modifier = Modifier.weight(1f))
            Text(DropEngine.money(result.amountCents), color = RdcColor.onNavy, fontFamily = FontFamily.Monospace, fontSize = 12.sp)
            Text(DropEngine.money(result.dropCents), color = RdcColor.onNavy, fontFamily = FontFamily.Monospace, fontSize = 12.sp, modifier = Modifier.width(52.dp), textAlign = TextAlign.End)
            val leftBg = if (!result.hasCount) RdcColor.navyDeep else if (result.balanced) RdcColor.ok else RdcColor.bad
            val leftFg = if (!result.hasCount) RdcColor.onNavy else if (result.balanced) RdcColor.okInk else RdcColor.badInk
            Text(
                DropEngine.money(result.leftCents),
                color = leftFg,
                fontFamily = FontFamily.Monospace,
                fontSize = 12.sp,
                textAlign = TextAlign.End,
                modifier = Modifier
                    .width(64.dp)
                    .background(leftBg)
                    .padding(4.dp),
            )
        }
    }
}

@Composable
private fun NumberPad(onDigit: (String) -> Unit, onClear: () -> Unit, onEnter: () -> Unit) {
    val keys = listOf("1", "2", "3", "4", "5", "6", "7", "8", "9", "Clear", "0", "Enter")
    Column(
        Modifier
            .fillMaxWidth()
            .background(RdcColor.navyDeep)
            .padding(8.dp),
        verticalArrangement = Arrangement.spacedBy(6.dp),
    ) {
        keys.chunked(3).forEach { row ->
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                row.forEach { k ->
                    val bg = when (k) {
                        "Enter" -> RdcColor.navy
                        "Clear" -> RdcColor.sheet
                        else -> RdcColor.paper
                    }
                    val fg = if (k == "Enter") RdcColor.onNavy else RdcColor.ink
                    Box(
                        Modifier
                            .weight(1f)
                            .height(48.dp)
                            .clip(RoundedCornerShape(8.dp))
                            .background(bg)
                            .clickable {
                                when (k) {
                                    "Clear" -> onClear()
                                    "Enter" -> onEnter()
                                    else -> onDigit(k)
                                }
                            },
                        contentAlignment = Alignment.Center,
                    ) {
                        Text(k, color = fg, fontWeight = if (k == "Enter" || k == "Clear") FontWeight.SemiBold else FontWeight.Normal, fontFamily = if (k.length == 1) FontFamily.Monospace else FontFamily.Default, fontSize = if (k.length == 1) 20.sp else 14.sp)
                    }
                }
            }
        }
    }
}
