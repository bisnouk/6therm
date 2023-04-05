# 6therm
 Temperature and ventilation management for Eberpasher heating
 
Old generation Eberspacher heaters are not delivered with a system for regulating the ambient temperature and regulating the supply ventilation of the unit heaters.
In addition, fan heaters often come with a noisy fan and a 2 or 3 speed switch.
For use in a motorhome or fitted cell, it is advantageous to be able to finely modulate the blowing to reduce noise.

The proposed module uses a Raspberry Pico and a touch graphic display from WareShare (https://www.waveshare.com/pico-restouch-lcd-2.8.htm).
It controls the triggering or stopping of the boiler, the starting and stopping of the fan and its speed by a PWM control.

The two low power outputs can be provided by a two-way 12V relay kit.

It is developed on the Arduino environment which supports the RP-2040.

![6therm](https://user-images.githubusercontent.com/28572566/230140698-9f757220-fcbf-4bb7-bf8b-e357f0c5ffc9.jpg)
![Hot water mode](https://user-images.githubusercontent.com/28572566/230140735-2fa0ed35-701b-44e1-87c8-fa6bb51cc64f.jpg)
![Manual mode](https://user-images.githubusercontent.com/28572566/230140749-62b174cd-3b76-4129-92df-0000e4c9dfe0.jpg)
![Stop Mode](https://user-images.githubusercontent.com/28572566/230140762-1a4582c0-b35e-4378-bcce-c362dff10605.jpg)


Icon images are stored in tabs ^ e.g. DASH_01.h etc more than one icon can be in a header file
Arrays containing FLASH images can be created with UTFT library tool: (libraries\UTFT\Tools\ImageConverter565.exe)
Convert to .c format then copy into a new tab

This sketch loading images from arrays stored in program (FLASH) memory.

Works with TFT_eSPI library here: https://github.com/Bodmer/TFT_eSPI

Make sure all the display driver and pin connections are correct by
editing the User_Setup.h file in the TFT_eSPI library folder.





