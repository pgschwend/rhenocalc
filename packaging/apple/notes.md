
## Notes for Apple Packaging

codesign --force --timestamp --options runtime --sign "Apple Distribution: Rhenosys GmbH (JJNH23H5Y5)" dist/macos/RhenoCalc.app/Contents/Frameworks/*

codesign --force --timestamp --options runtime --sign "Apple Distribution: Rhenosys GmbH (JJNH23H5Y5)" dist/macos/RhenoCalc.app/Contents/PlugIns/*/*

codesign --force --timestamp --options runtime --entitlements packaging/apple/Entitlements.plist --sign "Apple Distribution: Rhenosys GmbH (JJNH23H5Y5)" dist/macos/RhenoCalc.app