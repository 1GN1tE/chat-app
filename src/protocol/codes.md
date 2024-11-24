# Overview

# Protocol Structure

```lua
+----------------------------+----------------------------------------------------------+
| Type (1 byte)              | Message type:                                            |
|                            | 0x01: Request                                            |
|                            | 0x02: Response                                           |
|                            | Other types could be added in the future                 |
+----------------------------+----------------------------------------------------------+
| Command Code (1 byte)      | Command identifier (e.g., 0x01: Login, 0x02: Logout)     |
+----------------------------+----------------------------------------------------------+
| Payload Size (2 bytes)     | Length of the payload (in bytes, e.g., 0x0F = 15 bytes)  |
+----------------------------+----------------------------------------------------------+
| Payload                    | Actual message data or content (e.g., "Login successful")|
|                            | It could be a string or any other type of data           |
+----------------------------+----------------------------------------------------------+

```

# Codes

## Request Codes

| Command                              | Request Code | Example Request                               | Raw Request                                                   |
|--------------------------------------|--------------|-----------------------------------------------|---------------------------------------------------------------|
| **Login with username and password** | `0x10`       | `!login Alice securepass123`                  | `[0x01] [0x10] [0x12] [Alice securepass123]`                  |
| **Set new password**                 | `0x11`       | `!setpass newsecurepass456`                   | `[0x01] [0x11] [0x14] [newsecurepass456]`                     |
| **Change nickname**                  | `0x12`       | `!nick Alice_Wonderland`                      | `[0x01] [0x12] [0x14] [Alice_Wonderland]`                     |
| **List available channels**          | `0x20`       | `!listc`                                      | `[0x01] [0x20] [0x00] []`                                     |
| **List available users**             | `0x21`       | `!listu`                                      | `[0x01] [0x21] [0x00] []`                                     |
| **Get user messages**                | `0x22`       | `Bob`                                         | `[0x01] [0x22] [0x03] [Bob]`                                  |
| **Get channel messages**             | `0x23`       | `#general`                                    | `[0x01] [0x23] [0x08] [#general]`                             |
| **Send message to channel**          | `0x30`       | `#general Hello everyone!`                    | `[0x01] [0x30] [0x1E] [#generalHello everyone!]`              |
| **Send message to user**             | `0x31`       | `Bob How’s it going?`                         | `[0x01] [0x31] [0x13] [BobHow’s it going?]`                   |
| **Join channel**                     | `0x40`       | `!join #random`                               | `[0x01] [0x40] [0x07] [#random]`                              |
| **Change channel**                   | `0x41`       | `!change #random`                             | `[0x01] [0x40] [0x07] [#random]`                              |
| **Create channel**                   | `0x42`       | `!create #projects`                           | `[0x01] [0x41] [0x09] [#projects]`                            |
| **Change channel topic**             | `0x43`       | `!desc #general Welcome to the general chat!` | `[0x01] [0x42] [0x1C] [#generalWelcome to the general chat!]` |
| **Change channel key**               | `0x44`       | `!setkey #private secretkey123`               | `[0x01] [0x43] [0x11] [#private secretkey123]`                |
| **Kick user**                        | `0x50`       | `!kick #general Bob`                          | `[0x01] [0x50] [0x0A] [#generalBob]`                          |
| **Ban user**                         | `0x51`       | `!ban #general Bob`                           | `[0x01] [0x51] [0x0A] [#generalBob]`                          |
| **Send File**                        | `0x60`       | `!send <user/channel> <filename>`             | `[0x01] [0x60] [0x00] []`                                     |
| **Receive File**                     | `0x61`       | `!recv <user/channel> <file_id>`              | `[0x01] [0x61] [0x00] []`                                     |
|                                      | `0x75`       | continue file transer                         | `[0x01] [0x60] [0x00] []`                                     |
|                                      | `0x76`       | file transfer end                             | `[0x01] [0x60] [0x00] []`                                     |

---

## Response Codes

## Login and User Management

| Action                            | Outcome                              | Response Code |
|-----------------------------------|--------------------------------------|---------------|
| **Incorrect Request**             | Client Side Error                    | `0x00`        |
| **Incorrect Request**             | Server Side Error                    | `0x01`        |
| **Not Authenticated**             | Not Authenticated                    | `0x02`        |
| - | - | - |
| **Login with username and password** | Login successful                  | `0x10`        |
|                                   | User created                         | `0x11`        |
|                                   | Incorrect password                   | `0x12`        |
| **Set new password**              | Password updated successfully        | `0x20`        |
|                                   | Password update failed               | `0x21`        |
| **Change nickname**               | Nickname changed successfully        | `0x22`        |
|                                   | Nickname already in use              | `0x23`        |
| - | - | - |
|**Message sending**                | Message sent successfully            | `0x30`        |
|                                   | Failed to send message               | `0x31`        |
|                                   | Message from user                    | `0x32`        |
|                                   | Message from channel                 | `0x33`        |
|                                   | Failed to receive message            | `0x34`        |
| - | - | - |
| **List available channels**       | List of channels                     | `0x40`        |
| **List available users**          | List of users                        | `0x41`        |
| - | - | - |
| **Change channel**                | You have joined the channel          | `0x50`        |
|                                   | Channel does not exist               | `0x51`        |
| **Create channel**                | Channel created successfully         | `0x52`        |
|                                   | Channel already exists               | `0x53`        |
| **Change channel topic**          | Topic changed to new topic           | `0x54`        |
|                                   | Permission denied                    | `0x55`        |
|                                   | Invalid topic format                 | `0x56`        |
| **Change channel key**            | Channel key updated successfully     | `0x57`        |
|                                   | Permission denied                    | `0x58`        |
| - | - | - |
| **Kick User**                     | User kicked from channel             | `0x60`        |
|                                   | User not found in channel            | `0x61`        |
|                                   | Permission denied                    | `0x62`        |
| **Ban User**                      | User banned from channel             | `0x63`        |
|                                   | User not found in channel            | `0x64`        |
|                                   | Permission denied                    | `0x65`        |
| - | - | - |
| **File Transfer**                 | File uploaded                        | `0x70`        |
|                                   | File downloaded                      | `0x71`        |
|                                   | File upload failed                   | `0x72`        |
|                                   | File download failed                 | `0x73`        |
