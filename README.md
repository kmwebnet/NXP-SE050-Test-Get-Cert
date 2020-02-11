# NXP SE050 Test-Get-Cert

This communicates NXP SE050 secure chip from ESP32 and extract a certificate pre-provisioned by NXP.

# Requirements

  Platformio(PIO Core:4.1.0 PLATFORM: Espressif 32 1.11.1) with VS Code environment.  
  install "Espressif 32" platform definition on Platformio  

# Environment reference
  
  Espressif ESP32-DevkitC  
  this project initialize both of I2C 0,1 port, and the device on I2C port 0 is absent.  
  pin assined as below:  


      I2C 0 SDA GPIO_NUM_18
      I2C 0 SCL GPIO_NUM_19

      I2C 1 SDA GPIO_NUM_21
      I2C 1 SCL GPIO_NUM_22
          
  NXP SE050(on I2C port 1)  

# Usage

"git clone --recursive " on your target directory. 
and download "Plug & Trust MW Release v02.12.00_20191125" from NXP website
and put the contents into src as "simw-top" folder. 
you need to change a serial port number which actually connected to ESP32 in platformio.ini.  

# Run this project

just execute "Upload" on Platformio.   

# License

This software is released under the MIT license unless otherwise specified in the included source code. See License. 
