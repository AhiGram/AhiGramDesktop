## Obtaining your API credentials

To build your own version of **AhiGram Desktop**, you are required to provide your own **api_id** and **api_hash** for Telegram API access. This is a requirement from Telegram LLC for all third-party clients.

1. Sign in to your Telegram account here: [https://my.telegram.org](https://my.telegram.org)
2. Go to **"API development tools"** and fill out the form.
3. You will receive your unique **api_id** and **api_hash**.

### Development & Testing
If you are building AhiGram for personal testing or development purposes only, you can use the following test credentials. **Note:** These keys are heavily rate-limited.

- **api_id**: `17349`
- **api_hash**: `344583e45741c457fe1862106095a5eb`

> [!CAUTION]
> If you deploy a public release using these test credentials, users will encounter "Internal Server Errors" during login. Always use your own production keys for distribution.
