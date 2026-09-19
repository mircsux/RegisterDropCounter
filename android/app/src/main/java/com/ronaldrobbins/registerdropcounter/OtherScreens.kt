package com.ronaldrobbins.registerdropcounter

import android.content.Intent
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

@Composable
fun HistoryScreen(model: AppModel, modifier: Modifier = Modifier) {
    var selected by remember { mutableStateOf<HistoryEntry?>(null) }
    Column(
        modifier
            .fillMaxSize()
            .background(RdcColor.sheet)
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
    ) {
        Text("History", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg, fontSize = 20.sp)
        Text("Each Clear saves the sheet as it was.", color = RdcColor.muted, fontSize = 13.sp)
        Spacer(Modifier.height(12.dp))
        if (model.history.isEmpty()) {
            Text("No snapshots. Clear a register on Counter and it will show up here.", color = RdcColor.muted)
        }
        model.history.forEach { e ->
            val whenStr = SimpleDateFormat("EEE MMM d, yyyy  h:mm a", Locale.US).format(Date(e.at))
            val label = if (e.kind == HistoryEntry.ALL) "Cleared all registers"
            else "Cleared ${model.names.getOrElse(e.registerIndex ?: 0) { "R${(e.registerIndex ?: 0) + 1}" }}"
            Column(
                Modifier
                    .fillMaxWidth()
                    .padding(vertical = 8.dp)
                    .clickable { selected = e },
            ) {
                Text(whenStr, fontWeight = FontWeight.SemiBold, color = RdcColor.ink)
                Text(label, color = RdcColor.muted, fontSize = 13.sp)
            }
            HorizontalDivider(color = RdcColor.grid)
        }
        if (model.history.isNotEmpty()) {
            Spacer(Modifier.height(16.dp))
            Button(
                onClick = { model.clearHistory() },
                colors = ButtonDefaults.buttonColors(containerColor = RdcColor.bad, contentColor = RdcColor.badInk),
            ) { Text("Clear history") }
        }
    }
    selected?.let { e ->
        AlertDialog(
            onDismissRequest = { selected = null },
            title = { Text("Snapshot") },
            text = {
                Column {
                    e.registers.forEachIndexed { i, c ->
                        val r = DropEngine.compute(c, e.base)
                        val st = if (!r.hasCount) "Empty" else if (r.balanced) "OK" else "Off"
                        Text("${model.names[i]}  ${DropEngine.money(r.amountCents)}  $st", fontFamily = FontFamily.Monospace, fontSize = 13.sp)
                    }
                }
            },
            confirmButton = {
                TextButton(onClick = { model.restore(e.id); selected = null }) { Text("Restore") }
            },
            dismissButton = {
                TextButton(onClick = { selected = null }) { Text("Close") }
            },
        )
    }
}

@Composable
fun DropSlipDialog(model: AppModel, index: Int, onClose: () -> Unit) {
    val clip = LocalClipboardManager.current
    val ctx = LocalContext.current
    val r = model.result(index)
    val text = slipText(model, index, r)
    Dialog(onDismissRequest = { model.saveSlipFields(); onClose() }) {
        Column(
            Modifier
                .background(RdcColor.paper)
                .padding(20.dp)
                .verticalScroll(rememberScrollState()),
        ) {
            Text("Register Drop Slip", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
            Text("Star TSC100  ·  80 mm / 42 col", color = RdcColor.muted, fontSize = 12.sp)
            Spacer(Modifier.height(8.dp))
            Text(text, fontFamily = FontFamily.Monospace, fontSize = 11.sp, color = RdcColor.ink)
            Spacer(Modifier.height(8.dp))
            OutlinedTextField(value = model.bag, onValueChange = { model.bag = it }, label = { Text("Bag / seal #") }, singleLine = true)
            OutlinedTextField(value = model.initials, onValueChange = { model.initials = it }, label = { Text("Initials") }, singleLine = true)
            Spacer(Modifier.height(8.dp))
            Row {
                Button(
                    onClick = {
                        model.saveSlipFields()
                        clip.setText(AnnotatedString(slipText(model, index, r)))
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = RdcColor.navy),
                ) { Text("Copy") }
                Spacer(Modifier.padding(8.dp))
                Button(
                    onClick = {
                        model.saveSlipFields()
                        printDropSlip(ctx, "Drop Slip ${model.names[index]}", slipText(model, index, r))
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = RdcColor.navy),
                ) { Text("Print") }
                Spacer(Modifier.padding(8.dp))
                Button(
                    onClick = {
                        model.saveSlipFields()
                        val send = Intent(Intent.ACTION_SEND).apply {
                            type = "text/plain"
                            putExtra(Intent.EXTRA_SUBJECT, "Register Drop Slip ${model.names[index]}")
                            putExtra(Intent.EXTRA_TEXT, slipText(model, index, r))
                        }
                        ctx.startActivity(Intent.createChooser(send, "Print or share drop slip"))
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = RdcColor.navy),
                ) { Text("Share") }
                Spacer(Modifier.padding(8.dp))
                TextButton(onClick = { model.saveSlipFields(); onClose() }) { Text("Close") }
            }
        }
    }
}

private fun slipText(model: AppModel, index: Int, r: RegisterResult): String {
    return DropEngine.dropSlipText(
        till = model.names[index],
        baseDollars = model.base,
        r = r,
        bag = model.bag,
        initials = model.initials,
    )
}

@Composable
fun OptionsScreen(model: AppModel, modifier: Modifier = Modifier) {
    val clip = LocalClipboardManager.current
    Column(
        modifier
            .fillMaxSize()
            .background(RdcColor.sheet)
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
    ) {
        Text("Options", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg, fontSize = 20.sp)
        Spacer(Modifier.height(12.dp))
        Text("Appearance", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
        Row(
            Modifier
                .fillMaxWidth()
                .clickable { model.setDarkMode(!model.darkMode) }
                .padding(vertical = 8.dp),
        ) {
            Text("Dark mode", color = RdcColor.ink, modifier = Modifier.weight(1f))
            Text(if (model.darkMode) "On" else "Off", color = RdcColor.navyFg, fontWeight = FontWeight.SemiBold)
        }
        Text("Dark mode uses a night theme: teal chrome, carbon cards, and amber count cells.", color = RdcColor.muted, fontSize = 13.sp)
        Spacer(Modifier.height(16.dp))
        Text("Register base", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
        Row {
            DropEngine.baseOptions.forEach { b ->
                val on = model.base == b
                Text(
                    DropEngine.money(b * 100),
                    color = if (on) RdcColor.onNavy else RdcColor.navyFg,
                    modifier = Modifier
                        .padding(end = 8.dp, top = 8.dp)
                        .background(if (on) RdcColor.navy else RdcColor.paper)
                        .clickable { model.setBase(b) }
                        .padding(horizontal = 10.dp, vertical = 8.dp),
                    fontSize = 13.sp,
                )
            }
        }
        Spacer(Modifier.height(20.dp))
        Text("Cash log", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
        Text("Tap a till to copy that row for the national log.", color = RdcColor.muted, fontSize = 13.sp)
        Spacer(Modifier.height(8.dp))
        Text("Deposit (drop)", fontWeight = FontWeight.Medium, color = RdcColor.ink)
        repeat(DropEngine.REGISTER_COUNT) { i ->
            Text(
                "${model.names[i]}   Copy",
                color = RdcColor.ink,
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { clip.setText(AnnotatedString(DropEngine.oneCashLogTsv(model.result(i), true))) }
                    .padding(vertical = 8.dp),
            )
        }
        Text("EOD drawer", fontWeight = FontWeight.Medium, color = RdcColor.ink)
        repeat(DropEngine.REGISTER_COUNT) { i ->
            Text(
                "${model.names[i]}   Copy",
                color = RdcColor.ink,
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { clip.setText(AnnotatedString(DropEngine.oneCashLogTsv(model.result(i), false))) }
                    .padding(vertical = 8.dp),
            )
        }
    }
}

@Composable
fun AboutScreen(modifier: Modifier = Modifier) {
    Column(
        modifier
            .fillMaxSize()
            .background(RdcColor.sheet)
            .verticalScroll(rememberScrollState()),
    ) {
        Column(
            Modifier
                .fillMaxWidth()
                .background(RdcColor.navy)
                .padding(16.dp),
        ) {
            Text("Register Drop Counter", color = RdcColor.onNavy, fontSize = 20.sp, fontWeight = FontWeight.SemiBold)
            Text("Designed by Ronald Robbins Jr and SuperGrok", color = RdcColor.onNavy)
            Text("Version 2.31.0  (2026-09-19)", color = RdcColor.onNavy.copy(alpha = 0.85f), fontFamily = FontFamily.Monospace, fontSize = 12.sp)
        }
        Column(Modifier.padding(16.dp)) {
            Text("Count a drawer", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
            Text("Set the register base. Open a till (R1–R10). Tap the name to rename it. Type counts in the yellow cells with the number pad. After $100, Enter wraps back to pennies on the same till. Amount, Drop, and Left fill in. Left turns green when it equals the base.", color = RdcColor.ink)
            Spacer(Modifier.height(12.dp))
            Text("The drop", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
            Text("$100, $50, $20, $10, $5, $2, $1, then quarters, dimes, nickels, rolls, pennies. Loose coins drop before rolls.", color = RdcColor.ink)
            Spacer(Modifier.height(12.dp))
            Text("Changelog", fontWeight = FontWeight.SemiBold, color = RdcColor.navyFg)
            Text("v2.31.0  Visual Studio 2022 build compiles after the Windows dark-mode paint path.", color = RdcColor.ink)
            Text("v2.30.0  Windows 10 dark mode paints title bar, buttons, tabs, and lists.", color = RdcColor.ink)
            Text("v2.29.0  Options and About live under File.", color = RdcColor.ink)
            Text("v2.28.0  File > Stats for Nerds on the Windows desktop app.", color = RdcColor.ink)
            Text("v2.27.0  Current release on web, Windows, iPhone, Android, and C23.", color = RdcColor.ink)
            Text("v2.26.0  Dark mode is a night theme: carbon, teal, and amber.", color = RdcColor.ink)
            Text("v2.25.0  File > Stats for Nerds charts drop trends from History on the web app.", color = RdcColor.ink)
            Text("v2.24.0  Dark mode on desktop: gold count cells and readable amount columns.", color = RdcColor.ink)
            Text("v2.23.0  Header keeps File, Clear all, and Undo. Downloads live on About in the web app.", color = RdcColor.ink)
            Text("v2.22.0  Tab after $100 wraps to pennies on the same till.", color = RdcColor.ink)
            Text("v2.21.0  Current release on web, Windows, iPhone, Android, and C23.", color = RdcColor.ink)
            Text("v2.20.0  Sample drops do not write History snapshots.", color = RdcColor.ink)
            Text("v2.19.0  Sample drops stay under $6,000 per till.", color = RdcColor.ink)
            Text("v2.18.0  File > Load Sample Drops fills every till with random test counts.", color = RdcColor.ink)
            Text("v2.17.0  Drop slip shows drop total and left in drawer at the top.", color = RdcColor.ink)
            Text("v2.16.0  Web app — unzip and open index.html in any browser.", color = RdcColor.ink)
            Text("v2.15.0  Dark mode polish — count and amount cells go dark.", color = RdcColor.ink)
            Text("v2.14.1  Visual Studio build — History till selection and till-name editor.", color = RdcColor.ink)
            Text("v2.14.0  Dark mode on the Options tab.", color = RdcColor.ink)
            Text("v2.13.0  Star TSC100 drop slip — 80 mm / 42-column receipt.", color = RdcColor.ink)
            Text("v2.12.0  Android app (Kotlin + Jetpack Compose). Same drop math as Windows, iPhone, and C23.", color = RdcColor.ink)
            Text("v2.11.0  ISO C23 console app.", color = RdcColor.ink)
            Text("v2.10.0  iPhone app (SwiftUI).", color = RdcColor.ink)
            Text("v2.9.0  Named tills — tap R1–R10 to rename.", color = RdcColor.ink)
        }
    }
}
