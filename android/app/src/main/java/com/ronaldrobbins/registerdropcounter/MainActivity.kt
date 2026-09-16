package com.ronaldrobbins.registerdropcounter

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.History
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material.icons.outlined.Settings
import androidx.compose.material.icons.outlined.Calculate
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent { DropCounterApp() }
    }
}

@Composable
fun DropCounterApp(model: AppModel = viewModel()) {
    var tab by rememberSaveable { mutableIntStateOf(0) }
    Scaffold(
        containerColor = RdcColor.sheet,
        bottomBar = {
            NavigationBar(containerColor = RdcColor.navy, contentColor = RdcColor.onNavy) {
                val items = listOf(
                    Triple("Counter", Icons.Outlined.Calculate, 0),
                    Triple("History", Icons.Outlined.History, 1),
                    Triple("Options", Icons.Outlined.Settings, 2),
                    Triple("About", Icons.Outlined.Info, 3),
                )
                items.forEach { (label, icon, index) ->
                    NavigationBarItem(
                        selected = tab == index,
                        onClick = { tab = index },
                        icon = { Icon(icon, contentDescription = label) },
                        label = { Text(label) },
                        colors = NavigationBarItemDefaults.colors(
                            selectedIconColor = RdcColor.navy,
                            selectedTextColor = RdcColor.onNavy,
                            indicatorColor = RdcColor.paper,
                            unselectedIconColor = RdcColor.onNavy.copy(alpha = 0.7f),
                            unselectedTextColor = RdcColor.onNavy.copy(alpha = 0.7f),
                        ),
                    )
                }
            }
        },
    ) { pad ->
        when (tab) {
            0 -> CounterScreen(model, Modifier.padding(pad))
            1 -> HistoryScreen(model, Modifier.padding(pad))
            2 -> OptionsScreen(model, Modifier.padding(pad))
            else -> AboutScreen(Modifier.padding(pad))
        }
    }
}
