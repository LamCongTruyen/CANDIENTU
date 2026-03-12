# CÂN ĐIỆN TỬ - Dự Án Tự Học

# Cập nhật ngày 10/03/2026 : 
Dự án làm mạch cân điện tử kết hợp hiển thị không dây với ESP32 có vấn đề về xử lý tiêu thụ năng lượng với các loại cân nhỏ, cân di động, cân treo. 
Hiện tại tôi đã chuyển sang xử dụng STM32F1 thay thế cho dòng ESP32 làm MCU chính xử lý các tác vụ đọc ADC HX711-ADS1230, gửi dữ liệu cho IC giải mã led 7 đoạn. 
Với các thay đổi như vậy thì dòng điện tiêu thụ khi sử dụng ESP32 khoảng 140mA giảm xuống còn khoảng 80mA khi hoạt động bình thường, và tắt LED khi không có cân nặng trên Loadcell để giảm điện năng tiêu thụ thì dòng điện treo còn khoảng 40mA theo những gì tôi đo được từ đồng hồ điện tử. Thay đổi như vậy tương đương với các sản phẩm hiện có trên thị trường, tương lai khi hoàn thiện phần cứng tôi sẽ cập nhật thêm.

# Cập nhật ngày 12/03/2026 : 
Kết hợp thêm 1 nút nhấn tại chân SYS-WKUP, trong trường hợp không có cân nặng đặt trên Loadcell thì sau 3p sẽ tắt LED, nối tiếp sau đó 3p nếu vẫn không có thay đổi thì STM32 sẽ vào Low power Mode - Standby Mode.

# Ưu điểm (hoạt động ở 5V - 16Mhz) :
- Tiết kiệm điện tối đa [ Nomal: ~81mA , OffLedd; ~40mA, StandbyMode: ~10uA (chưa đo được chính xác) ]
- Dễ tinh chỉnh cấu hình phù hợp theo yêu cầu người dùng - sửa chữa - thay thế
- Gía thành ổn
- Sử dụng IC hiển thị TM1629A có thể thay thế - sửa chữa cho nhiều dòng cân
- ADC HX711-ADS1230 có sẵn trên nhiều dòng cân có thể tận dụng lại

# Nhược điểm :
- Cần thêm 1 PCB nhỏ
  
# Hình ảnh phần cứng thực hiện kiểm tra độ ổn định:

![z7614591732828_4812db6e06ef42b3008b904f9e274086](https://github.com/user-attachments/assets/6bb186e7-45fb-47e4-9d3a-65b3e4b4d108)

# Video test

Link:https://youtu.be/9Hn5jxMhens

# Trước tháng 12/2025
Dự án này là dự án cá nhân tự học, tôi nghiên cứu nhằm giải quyết vấn đề một số cân điện tử đặc biệt là cân hải sản kích thước lớn không có tính linh hoạt hoặc một số hoạt động trong môi trường ẩm ướt lâu này dễ làm hư hỏng bảng mạch chính.

File main.cpp là chương trình chính trong dự án, gồm chương trình liên quan đến Firebase (dùng để tạo giao tiếp với ứng dụng giám sát và điều khiển trên điện thoại), chương trình dùng để hiển thị trên led 7 đoạn bằng họ IC TM16XX cụ thể trong dự án này là TM1629A và TM1624 được tôi xây dựng lại dựa trên thư viện cùng họ là TM1628 của tác giả sky01, cũng như chương trình hỗ trợ chuyển đổi giao tiếp ESP32 với ứng dụng bằng Firebase qua giao tiếp BLE cung cấp điều kiện hoạt động nhiều hơn.

Ngoài các thư viện liên quan đến BLE, dự án cũng sử dụng thêm một số thư viện quan trọng: 
  **syk01/TM1628@^1.0.0
  bogde/HX711@^0.7.5
  mobizt/Firebase ESP32 Client@^4.4.17**

Giống như một số dự án trước, trong dự án này tôi cũng xây dựng một chương trình ứng dụng Android giúp giám sát và điều khiển từ xa:

<img width="373" height="803" alt="image" src="https://github.com/user-attachments/assets/a20edd5f-00d5-4952-9e9a-1a384d5c2c18" />

<img width="373" height="803" alt="image" src="https://github.com/user-attachments/assets/9ee37c36-c8a3-4287-96fb-e64d73f79a87" />

<img width="373" height="803" alt="image" src="https://github.com/user-attachments/assets/fe96a1e2-447c-44c4-bd72-2d0c50c66544" />

<img width="373" height="803" alt="image" src="https://github.com/user-attachments/assets/7300cda9-70f7-4761-81f4-c5857bffa1f5" />

Hiện tại 27/07/25, dự án vẫn đang trong quá trình hoàn thiện thêm một số chức năng và tối ưu hoạt động. Thời gian tới tôi sẽ update nếu có sự thay đổi gì đó đáng kể, xin chân thành cảm ơn bạn đã xem.

Hiện tại 04/08/2025, dự án có một số thay đổi đó là tôi không còn sử dụng Firebase nhằm thực hiện giao tiếp không dây nữa bởi vì trong khả năng tài chính của tôi thì không đủ kinh phí để trả phí cho dịch vụ. Và điều quan trọng nhất đó là khi thực hiện giao tiếp mạng trong một khoảng thời gian thì Token liên tục bị mất hiệu lực và tôi không tìm được cách để xử lí tình huống nào do đó tôi chuyển qua sử dụng giao tiếp BLE để gửi,nhận dữ liệu giữa ESP32 và App. Bên cạnh đó tôi cũng sử dụng thêm giao thức ESPNOW để gửi,nhận dữ liệu giữa hai ESP. Điều này được thực hiện sau khi tôi thấy Firebase không còn phù hợp và tôi nghĩ ra rằng mình nên tạo một bảng điều khiển và hiển thị LED 7 đoạn được kết nối không dây với Loadcells sẽ phục vụ tốt trong những môi trường ẩm ướt như cân thủy sản,... . Và với khoảng cách hỗ trợ của hai giao thứ này (BLE,ESPNOW) cũng khá lý tưởng (khoảng 50-200m ở điều kiện tốt) thì cũng hỗ trợ sử dụng ở thực tế tốt hơn.

Đến ngày 24/08/25 tôi bổ sung thêm chức năng "Deep sleep" cho khối hiển thị và giao tiếp app đồn thời "Light Sleep" cho khối đọc ADC. Từ đó tăng khả năng tiết kiệm năng lượng nhưng vẫn cho ra chức năn vượt trội so với các sản phẩm trên thị trường.

Chương trình sửa đổi trên ESP32 được tôi thêm vào với tên file là: main_useBLE_ESPNOW_1, main_useBLE_ESPNOW_2. Sắp tới tôi cũng sẽ tinh chỉnh một tí ở giao diện app và sẽ cập nhật lên.

Tôi nghĩ rằng trong tương lai nếu khách hàng có yêu cầu thì tôi sẽ phát triển lại phần Firebase cho dự án này.
