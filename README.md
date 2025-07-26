# CÂN ĐIỆN TỬ GIÁM SÁT TỪ XA
Dự án này là tự án cá nhân tự học, tôi nghiên cứu nhằm giải quyết vấn đề một số cân điện tử đặc biệt là cân hải sản kích thước lớn không có tính linh hoạt hoặc một số hoạt động trong môi trường ẩm ướt lâu này dễ làm hư hỏng bảng mạch chính.

File main.cpp là chương trình chính trong dự án, gồm chương trình liên quan đến Firebase (dùng để tạo giao tiếp với ứng dụng giám sát và điều khiển trên điện thoại), chương trình dùng để hiển thị trên led 7 đoạn bằng họ IC TM16XX cụ thể trong dự án này là TM1629A và TM1624 được tôi xây dựng lại dựa trên thư viện cùng họ là TM1628 của tác giả sky01, cũng như chương trình hỗ trợ chuyển đổi giao tiếp ESP32 với ứng dụng bằng Firebase qua giao tiếp BLE cung cấp điều kiện hoạt động nhiều hơn.

Ngoài các thư viện liên quan đến BLE, dự án cũng sử dụng thêm một số thư viện quan trọng: 
  **syk01/TM1628@^1.0.0
  bogde/HX711@^0.7.5
  mobizt/Firebase ESP32 Client@^4.4.17**
