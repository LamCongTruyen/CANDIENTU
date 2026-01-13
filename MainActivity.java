package com.example.light_control;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import com.example.light_control.R;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.example.light_control.R;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.AuthResult;
import com.google.firebase.auth.FirebaseAuth;

public class MainActivity extends AppCompatActivity {
    private EditText editemail;
    private EditText editpass;
    private Button btnlogin;
    private FirebaseAuth mAuth;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // khởi tạo
        mAuth=FirebaseAuth.getInstance();
        // tiến hành ánh xạ
        editemail=findViewById(R.id.editemail);
        editpass=findViewById(R.id.editpass);
        btnlogin=findViewById(R.id.btnlogin);

        // tiến hành kiểm tra nút login
        btnlogin.setOnClickListener(view -> login());
    }
    private void login(){
        String email,pass;
        email=editemail.getText().toString();
        pass=editpass.getText().toString();
        // xử lý nếu bạn bỏ trống email
        if(TextUtils.isEmpty(email)){
            Toast.makeText(this,"Vui lòng nhập Email!",Toast.LENGTH_SHORT).show();
            return;
        }
        // xử lý nếu bạn bỏ trống mật khẩu
        if(TextUtils.isEmpty(pass)){
            Toast.makeText(this,"Vui lòng nhập mật khẩu !",Toast.LENGTH_SHORT).show();
            return;
        }

        mAuth.signInWithEmailAndPassword(email,pass).addOnCompleteListener(new OnCompleteListener<AuthResult>() {
            @Override
            public void onComplete(@NonNull Task<AuthResult> task) {
                if (task.isSuccessful()){
                    Toast.makeText(getApplicationContext(),"Đăng nhập thành công",Toast.LENGTH_SHORT).show();
                    Intent intent= new Intent(MainActivity.this, MainActivity3.class);
                    startActivity(intent);
                }
                else{
                    Toast.makeText(getApplicationContext(),"Đăng nhập không thành công",Toast.LENGTH_SHORT).show();
                }

            }
        });

    }


}