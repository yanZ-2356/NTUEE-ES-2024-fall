/*
 * Copyright 2024 Punch Through Design LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.punchthrough.blestarterappandroid

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGattCharacteristic
import android.content.Context
import android.os.Bundle
import android.os.SystemClock
import android.view.MenuItem
import android.view.View
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.constraintlayout.widget.ConstraintLayout
import com.punchthrough.blestarterappandroid.ble.ConnectionEventListener
import com.punchthrough.blestarterappandroid.ble.ConnectionManager
import com.punchthrough.blestarterappandroid.ble.ConnectionManager.parcelableExtraCompat
import com.punchthrough.blestarterappandroid.ble.isIndicatable
import com.punchthrough.blestarterappandroid.ble.isNotifiable
import com.punchthrough.blestarterappandroid.ble.isReadable
import com.punchthrough.blestarterappandroid.ble.isWritable
import com.punchthrough.blestarterappandroid.ble.isWritableWithoutResponse
import com.punchthrough.blestarterappandroid.ble.toHexString
import com.punchthrough.blestarterappandroid.databinding.ActivityBleOperationsBinding
import java.math.BigInteger
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.UUID

class BleOperationsActivity : AppCompatActivity() {
    private val minUUID = UUID.fromString("00000000-0001-11e1-ac36-0002a5d5c51b")
    private val targetUUID = UUID.fromString("01000000-0001-11e1-ac36-0002a5d5c51b")

    private var minVal = 499
    private var targetVal = 2000

    private lateinit var minTextView: TextView
    private lateinit var targetTextView: TextView
    private lateinit var minSeekBar: SeekBar
    private lateinit var targetSeekBar: SeekBar
    private var lastWriteTime: Long = 0 // prevent rapid sending write req

    private lateinit var binding: ActivityBleOperationsBinding
    private val device: BluetoothDevice by lazy {
        intent.parcelableExtraCompat(BluetoothDevice.EXTRA_DEVICE)
            ?: error("Missing BluetoothDevice from MainActivity!")
    }
    private val dateFormatter = SimpleDateFormat("MMM d, HH:mm:ss", Locale.US)
    private val characteristics by lazy {
        ConnectionManager.servicesOnDevice(device)?.flatMap { service ->
            service.characteristics ?: listOf() }?.filter { it.uuid.equals(minUUID) || it.uuid.equals(targetUUID)
        } ?: listOf()
    }
    private val characteristicProperties by lazy {
        characteristics.associateWith { characteristic ->
            mutableListOf<CharacteristicProperty>().apply {
                if (characteristic.isNotifiable()) add(CharacteristicProperty.Notifiable)
                if (characteristic.isIndicatable()) add(CharacteristicProperty.Indicatable)
                if (characteristic.isReadable()) add(CharacteristicProperty.Readable)
                if (characteristic.isWritable()) add(CharacteristicProperty.Writable)
                if (characteristic.isWritableWithoutResponse()) {
                    add(CharacteristicProperty.WritableWithoutResponse)
                }
            }.toList()
        }
    }
    private val notifyingCharacteristics = mutableListOf<UUID>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        ConnectionManager.registerListener(connectionEventListener)

        binding = ActivityBleOperationsBinding.inflate(layoutInflater)

        setContentView(binding.root)
        supportActionBar?.apply {
            setDisplayHomeAsUpEnabled(true)
            setDisplayShowTitleEnabled(true)
            title = getString(R.string.config_page_title)
        }

        val min_view = findViewById<ConstraintLayout>(R.id.min_view)
        val target_view = findViewById<ConstraintLayout>(R.id.target_view)
        minTextView =  findViewById(R.id.min_textview)
        targetTextView  = findViewById(R.id.target_textview)
        minSeekBar = findViewById(R.id.min_seekbar)
        targetSeekBar = findViewById(R.id.target_seekbar)

        // set the correct range for the seekbars
        minSeekBar.max = 800
        minSeekBar.min = 200
        targetSeekBar.max = 4000
        targetSeekBar.min = 500

        // set up listeners
//        min_view.setOnClickListener { showCharacteristicOptions(characteristics[0]) }
//        target_view.setOnClickListener { showCharacteristicOptions(characteristics[1]) }
        minSeekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                if (!fromUser) return
                if (SystemClock.uptimeMillis() - lastWriteTime < 100) return // prevent rapid firing writing request
                val bytes = String.format("%04x", progress).hexToBytes()
                log("Writing to min: ${bytes.toHexString()}")
                minVal = progress
                ConnectionManager.writeCharacteristic(device, characteristics[0], bytes)
                lastWriteTime = SystemClock.uptimeMillis()
            }
            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {
                val progress = seekBar.progress
                val bytes = String.format("%04x", progress).hexToBytes()
                log("Writing to min: ${bytes.toHexString()}")
                minVal = progress
                ConnectionManager.writeCharacteristic(device, characteristics[0], bytes)
            }
        })
        targetSeekBar.setOnSeekBarChangeListener(object : OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                if (!fromUser) return
                if (kotlin.math.abs(SystemClock.uptimeMillis() - lastWriteTime) < 100) return // prevent rapid firing writing request
                val bytes = String.format("%04x", progress).hexToBytes()
                log("Writing to target: ${bytes.toHexString()}")
                targetVal = progress
                ConnectionManager.writeCharacteristic(device, characteristics[1], bytes)
                lastWriteTime = SystemClock.uptimeMillis()
            }
            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {
                val progress = seekBar.progress
                val bytes = String.format("%04x", progress).hexToBytes()
                log("Writing to target: ${bytes.toHexString()}")
                targetVal = progress
                ConnectionManager.writeCharacteristic(device, characteristics[1], bytes)
            }
        })

        // get the initial values of the settings
        ConnectionManager.readCharacteristic(device, characteristics[0])
        ConnectionManager.readCharacteristic(device, characteristics[1])
    }

    override fun onDestroy() {
        ConnectionManager.unregisterListener(connectionEventListener)
        ConnectionManager.teardownConnection(device)
        super.onDestroy()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            android.R.id.home -> {
                onBackPressed()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    @SuppressLint("SetTextI18n")
    private fun log(message: String) {
//        val formattedMessage = "${dateFormatter.format(Date())}: $message"
//        runOnUiThread {
//            val uiText = binding.logTextView.text
//            val currentLogText = uiText.ifEmpty { "Beginning of log." }
//            binding.logTextView.text = "$currentLogText\n$formattedMessage"
//            binding.logScrollView.post { binding.logScrollView.fullScroll(View.FOCUS_DOWN) }
//        }
    }

    private fun showCharacteristicOptions(
        characteristic: BluetoothGattCharacteristic
    ) = runOnUiThread {
        characteristicProperties[characteristic]?.let { properties ->
            AlertDialog.Builder(this)
                .setTitle("Select an action to perform")
                .setItems(properties.map { it.action }.toTypedArray()) { _, i ->
                    when (properties[i]) {
                        CharacteristicProperty.Readable -> {
                            log("Reading from ${characteristic.uuid}")
                            ConnectionManager.readCharacteristic(device, characteristic)
                        }
                        CharacteristicProperty.Writable, CharacteristicProperty.WritableWithoutResponse -> {
                            showWritePayloadDialog(characteristic)
                        }
                        CharacteristicProperty.Notifiable, CharacteristicProperty.Indicatable -> {
                            if (notifyingCharacteristics.contains(characteristic.uuid)) {
                                log("Disabling notifications on ${characteristic.uuid}")
                                ConnectionManager.disableNotifications(device, characteristic)
                            } else {
                                log("Enabling notifications on ${characteristic.uuid}")
                                ConnectionManager.enableNotifications(device, characteristic)
                            }
                        }
                    }
                }
                .show()
        }
    }

    @SuppressLint("InflateParams")
    private fun showWritePayloadDialog(characteristic: BluetoothGattCharacteristic) {
        val hexField = layoutInflater.inflate(R.layout.edittext_hex_payload, null) as EditText
        AlertDialog.Builder(this)
            .setView(hexField)
            .setPositiveButton("Write") { _, _ ->
                with(hexField.text.toString()) {
                    if (isNotBlank() && isNotEmpty()) {
                        val bytes = String.format("%04x", toInt()).hexToBytes()
                        log("Writing to ${characteristic.uuid}: ${bytes.toHexString()}")
                        if (characteristic.uuid.equals(minUUID)) {
                            minVal = toInt()
                        } else if (characteristic.uuid.equals(targetUUID)) {
                            targetVal = toInt()
                        }
                        ConnectionManager.writeCharacteristic(device, characteristic, bytes)
                    } else {
                        log("Please enter a hex payload to write to ${characteristic.uuid}")
                    }
                }
            }
            .setNegativeButton("Cancel", null)
            .create()
            .apply {
                window?.setSoftInputMode(
                    WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE
                )
                hexField.showKeyboard()
                show()
            }
    }

    private val connectionEventListener by lazy {
        ConnectionEventListener().apply {
            onDisconnect = {
                runOnUiThread {
                    AlertDialog.Builder(this@BleOperationsActivity)
                        .setTitle("Disconnected")
                        .setMessage("Disconnected from device.")
                        .setPositiveButton("OK") { _, _ -> onBackPressed() }
                        .show()
                }
            }

            onCharacteristicRead = { _, characteristic, value ->
                log("Read from ${characteristic.uuid}: ${value.toHexString()}")
                val t = BigInteger(value).toInt()
                runOnUiThread {
                    if (characteristic.uuid.equals(minUUID)) {
                        minVal = t
                        minTextView.text = "$t"
                        minSeekBar.progress = t
                    } else if (characteristic.uuid.equals(targetUUID)) {
                        targetVal = t
                        targetTextView.text = "$t"
                        targetSeekBar.progress = t
                    }
                }
            }

            onCharacteristicWrite = { _, characteristic ->
                log("Wrote to ${characteristic.uuid}")
                runOnUiThread {
                    if (characteristic.uuid.equals(minUUID)) {
                        minTextView.text = "$minVal"
                    } else if (characteristic.uuid.equals(targetUUID)) {
                        targetTextView.text = "$targetVal"
                    }
                }
            }

            onMtuChanged = { _, mtu ->
                log("MTU updated to $mtu")
            }

            onCharacteristicChanged = { _, characteristic, value ->
                log("Value changed on ${characteristic.uuid}: ${value.toHexString()}")
            }

            onNotificationsEnabled = { _, characteristic ->
                log("Enabled notifications on ${characteristic.uuid}")
                notifyingCharacteristics.add(characteristic.uuid)
            }

            onNotificationsDisabled = { _, characteristic ->
                log("Disabled notifications on ${characteristic.uuid}")
                notifyingCharacteristics.remove(characteristic.uuid)
            }
        }
    }

    private enum class CharacteristicProperty {
        Readable,
        Writable,
        WritableWithoutResponse,
        Notifiable,
        Indicatable;

        val action
            get() = when (this) {
                Readable -> "Read"
                Writable -> "Write"
                WritableWithoutResponse -> "Write Without Response"
                Notifiable -> "Toggle Notifications"
                Indicatable -> "Toggle Indications"
            }
    }

    private fun Activity.hideKeyboard() {
        hideKeyboard(currentFocus ?: View(this))
    }

    private fun Context.hideKeyboard(view: View) {
        val inputMethodManager = getSystemService(Activity.INPUT_METHOD_SERVICE) as InputMethodManager
        inputMethodManager.hideSoftInputFromWindow(view.windowToken, 0)
    }

    private fun EditText.showKeyboard() {
        val inputMethodManager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
        requestFocus()
        inputMethodManager.showSoftInput(this, InputMethodManager.SHOW_IMPLICIT)
    }

    private fun String.hexToBytes() =
        this.chunked(2).map { it.uppercase(Locale.US).toInt(16).toByte() }.toByteArray()
}
