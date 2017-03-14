# XMega_ESP8266
ESP8266 AT Command Set Library for ATXMega controllers (Tested on XMega256A3BU) 
</br> 
This Library contains all the functions pertaining to AT Command set for ESP-01 AND ESP-12 Firmware (per ver 1.3.0.0)</br> 
 on ESP8266.</br> 
 	</br> 
Serial Configuration is included, provided you change the Serial Port used defined in AT_USART  </br> 
  </br> 
 	Buffers str_res and AT_output are used to return result as a string, you need to include them as external variables to utilize them</br>  
 #  Ways to include in your file:</br> 
  include "Serial_AT_Functions.h"</br> 
 </br> 
 			extern char str_res[BUFF_LEN];</br> 
 			extern char AT_output[BUFF_LEN/4];</br> </br>
     
# Adapting to Other AVRs
  The code can be adapted to other AVRs by changing the Serial Config and sendChar functions as per your uController

 
