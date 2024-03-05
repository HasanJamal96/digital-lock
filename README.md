# digital-lock

This project creates a secure and versatile digital lock system using an ESP32. It offers user access management through both a physical keypad and a web-based captive portal, providing flexibility and convenience.  

## Hardware Components:
  - ESP32 microcontroller
  - 4x3 keypad with backlight
  - Buzzer
  - Relay
  - LED for status indication

## Features:
### User Creation:
  - Admins can create/delete users using either the keypad or the captive portal.
  - Keypad-created users have basic access.
### Captive portal features:
  - Add, Delete or Edit users
  - Manual control of relay.
  - Maximum password usage count for users
  - Scheduled access timeframes for users (Guest user)
  - Set schedules for automatic relay activation/deactivation.
  - Displays a history of user access attempts and relay schedules.
  - Modify admin password
  - Configure ESP32 access point name (SSID) and password
  - Synchronize system time