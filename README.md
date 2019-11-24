# bootloaderV2

There are 4 parts to this
The bootloader (this), the ecryption application, the firmware application, and the PC firmware updater.

This is a universal bootloader currently for the STM32F105. The bootloader uses a PC Firmware updater app which takes a encrypted hex file (AES 128bit) https://github.com/karlyamashita/AES-Encryption and decrypts the file and loads it into program space.

When the bootloader is first flashed onto the STM32F105 it does not have a product name embedded. When you update the application firmware it will embed a product name. So the next time you try to update the application firmware the PC and Bootloader will check to see if the ecrypted hex file is designated for that particular bootloader before proceeding with the update.

The bootloader will not allow overwriting the bootloader address section. The bootloader will erase the program space prior to updating the application firmware. If the update is interrupted, USB cable disconnected, obviously the application wll not work but you can retry and update the firmware again as the bootloader is still intact. So there is no fear of bricking a product.

