# Web dashboard

Đây là dashboard web để xem dữ liệu từ Supabase và thay đổi ngưỡng điều khiển.

## Cấu hình
Mở `index.html` và truyền URL/key qua hash:

`index.html#url=https://YOUR_PROJECT.supabase.co&key=YOUR_SUPABASE_ANON_KEY`

Hoặc lưu `url` và `key` vào localStorage bằng DevTools.

## Deploy
Có thể bật GitHub Pages cho thư mục web bằng GitHub Actions hoặc đổi cấu trúc để dùng Vercel/Netlify.

Không đưa Supabase service_role key vào frontend. Frontend chỉ dùng anon key + RLS.
