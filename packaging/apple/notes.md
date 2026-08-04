
## Notes for Apple Packaging

### 1. Create target directory (if needed) and create fresh build
```bash
scripts/deploy_macos.sh
```

### 2. Verify the package signature locally (Optional)
```bash
pkgutil --check-signature dist/macos/rhenocalc-macos-0.x.x.pkg
```

### 3. Validate the package against App Store requirements (Optional)
```bash
xcrun altool --validate-app -f dist/macos/rhenocalc-macos-0.x.x.pkg -t macos -u "APPLE_MAIL" -p "APPLE_PASSWORD"
```

### 4. Upload the package to App Store Connect
```bash
xcrun altool --upload-app -f dist/macos/rhenocalc-macos-0.x.x.pkg -t macos -u "APPLE_MAIL" -p "APPLE_PASSWORD"
```


