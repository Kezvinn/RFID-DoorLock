#RFID locking system

Feature of product:
  - RFID module
    - Scan RFID card and retrieve the UID of the card for verification  
  - LCD display:
    - Display correspond message based on the state of the system
  - Numpad
    - Enable to input passcode from 0-9, '*' and '#' for special action (explain below)

How to use:
  - The user first press '*' to swtich to scanning card process.
  - RFID scan: 
    - If UID card == valid, the user then enters their passcode and press '#' at the end.
    - If UID card |= valid, the user has to press '*' again to start over.
  - Passcode enter:
    - User has 3 trieds to enter passcode
    - If passcode is corrected and within 3 trieds -> user access granted
    - If passcode is incorrect and > 3 tried -> user access denied

Further development:
  - This project can be further implemented with different modules such as servo, solenoid lock, relay,... for controlling the locking mechanism, depend on the design.
