package com.tcc0lin.aura

import android.annotation.SuppressLint
import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.net.Uri
import android.os.Bundle
import android.os.IBinder
import android.os.RemoteException
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.ClickableText
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.EmojiObjects
import androidx.compose.material.icons.outlined.Info
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.CenterAlignedTopAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExtendedFloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.LocalTextStyle
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat.startActivity
import com.tcc0lin.aura.ui.theme.AuraTheme

class MainActivity : ComponentActivity() {
    private val TAG = "[Aura Detector]"

    companion object {
        var mRemoteService: IRemoteService? = null
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            AuraTheme {
                var showDialog by remember { mutableStateOf(false) }
                if (showDialog) AboutDialog { showDialog = false }
                Scaffold(
                    topBar = { MainTopBar() },
                    floatingActionButton = { MainFab { showDialog = true } },
                ) { innerPadding ->
                    MainPage(Modifier.padding(innerPadding))
                }
            }
        }
        initServiceClient()
    }

    fun initServiceClient() {
        Log.i(TAG, "MainActivity: " + android.os.Process.myPid() + " onServiceInit");
        var connection = object : ServiceConnection {
            override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
                Log.i(TAG, "MainActivity onServiceConnected")
                try {
                    mRemoteService = IRemoteService.Stub.asInterface(service)
                } catch (e: RemoteException) {
                    Log.i(TAG, "RemoteException: ${e.message}")
                } catch (e: Exception) {
                    Log.i(TAG, "General Exception: ${e.message}")
                }
            }

            override fun onServiceDisconnected(name: ComponentName?) {
                Log.i(TAG, "MainActivity onServiceDisconnected")
            }
        }
        bindService(Intent(this, RemoteService::class.java), connection, Context.BIND_AUTO_CREATE)
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun MainTopBar() {
    CenterAlignedTopAppBar(title = { Text("Aura Detector") })
}

@Composable
private fun MainFab(onClick: () -> Unit) {
    ExtendedFloatingActionButton(
        icon = { Icon(Icons.Outlined.EmojiObjects, "About") },
        text = { Text("About") },
        onClick = onClick
    )
}

@Composable
private fun AboutDialog(onDismiss: () -> Unit) {
    val context = LocalContext.current
    AlertDialog(
        containerColor = Color(0xF0F8FFFF),
        onDismissRequest = onDismiss,
        confirmButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(android.R.string.ok))
            }
        },
        title = { Text("About") },
        text = {
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                CompositionLocalProvider(LocalTextStyle provides MaterialTheme.typography.bodyLarge) {
                    Text("Aura Detector")
                    Text("Authored by tcc0lin")
                }
                Spacer(Modifier.height(10.dp))
                val annotatedString = buildAnnotatedString {
                    pushStringAnnotation("GitHub", "https://github.com/tcc0lin/aura")
                    withStyle(SpanStyle(color = MaterialTheme.colorScheme.primary)) {
                        append("Source link")
                    }
                    pop()
                    append("    ")
                }
                ClickableText(
                    annotatedString, style = MaterialTheme.typography.bodyLarge
                ) { offset ->
                    annotatedString.getStringAnnotations("GitHub", offset, offset).firstOrNull()
                        ?.let {
                            startActivity(
                                context, Intent(Intent.ACTION_VIEW, Uri.parse(it.item)), null
                            )
                        }
                }
            }
        },
    )
}