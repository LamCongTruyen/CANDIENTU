package com.example.light_control;
import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.pm.PackageManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;

import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;



import java.util.UUID;

public class MainActivity4 extends AppCompatActivity{

    private BluetoothAdapter bluetoothAdapter;
    private BluetoothGatt bluetoothGatt;
    private TextView statusText;
    private TextView bleDataTextView;
    private Button connectBtn;
    private EditText inputData;
    private EditText inputData2;
    private Button relay1;
    private Button relay2;
    private final String data_tare = "TARE";
    private final String data_calib = "CALIBRATE";
    private static final int REQUEST_BLUETOOTH_PERMISSION = 1;
    public static final UUID SERVICE_UUID = UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
    public static final UUID CHARACTERISTIC_UUID = UUID.fromString("beb5483e-36e1-4688-b7f5-ea07361b26a8");
    public static final UUID CLIENT_CHARACTERISTIC_CONFIG =
            UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    @RequiresApi(api = Build.VERSION_CODES.S)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main4);
        // Khởi tạo Bluetooth
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        BluetoothManager bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        bluetoothAdapter = bluetoothManager.getAdapter();
        connectBtn = findViewById(R.id.btn_connect_bt);
        Button disconnectBtn = findViewById(R.id.btn_disconnect_bt);
        Button sendBtn = findViewById(R.id.btn_send_data);
        statusText = findViewById(R.id.bt_status);
        bleDataTextView = findViewById(R.id.mq2TextView);
        inputData = findViewById(R.id.input_data);
        inputData2 = findViewById(R.id.input_data2);
        relay1 = findViewById(R.id.Relay1);
        relay2 = findViewById(R.id.Relay2);
        ImageButton btnBack = findViewById(R.id.btnBack);

        ///candientu@gmail.com ///namdt@
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        relay2.setOnClickListener(null);
        relay2.setOnClickListener(v -> {
            if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            sendData_button(data_calib);
            relay2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#DDDDDD"))); // Màu nhấn
            new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
                relay2.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#dc143c"))); // Màu ban đầu
            }, 200);
        });
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        relay1.setOnClickListener(null);
        relay1.setOnClickListener(v -> {
            if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            sendData_button(data_tare);
            relay1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#DDDDDD"))); // Màu nhấn
            new android.os.Handler(Looper.getMainLooper()).postDelayed(() -> {
                relay1.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#dc143c"))); // Màu ban đầu
            }, 200);
        });
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //xử lý nút quay lại
        btnBack.setOnClickListener(view -> finish());
        if (!bluetoothAdapter.isEnabled()) {
            Toast.makeText(this, "Vui lòng bật Bluetooth", Toast.LENGTH_LONG).show();
            return;
        }
        // Yêu cầu quyền
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED ||
                ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED ||
                ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{
                    Manifest.permission.BLUETOOTH_SCAN,
                    Manifest.permission.BLUETOOTH_CONNECT,
                    Manifest.permission.ACCESS_FINE_LOCATION
            }, 1);
        }
        // Xử lý nút Kết nối
        connectBtn.setOnClickListener(v -> startBleScan());
        // Xử lý nút Ngắt kết nối
        disconnectBtn.setOnClickListener(v -> disconnectGatt());
        // Xử lý nút Gửi dữ liệu
        sendBtn.setOnClickListener(v -> {
            if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(MainActivity4.this, "Yêu cầu quyền Bluetooth", Toast.LENGTH_SHORT).show();
                return;
            }
            String calibrationWeight = inputData.getText().toString().trim();
            String maxWeight = inputData2.getText().toString().trim();
            if (calibrationWeight.isEmpty() || maxWeight.isEmpty()) {
                Toast.makeText(MainActivity4.this, "Vui lòng nhập cả hai thông số", Toast.LENGTH_SHORT).show();
                return;
            }
            try {
                float calWeight = Float.parseFloat(calibrationWeight);
                float maxWeightVal = Float.parseFloat(maxWeight);
                if (calWeight <= 0 || maxWeightVal <= 0) {
                    Toast.makeText(MainActivity4.this, "Thông số phải lớn hơn 0", Toast.LENGTH_SHORT).show();
                    return;
                }
                // Định dạng chuỗi: "CAL:<calibrationWeight>,MAX:<maxWeight>"
                String data = "CAL" + calWeight + "MAX" + maxWeightVal;
                sendData(data);
                Toast.makeText(MainActivity4.this, "Gửi dữ liệu: " + data, Toast.LENGTH_SHORT).show();
            } catch (NumberFormatException e) {
                Toast.makeText(MainActivity4.this, "Thông số phải là số hợp lệ", Toast.LENGTH_SHORT).show();
            }
        });
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_BLUETOOTH_PERMISSION) {
            if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                connectBtn.performClick(); // Thử lại kết nối khi được cấp quyền
            } else {
                statusText.setText(R.string.trang_thai_bluetooth_quyenbituchoi);
            }
        }
    }
    ////////////////////////
    private void startBleScan() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_SCAN) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        statusText.setText(R.string.trang_thai_bluetooth_dangquet);
        BluetoothLeScanner scanner = bluetoothAdapter.getBluetoothLeScanner();
        if (scanner == null) {
            Toast.makeText(this, "Không thể khởi động quét Bluetooth", Toast.LENGTH_LONG).show();
            return;
        }
        scanner.startScan(new ScanCallback() {
            @Override
            public void onScanResult(int callbackType, ScanResult result) {
                BluetoothDevice device = result.getDevice();
                if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                    return;
                }
                // Kết nối khi tìm thấy thiết bị
                if ("ESP32-BLE-Server".equals(device.getName())) { // Lọc thiết bị ESP32
                    scanner.stopScan(this);
                    statusText.setText(R.string.trang_thai_bluetooth_dang_ketnoi);
                    bluetoothGatt = device.connectGatt(MainActivity4.this, false, gattCallback); // Lưu bluetoothGatt
                }
            }
            @Override
            public void onScanFailed(int errorCode) {
                runOnUiThread(() -> Toast.makeText(MainActivity4.this, "Quét thất bại", Toast.LENGTH_SHORT).show());
            }
        });
    }
    ////////////////////////
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                return;
            }
            runOnUiThread(() -> {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    statusText.setText(R.string.trang_thai_ket_noi_da_ketnoi);
                    bluetoothGatt = gatt;
                    gatt.discoverServices();
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    statusText.setText(R.string.trang_thai_ket_noi_chua_ket_noi);
                    if (bluetoothGatt != null) {
                        bluetoothGatt.close();
                        bluetoothGatt = null;
                    }
                }
            });
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                BluetoothGattService service = gatt.getService(SERVICE_UUID);
                if (service != null) {
                    BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
                    if (characteristic != null) {
                        if (ActivityCompat.checkSelfPermission(MainActivity4.this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
                            return;
                        }
                        // Bật notification
                        gatt.setCharacteristicNotification(characteristic, true);
                        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(CLIENT_CHARACTERISTIC_CONFIG);
                        if (descriptor != null) {
                            descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                            gatt.writeDescriptor(descriptor);
                        }
                    }
                }
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
            runOnUiThread(() -> {
                // Đọc dữ liệu từ characteristic
                byte[] data = characteristic.getValue();
                String dataString = new String(data); // Giả sử dữ liệu là chuỗi
                bleDataTextView.setText(String.format("%sKG", dataString));
            });
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                runOnUiThread(() -> Toast.makeText(MainActivity4.this, "Đã bật notification", Toast.LENGTH_LONG).show());
            }
        }
    };

    ////////////////////////
    private void sendData_button(String Data) {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        if (bluetoothGatt == null) {
            Toast.makeText(this, "Chưa kết nối với thiết bị", Toast.LENGTH_SHORT).show();
        }else{
            BluetoothGattService service = bluetoothGatt.getService(SERVICE_UUID);
            if (service == null) {
                Toast.makeText(this, "Không tìm thấy service", Toast.LENGTH_SHORT).show();
                return;
            }
            BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
            if (characteristic == null) {
                Toast.makeText(this, "Không tìm thấy characteristic", Toast.LENGTH_SHORT).show();
                return;
            }
            // Gửi dữ liệu
            characteristic.setValue(Data.getBytes());
            characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
            boolean success = bluetoothGatt.writeCharacteristic(characteristic);
            if (success) {
                Toast.makeText(this, "Gửi dữ liệu thành công", Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(this, "Gửi dữ liệu thất bại", Toast.LENGTH_LONG).show();
            }
        }
    }

    private void sendData(String data) {
        if (bluetoothGatt == null) {
            Toast.makeText(this, "Chưa kết nối với thiết bị", Toast.LENGTH_SHORT).show();
            return;
        }

//        String data = inputData.getText().toString().trim();
        if (data == null || data.trim().isEmpty()) {
            Toast.makeText(this, "Vui lòng nhập dữ liệu", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothGattService service = bluetoothGatt.getService(SERVICE_UUID);
        if (service == null) {
            Toast.makeText(this, "Không tìm thấy service", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothGattCharacteristic characteristic = service.getCharacteristic(CHARACTERISTIC_UUID);
        if (characteristic == null) {
            Toast.makeText(this, "Không tìm thấy characteristic", Toast.LENGTH_SHORT).show();
            return;
        }

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        // Gửi dữ liệu
        characteristic.setValue(data.getBytes());
        characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);
        boolean success = bluetoothGatt.writeCharacteristic(characteristic);
        if (success) {
            Toast.makeText(this, "Gửi dữ liệu thành công", Toast.LENGTH_LONG).show();
        } else {
            Toast.makeText(this, "Gửi dữ liệu thất bại", Toast.LENGTH_LONG).show();
        }
    }
    private void disconnectGatt() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            return;
        }
        if (bluetoothGatt != null) {
            bluetoothGatt.disconnect();
            bluetoothGatt.close();
            bluetoothGatt = null;
            statusText.setText(R.string.trangthai);
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnectGatt();
        statusText.setText(R.string.trang_thai_bluetooth_ngatketnoi);
    }
}
