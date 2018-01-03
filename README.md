# GPS_stm32
ATK-S1216F8_GPS with stm32f407

## 项目介绍:

基于探索者STM32F4开发板，使用ATK-S1216F8-BD模块，实现GPS/北斗定位。并实现了将UTC时间转化成北京时间（Timezone设置成8），
或者根据计算出的时区显示当地时间。

## 操作

   通过串口3连接ATK-S1216F8-BD模块，然后通过液晶显示GPS信息，包括精度、纬度、高度、速度、
用于定位的卫星数、可见卫星数、UTC时间等信息。同时，可以通过USMART工具，设置GPS模块的刷
新速率（最大支持20Hz刷新）和时钟脉冲的配置。另外，通过KEY0按键，可以开启或关闭NMEA数据的
上传（即输出到串口1，方便开发调试）。

## 硬件资源:

- DS0(连接在PF9) 
- 串口1(波特率:115200,PA9/PA10连接在板载USB转串口芯片CH340上面)
- ALIENTEK 2.8/3.5/4.3/7寸TFTLCD模块(通过FSMC驱动,FSMC_NE4接LCD片选/A6接RS) 
- 按键KEY0(PE4)
- 串口3(波特率:38400,PB10/PB11连接ATK-S1216F8-BD模块)
- ATK MODULE接口&ATK-S1216F8-BD模块(模块直接插ATK MODULE接口即可),连接关系如下: 
   ```
   STM32开发板-->ATK-S1216F8-BD模块
   PF6-->PPS
   PB10-->RXD
   PB11-->TXD 
   GND-->GND
   5V-->VCC

