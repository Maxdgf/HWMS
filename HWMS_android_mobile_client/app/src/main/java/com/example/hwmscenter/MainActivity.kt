package com.example.hwmscenter

/*
=====================================================================
----------------|Simple HWMS🌤️ Android Parser App|-------------------
----------------| by https://github.com/Maxdgf |---------------------
=====================================================================
|    Shows current weather data and device ip from HWMS web page.   |
 */

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.basicMarquee
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults.topAppBarColors
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.hwmscenter.Modules.HWMS_api_service
import com.example.hwmscenter.ui.theme.HWMSCenterTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

//data refresh delay
private const val REFRESH_DELAY: Long = 1000

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            HWMSCenterTheme {
                //setting main content screen
                MainContent()
            }
        }
    }
}

//main content screen
@Preview
@Composable
fun MainContent() {
    val hwms = HWMS_api_service()

    var mainData by remember { mutableStateOf("Loading...\n(check wifi connection)") } //default text
    var currentIpData by remember { mutableStateOf("current ip: ?") } //default text

    val dataScope = rememberCoroutineScope()

    LaunchedEffect(Unit) {
        //launching coroutine
        dataScope.launch(Dispatchers.IO) {
            while (true) {
                val currentIp = hwms.fetchHWMScurrentIp()
                val mainValues = hwms.fetchHWMSvalues()

                withContext(Dispatchers.Main) {
                    currentIpData = currentIp
                    mainData = mainValues
                }

                delay(REFRESH_DELAY)
            }
        }
    }

    Scaffold(
        topBar = { AppTopBar() },
        modifier = Modifier.fillMaxSize()
    ) { innerPadding ->
        Box(
            modifier = Modifier
                .padding(innerPadding)
                .fillMaxSize()
        ) {
            Text(
                text = "Track temperature\uD83C\uDF21\uFE0F, humidity\uD83D\uDCA7 and atmospheric pressure☁\uFE0F here!",
                modifier = Modifier
                    .padding(top = 20.dp)
                    .align(Alignment.TopCenter)
                    .basicMarquee(iterations = Int.MAX_VALUE)
            )

            Text(
                text = mainData,
                modifier = Modifier.align(Alignment.Center),
                textAlign = TextAlign.Center,
                fontSize = 13.sp
            )

            Text(
                text = currentIpData,
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(10.dp),
                textAlign = TextAlign.Center,
                fontStyle = FontStyle.Italic
            )
        }
    }
}

//app top bar with text logo
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AppTopBar() {
    TopAppBar(
        title = {
            Text(
                text = "HWMS\uD83C\uDF24\uFE0F center",
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onPrimary,
                fontStyle = FontStyle.Italic,
                fontWeight = FontWeight.Bold
            )
        },
        colors = topAppBarColors(containerColor = MaterialTheme.colorScheme.primary)
    )
}