package com.tcc0lin.aura

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import com.tcc0lin.aura.MyApplication.Companion.appContext
import com.tcc0lin.aura.component.CheckCard
import com.tcc0lin.aura.component.IconHintCard
import com.tcc0lin.aura.detectors.FridaDetector
import kotlinx.coroutines.*

val snapShotList = mutableStateListOf<Triple<IDetector, IDetector.Result?, Detail?>>(
    Triple(FridaDetector(appContext), null, null),
)

suspend fun runDetector(id: Int, packages: Collection<String>?) {
    withContext(Dispatchers.IO) {
        val detail = mutableListOf<Pair<String, IDetector.Result>>()
        val result = snapShotList[id].first.run(packages, detail)
        snapShotList[id] = Triple(snapShotList[id].first, result, detail)
    }
}

@Composable
fun MainPage(modifier: Modifier) {
    LaunchedEffect(appContext) {
        while (true) {
            runDetector(0, null)
            delay(10_000)
        }
    }

    Column(
        modifier = modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState()),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        IconHintCard()
        snapShotList.forEach {
            CheckCard(
                title = it.first.name,
                result = it.second,
                detail = it.third
            )
        }
    }
}
