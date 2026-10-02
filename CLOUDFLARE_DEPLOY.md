# Deploy NSL Greenhouse lên Cloudflare Pages

## Cách 1 — Cloudflare Dashboard (khuyến nghị cho lần đầu)

1. Tạo Supabase project và chạy `database/schema.sql`.
2. Vào Cloudflare Dashboard → Workers & Pages → Create application → Pages → Import an existing Git repository.
3. Chọn repository `bonnydark/du-an-nha-kinh-2026-2027-NSL`.
4. Production branch: `main`.
5. Build command: để trống.
6. Build output directory: `web`.
7. Deploy.
8. Sau khi có domain `*.pages.dev`, mở `/setup.html`, nhập Supabase URL + anon key.
9. Tạo tài khoản trong Supabase Authentication → Users rồi đăng nhập.

## Cách 2 — GitHub Actions tự deploy

Tạo Cloudflare Pages project tên `nsl-greenhouse`, sau đó tạo GitHub Actions secrets:

- `CLOUDFLARE_API_TOKEN`: API token có quyền Cloudflare Pages Edit.
- `CLOUDFLARE_ACCOUNT_ID`: Account ID của Cloudflare.

Workflow `.github/workflows/cloudflare-pages.yml` sẽ tự deploy thư mục `web` mỗi lần push vào `main`.

## Bảo mật

- Chỉ đưa Supabase anon/publishable key vào frontend.
- Tuyệt đối không commit Supabase service_role/secret key.
- Production nên bật Email confirmation và RLS theo user.
