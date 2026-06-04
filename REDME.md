# HỆ THỐNG CUNG CẤP NƯỚC VÀ THỨC ĂN TỰ ĐỘNG CHO VẬT NUÔI

## 📖 Giới thiệu

Đây là dự án xây dựng hệ thống cung cấp thức ăn và nước uống tự động cho vật nuôi sử dụng vi điều khiển ESP32.

Hệ thống có khả năng theo dõi lượng thức ăn và nước trong máng, tự động bổ sung khi xuống dưới ngưỡng cài đặt, đồng thời hỗ trợ giám sát và quản lý từ xa thông qua giao diện Web.

Dự án được thực hiện trong học phần **Nông nghiệp Thông minh**, nhằm ứng dụng công nghệ IoT vào lĩnh vực chăn nuôi để giảm công lao động và nâng cao hiệu quả quản lý.

---

## 📋 Thông tin dự án

| Nội dung            | Thông tin                                              |
| ------------------- | ------------------------------------------------------ |
| Tên đề tài          | Hệ thống cung cấp nước và thức ăn tự động cho vật nuôi |
| Vi điều khiển       | ESP32 DevKit V1                                        |
| Lĩnh vực            | IoT - Nông nghiệp thông minh                           |
| Ngôn ngữ            | C/C++, HTML, CSS, JavaScript                           |
| Trường              | Đại học Đại Nam                                        |
| Sinh viên thực hiện | Đoàn Minh Châu                                         |

---

## 🚀 Chức năng chính

### 🍖 Cấp thức ăn tự động

* Sử dụng Loadcell và HX711 để đo khối lượng thức ăn trong máng.
* Khi lượng thức ăn nhỏ hơn **100g**:

  * Servo tự động mở cửa kho thức ăn.
  * Cấp thêm khoảng **500g thức ăn**.
* Tự động đóng cửa sau khi cấp đủ lượng thức ăn.
* Lưu lịch sử cho ăn.

### 💧 Cấp nước tự động

* Sử dụng cảm biến mực nước để đo lượng nước trong máng.
* Khi mực nước nhỏ hơn **5cm**:

  * Relay kích hoạt bơm nước.
* Khi mực nước đạt **20cm**:

  * Bơm tự động dừng.

### 📦 Giám sát nguyên liệu

* Theo dõi lượng thức ăn còn lại trong kho.
* Theo dõi lượng nước còn lại trong bể chứa.
* Cảnh báo khi nguyên liệu xuống thấp.

### 🌐 Quản lý từ xa

* Web Dashboard chạy trực tiếp trên ESP32.
* Theo dõi dữ liệu thời gian thực bằng điện thoại hoặc máy tính.
* Xem lịch sử cấp thức ăn.
* Xem lịch sử bơm nước.
* Đồng bộ thời gian hệ thống.
* Xóa lịch sử hoạt động.
* Khởi tạo lại lượng thức ăn và nước.

---

## 🛠 Công nghệ sử dụng

### Phần cứng

* ESP32 DevKit V1
* Loadcell 5kg
* Module HX711
* Servo SG90
* Relay 1 kênh
* Bơm mini DC 12V
* Water Level Sensor
* LCD 16x2 I2C
* RTC DS1302

### Phần mềm

* Arduino IDE
* HTML
* CSS
* JavaScript
* ESP32 Web Server

---

## 🏗 Sơ đồ khối hệ thống

```text
                    +------------------+
                    |   Kho thức ăn    |
                    +--------+---------+
                             |
                             v
+-----------+      +------------------+
| Loadcell  |----->|                  |
| (Máng ăn) |      |                  |
+-----------+      |                  |
                   |      ESP32       |
+-----------+      |                  |
| Loadcell  |----->|                  |
| (Kho ăn)  |      |                  |
+-----------+      +--------+---------+
                             |
           +-----------------+----------------+
           |                                  |
           v                                  v

   +---------------+                 +---------------+
   | Servo SG90    |                 | Relay 1 Kênh  |
   +---------------+                 +-------+-------+
                                             |
                                             v
                                      +-------------+
                                      | Bơm nước DC |
                                      +------+------+ 
                                             |
                                             v
                                      +-------------+
                                      | Máng nước   |
                                      +-------------+

           ^
           |
+--------------------+
| Water Sensor       |
+--------------------+

           |
           v

+--------------------+
| LCD 16x2 I2C       |
+--------------------+

           |
           v

+--------------------+
| Web Dashboard      |
| Mobile / PC        |
+--------------------+
```

---

## ⚙️ Nguyên lý hoạt động

### 🍖 Cấp thức ăn

1. Đọc dữ liệu từ Loadcell.
2. Nếu lượng thức ăn trong máng < 100g.
3. Servo mở cửa kho.
4. Thức ăn được cấp xuống máng.
5. Khi đạt khoảng 500g.
6. Servo đóng cửa.
7. Lưu lịch sử cấp thức ăn.

### 💧 Cấp nước

1. Đọc dữ liệu từ cảm biến mực nước.
2. Nếu mực nước < 5cm.
3. Relay bật.
4. Bơm hoạt động.
5. Khi nước đạt 20cm.
6. Relay tắt.
7. Bơm dừng.

---

## 🌐 Giao diện Web

Web Dashboard được lưu trực tiếp trên ESP32.

### Chức năng

* Hiển thị thời gian thực
* Hiển thị lượng thức ăn
* Hiển thị lượng nước
* Hiển thị lượng thức ăn còn lại
* Hiển thị lượng nước còn lại
* Đồng bộ thời gian
* Reset kho thức ăn và nước
* Xóa lịch sử hoạt động
* Theo dõi lịch sử cho ăn
* Theo dõi lịch sử bơm nước

---

## 📂 Cấu trúc chương trình

```text
Project
│
├── ESP32 Controller
├── HX711
│   ├── Loadcell máng ăn
│   └── Loadcell kho thức ăn
├── Water Sensor
├── Servo Control
├── Relay Control
├── LCD Display
├── RTC DS1302
├── Web Server
└── Data Management
    ├── Feed History
    └── Water History
```

---

## 📚 Thư viện sử dụng

```cpp
WiFi.h
WebServer.h
HX711.h
ESP32Servo.h
LiquidCrystal_I2C.h
ThreeWire.h
RtcDS1302.h
Wire.h
```

---

## 🔧 Hướng dẫn cài đặt

### Bước 1

Cài đặt Arduino IDE.

### Bước 2

Cài đặt ESP32 Board:

```text
Tools → Board Manager → ESP32 by Espressif Systems
```

### Bước 3

Cài đặt thư viện:

* HX711
* ESP32Servo
* LiquidCrystal_I2C
* RtcDS1302

### Bước 4

Cập nhật thông tin WiFi:

```cpp
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

### Bước 5

Upload chương trình lên ESP32.

### Bước 6

Mở Serial Monitor để xem địa chỉ IP.

Ví dụ:

```text
172.20.10.11
```

### Bước 7

Truy cập trình duyệt bằng địa chỉ IP của ESP32.

---

## ✅ Kết quả đạt được

* Tự động cấp thức ăn khi dưới 100g.
* Tự động cấp khoảng 500g thức ăn.
* Tự động cấp nước từ 5cm đến 20cm.
* Hiển thị dữ liệu trên LCD.
* Giám sát qua điện thoại.
* Lưu lịch sử hoạt động.
* Hỗ trợ quản lý từ xa bằng WiFi.

---

## 🔮 Hướng phát triển

* Kết nối Cloud.
* Ứng dụng Android.
* Thông báo Telegram/Zalo.
* Camera giám sát vật nuôi.
* Phân tích dữ liệu bằng AI.
* Thống kê tiêu thụ thức ăn và nước theo ngày.

---

## 👨‍🎓 Thông tin thực hiện

**Sinh viên:** Đoàn Minh Châu

**Khoa:** Công nghệ Thông tin

**Trường:** Đại học Đại Nam
