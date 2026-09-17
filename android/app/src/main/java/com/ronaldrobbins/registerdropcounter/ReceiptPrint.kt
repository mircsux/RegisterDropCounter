package com.ronaldrobbins.registerdropcounter

import android.content.Context
import android.graphics.Paint
import android.graphics.Typeface
import android.os.Bundle
import android.os.CancellationSignal
import android.os.ParcelFileDescriptor
import android.print.PageRange
import android.print.PrintAttributes
import android.print.PrintDocumentAdapter
import android.print.PrintDocumentInfo
import android.print.PrintManager
import android.print.pdf.PrintedPdfDocument
import java.io.FileOutputStream

/** 80 mm Star TSC100 / TSP100 receipt: 3149 mils wide, 200 mm long. */
private val star80mm = PrintAttributes.MediaSize("RDC_80MM", "Star 80mm", 3150, 7874)

fun printDropSlip(context: Context, jobName: String, text: String) {
    val pm = context.getSystemService(Context.PRINT_SERVICE) as PrintManager
    val attrs = PrintAttributes.Builder()
        .setMediaSize(star80mm)
        .setMinMargins(PrintAttributes.Margins(118, 118, 118, 433))
        .setColorMode(PrintAttributes.COLOR_MODE_MONOCHROME)
        .build()
    pm.print(jobName, ReceiptPrintAdapter(context, text), attrs)
}

private class ReceiptPrintAdapter(
    private val context: Context,
    private val text: String,
) : PrintDocumentAdapter() {
    private var doc: PrintedPdfDocument? = null

    override fun onLayout(
        oldAttributes: PrintAttributes?,
        newAttributes: PrintAttributes,
        cancellationSignal: CancellationSignal?,
        callback: LayoutResultCallback,
        extras: Bundle?,
    ) {
        doc?.close()
        doc = PrintedPdfDocument(context, newAttributes)
        if (cancellationSignal?.isCanceled == true) {
            callback.onLayoutCancelled()
            return
        }
        callback.onLayoutFinished(
            PrintDocumentInfo.Builder("drop-slip.pdf")
                .setContentType(PrintDocumentInfo.CONTENT_TYPE_DOCUMENT)
                .setPageCount(1)
                .build(),
            true,
        )
    }

    override fun onWrite(
        pages: Array<PageRange>,
        destination: ParcelFileDescriptor,
        cancellationSignal: CancellationSignal?,
        callback: WriteResultCallback,
    ) {
        val document = doc ?: run {
            callback.onWriteFailed("no document")
            return
        }
        val page = document.startPage(0)
        val canvas = page.canvas
        val paint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
            typeface = Typeface.MONOSPACE
            color = android.graphics.Color.BLACK
            textSize = 9f
        }
        val usable = canvas.width - 16f
        var size = 9f
        val sample = "0".repeat(DropEngine.RECEIPT_COLS)
        while (paint.measureText(sample) > usable && size > 6f) {
            size -= 0.5f
            paint.textSize = size
        }
        var y = 12f + paint.textSize
        val lh = paint.fontSpacing
        for (line in text.split('\n')) {
            if (cancellationSignal?.isCanceled == true) {
                document.finishPage(page)
                callback.onWriteCancelled()
                return
            }
            canvas.drawText(line, 8f, y, paint)
            y += lh
        }
        document.finishPage(page)
        try {
            document.writeTo(FileOutputStream(destination.fileDescriptor))
            callback.onWriteFinished(arrayOf(PageRange.ALL_PAGES))
        } catch (e: Exception) {
            callback.onWriteFailed(e.message)
        }
    }

    override fun onFinish() {
        doc?.close()
        doc = null
    }
}
