# Dự án nhà kính 2026-2027 NSL

Hệ thống giám sát và tự động hóa nhà kính dùng ESP32.

## Chức năng
- Đọc độ ẩm đất cho hàng cây A/B.
- Đọc nhiệt độ/độ ẩm môi trường.
- Phát hiện khói bằng MQ-2.
- Phát hiện mưa để theo dõi mái che.
- Điều khiển bơm và quạt qua relay.
- Còi và LED cảnh báo.
- Hiển thị dữ liệu trên LCD I2C.
- Kết nối Wi-Fi và cung cấp web dashboard nội bộ.

## Sơ đồ kết nối mặc định

| Thiết bị | ESP32 |
|---|---|
| Soil A | GPIO 34 |
| Soil B | GPIO 35 |
| DHT22 DATA | GPIO 4 |
| MQ-2 AO | GPIO 36 |
| Rain AO | GPIO 39 |
| Relay bơm | GPIO 26 |
| Relay quạt | GPIO 27 |
| Buzzer | GPIO 25 |
| LED | GPIO 2 |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |

> Các chân có thể thay đổi trong `src/main.cpp`. Kiểm tra module thực tế trước khi cấp nguồn.

## Chạy
Mở project bằng PlatformIO và nạp firmware cho ESP32. Điền SSID/password Wi-Fi trong `src/main.cpp`.

Sau khi ESP32 kết nối mạng, Serial Monitor sẽ in địa chỉ IP. Mở IP đó trên điện thoại/máy tính cùng mạng Wi-Fi để xem dashboard.
# - subnet eb https://30e71ebd.nsl-greenhouse.pages.dev/
