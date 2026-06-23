package com.example.light_control;

import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity3 extends AppCompatActivity {
    private TextView mq2TextView;
    private final boolean[] isOn = {false, false, false, false, false, false};
    boolean isFirebaseUpdate = false;
    boolean isFirebaseUpdate1 = false;
    boolean isFirebaseUpdate2 = false;
    boolean isFirebaseUpdate3 = false;
    boolean isFirebaseUpdate4 = false;
    private float maxWeightKg = 0f;
    private float CalibWeightKg = 0f;
    private EditText editTextCalibKg;
    private EditText editTextMaxKg;
    private ImageButton btnwarning;
    private ImageButton relay1;
    private ImageButton relay2;
    private ImageButton relay3;
    private ImageButton relay4;
    DatabaseReference databaseRefWeightKg;
    DatabaseReference btnwarningdata;
    DatabaseReference sw1data;
    DatabaseReference sw2data;
    DatabaseReference sw3data;
    DatabaseReference sw4data;
    DatabaseReference gasdata;
    Boolean btnwrdata;
    Boolean btnwrdata1;
    Boolean btnwrdata2;
    Boolean btnwrdata3;
    Boolean btnwrdata4;


    //    @SuppressLint("MissingInflatedId")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main3);

        Button btnlogout = findViewById(R.id.btnlogout);
        Button btn_DONGY2 = findViewById(R.id.btnDONGY2);

        editTextMaxKg = findViewById(R.id.textmaxkg);
        editTextCalibKg = findViewById(R.id.editTextNote);
        btnwarning = findViewById(R.id.buttonwarning);
        relay1 = findViewById(R.id.Relay1);
        relay2 = findViewById(R.id.Relay2);
        relay3 = findViewById(R.id.Relay3);
        relay4 = findViewById(R.id.Relay4);

        ImageButton btnBLE = findViewById(R.id.btnBLE);
        mq2TextView = findViewById(R.id.mq2TextView);
        btnwarningdata = FirebaseDatabase.getInstance().getReference().child("NUTNHAN5");
        sw1data = FirebaseDatabase.getInstance().getReference().child("NUTNHAN1");
        sw2data = FirebaseDatabase.getInstance().getReference().child("NUTNHAN2");
        sw3data = FirebaseDatabase.getInstance().getReference().child("NUTNHAN3");
        sw4data = FirebaseDatabase.getInstance().getReference().child("NUTNHAN4");
///candientu@gmail.com ///namdt@
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Xử lý nút Kết nối
        btn_DONGY2.setOnClickListener(v -> {
            String calibInput  = editTextCalibKg.getText().toString().trim();
            String maxInput  = editTextMaxKg.getText().toString().trim();
            if (calibInput.isEmpty()) {
                editTextCalibKg.setError("Vui lòng nhập khối lượng");
                return;
            }
            if (maxInput.isEmpty()) {
                editTextMaxKg.setError("Vui lòng nhập MAXCELL");
                return;
            }
            try {
                CalibWeightKg = Float.parseFloat(calibInput);
                maxWeightKg = Float.parseFloat(maxInput);

                if (CalibWeightKg <= 0) {
                    editTextCalibKg.setError("Khối lượng phải lớn hơn 0");
                    return;
                }

                databaseRefWeightKg = FirebaseDatabase.getInstance().getReference();
                databaseRefWeightKg.child("CALIBCELL").setValue(CalibWeightKg)
                        .addOnFailureListener(e -> Toast.makeText(MainActivity3.this, "Lỗi khi gửi dữ liệu: " + e.getMessage(), Toast.LENGTH_SHORT).show());
                databaseRefWeightKg.child("MAXCELL").setValue(maxWeightKg)
                        .addOnSuccessListener(aVoid ->
                                Toast.makeText(this, "Đã gửi thành công hai giá trị", Toast.LENGTH_SHORT).show())
                        .addOnFailureListener(e ->
                                Toast.makeText(this, "Lỗi gửi MAXCELL: " + e.getMessage(), Toast.LENGTH_SHORT).show());

            } catch (NumberFormatException e) {
                editTextCalibKg.setError("Giá trị không hợp lệ");
            }
        });
        //BUTTONWARNING
        btnwarningdata.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    isFirebaseUpdate = true;
                    btnwrdata = snapshot.getValue(Boolean.class);
                    isOn[5] = (btnwrdata != null && btnwrdata);
                    btnwarning.setSelected(isOn[5]);
                } else {
                    isOn[5] = false;
                    btnwarning.setSelected(false);
                }
            }
            public void onCancelled(@NonNull DatabaseError error) {
                //NOP
            }
        });
        btnwarning.setOnClickListener(v -> {
            if (isOn[5]) {
                btnwarning.setSelected(isOn[5]);
                btnwarningdata.setValue(false);
                isOn[5] = false;
            } else {
                btnwarning.setSelected(isOn[5]);
                btnwarningdata.setValue(true);
                isOn[5] = true;
            }
        });
        //RELAY1
        sw1data.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    isFirebaseUpdate1 = true;
                    btnwrdata1 = snapshot.getValue(Boolean.class);
                    isOn[0] = (btnwrdata1 != null && btnwrdata1);
                    relay1.setSelected(isOn[0]);
                }else {
                    isOn[0] = false;
                    relay1.setSelected(isOn[0]);
                }
            }
            public void onCancelled(@NonNull DatabaseError error) {
                //NOP
            }
        });
        relay1.setOnClickListener(v -> {
            if (isOn[0]) {
                relay1.setSelected(isOn[0]);
                sw1data.setValue(false);
                isOn[0] = false;
            } else {
                relay1.setSelected(isOn[0]);
                sw1data.setValue(true);
                isOn[0] = true;
            }
        });
        //RELAY2
        sw2data.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    isFirebaseUpdate2 = true;
                    btnwrdata2 = snapshot.getValue(Boolean.class);
                    isOn[1] = (btnwrdata2 != null && btnwrdata2);
                    relay2.setSelected(isOn[1]);
                }else {
                    isOn[1] = false;
                    relay2.setSelected(isOn[1]);
                }
            }
            public void onCancelled(@NonNull DatabaseError error) {
                //NOP
            }
        });
        relay2.setOnClickListener(v -> {
            if (isOn[1]) {
                relay2.setSelected(isOn[1]);
                sw2data.setValue(false);
                isOn[1] = false;
            } else {
                relay2.setSelected(isOn[1]);
                sw2data.setValue(true);
                isOn[1] = true;
            }
        });
        //RELAY3
        sw3data.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    isFirebaseUpdate3 = true;
                    btnwrdata3 = snapshot.getValue(Boolean.class);
                    isOn[2] = (btnwrdata3 != null && btnwrdata3);
                    relay3.setSelected(isOn[2]);
                }else {
                    isOn[2] = false;
                    relay3.setSelected(isOn[2]);
                }

            }
            public void onCancelled(@NonNull DatabaseError error) {
                //NOP
            }
        });
        relay3.setOnClickListener(v -> {
            if (isOn[2]) {
                relay3.setSelected(isOn[2]);
                sw3data.setValue(false);
                isOn[2] = false;
            } else {
                relay3.setSelected(isOn[2]);
                sw3data.setValue(true);
                isOn[2] = true;
            }
        });
        //RELAY4
        sw4data.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                if (snapshot.exists()) {
                    isFirebaseUpdate4 = true;
                    btnwrdata4 = snapshot.getValue(Boolean.class);
                    isOn[3] = (btnwrdata4 != null && btnwrdata4);
                    relay4.setSelected(isOn[3]);
                }else {
                    isOn[3] = false;
                    relay4.setSelected(isOn[3]);
                }

            }
            public void onCancelled(@NonNull DatabaseError error) {
                //NOP
            }
        });
        relay4.setOnClickListener(v -> {
            if (isOn[3]) {
                relay4.setSelected(isOn[3]);
                sw4data.setValue(false);
                isOn[3] = false;
            } else {
                relay4.setSelected(isOn[3]);
                sw4data.setValue(true);
                isOn[3] = true;
            }
        });
        //GAS
        gasdata = FirebaseDatabase.getInstance().getReference();
        gasdata.child("KHOILUONG").addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot snapshot) {
                mq2TextView.setText(String.format("%sKG", snapshot.getValue().toString()));
            }
            @Override
            public void onCancelled(@NonNull DatabaseError error) {
            }
        });

        btnBLE.setOnClickListener(v -> {
            Intent intent = new Intent(MainActivity3.this, MainActivity4.class);
            startActivity(intent);
        });
        // xu ly nut logout
        btnlogout.setOnClickListener(view -> finish());

    }
}
