# 6therm
 Temperature and ventilation management for D4WSC Eberpasher heating
 
Old generation Eberspacher heaters are not delivered with a system for regulating the ambient temperature and regulating the supply ventilation of the unit heaters.
In addition, fan heaters often come with a noisy fan and a 2 or 3 speed switch.
For use in a motorhome or fitted cell, it is advantageous to be able to finely modulate the blowing to reduce noise.

This project describes how to use a Raspberry Pico microcontroller and a touch graphic display from WareShare (https://www.waveshare.com/pico-restouch-lcd-2.8.htm)
for controlling a 4 pin fan (pwm controlled fan).

It controls the triggering or stopping of the Eberspacher heater, the starting and stopping of the fan and its speed by a PWM control.
The two low power outputs can be provided by a two-way 12V relay kit.

It is developed on the Arduino environment which supports the RP-2040.

Main features are:

	mode 0 : Stop mode : only temperature display
    mode 1 : Hot water / Shower mode : Heater ON but no inboard fan 
	mode 2 : Auto temperature controller mode : the fan speed is automatically regulated according to the temperature set point. 
        	 The more the temperature is below the set point, the higher the speed.
			 By using the mode change button (double arrow), it is possible to change the automatic ventilation power regulation mode: 
			 Silent, Cool, Normal, Maxi, 60 minutes, i.e. Silent mode with automatic shutdown after 60 minutes
    mode 3 : Manual fan speed controller mode : the fan speed is adjusted according to the plus or minus keys

    measurement of ambient temperature via OneWire probe : DS18B20
    TFT touch display for showing status information and for setting pwm or target temperature

Even if you don't want to use all of these features, the project can hopefully easily be simplified or extended.


Images

Cool mode : low ventilation for silent mode

![6therm](https://user-images.githubusercontent.com/28572566/230140698-9f757220-fcbf-4bb7-bf8b-e357f0c5ffc9.jpg)

Hot water mode : no ventilation, only heating for the plate heat exchanger to heat the water for the shower.  

![Hot water mode](https://user-images.githubusercontent.com/28572566/230140735-2fa0ed35-701b-44e1-87c8-fa6bb51cc64f.jpg)

Manual mode : manual fan flow

![Manual mode](https://user-images.githubusercontent.com/28572566/230140749-62b174cd-3b76-4129-92df-0000e4c9dfe0.jpg)

Stop mode : only temperature display

![Stop Mode](https://user-images.githubusercontent.com/28572566/230140762-1a4582c0-b35e-4378-bcce-c362dff10605.jpg)


All icon images are stored in tabs ^ e.g. DASH_01.h
Arrays containing FLASH images can be created with UTFT library tool: (libraries\UTFT\Tools\ImageConverter565.exe)
Convert to .c format then copy into a new tab

This sketch loading images from arrays stored in program (FLASH) memory.

Works with TFT_eSPI library here: https://github.com/Bodmer/TFT_eSPI

Make sure all the display driver and pin connections are correct by
editing the User_Setup.h file in the TFT_eSPI library folder.





