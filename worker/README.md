# NSL Greenhouse Cloudflare Worker

Worker tối thiểu để xác nhận Cloudflare deployment.

## Deploy một lệnh

Từ thư mục gốc:

`bash deploy-cloudflare.sh`

Wrangler sẽ mở trình duyệt để đăng nhập Cloudflare nếu máy chưa đăng nhập, sau đó tạo/deploy Worker tên `nsl-greenhouse`.

Cloudflare sẽ cấp URL dạng:

`https://nsl-greenhouse.<account-subdomain>.workers.dev`

Nếu tài khoản Cloudflare của bạn có subdomain `xzort`, URL sẽ là:

`https://nsl-greenhouse.xzort.workers.dev`

## Lưu ý

Tên `xzort.workers.dev` là subdomain tài khoản Cloudflare. Script không thể tự chiếm/tạo subdomain này nếu tài khoản hiện tại chưa sở hữu nó; Cloudflare quyết định subdomain theo tài khoản đã đăng nhập.
