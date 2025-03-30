package com.tcc0lin.aura.ui.theme

import android.app.Activity
import android.os.Build
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.material3.dynamicDarkColorScheme
import androidx.compose.material3.dynamicLightColorScheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext

private val DarkColorScheme = darkColorScheme(
    primary = Purple80,
    secondary = PurpleGrey80,
    tertiary = Pink80
)

private val LightColorScheme = lightColorScheme(
    primary = Color(0xFF006782),
    onPrimary = Color(0xFFFFFFFF),
    primaryContainer = Color(0xFFB5EAFF),
    onPrimaryContainer = Color(0xFF001F29),

    secondary = Color(0xFF4C626B),
    onSecondary = Color(0xFFFFFFFF),
    secondaryContainer = Color(0xFFCFE6F1),
    onSecondaryContainer = Color(0xFF071E26),

    tertiary = Color(0xFF5A5B7D),
    onTertiary = Color(0xFFFFFFFF),
    tertiaryContainer = Color(0xFFE1E0FF),
    onTertiaryContainer = Color(0xFF171837),

    error = Color(0xFFBA1B1B),
    onError = Color(0xFFFFFFFF),
    errorContainer = Color(0xFFFFDAD4),
    onErrorContainer = Color(0xFF410001),

    background = Color(0xFFF9FDFF),
    onBackground = Color(0xFF191C1D),
    surface = Color(0xFFF9FDFF),
    onSurface = Color(0xFF191C1D),
    surfaceVariant = Color(0xFFF9FDFF),
    onSurfaceVariant = Color(0xFF40484C),
    outline = Color(0xFF70787C)
)

@Composable
fun AuraTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    // Dynamic color is available on Android 12+
    dynamicColor: Boolean = true,
    content: @Composable () -> Unit
) {
//    val colorScheme = when {
//        dynamicColor && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S -> {
//            val context = LocalContext.current
//            if (darkTheme) dynamicDarkColorScheme(context) else dynamicLightColorScheme(context)
//        }
//
//        darkTheme -> DarkColorScheme
//        else -> LightColorScheme
//    }

    val colorScheme = LightColorScheme

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        content = content
    )
}