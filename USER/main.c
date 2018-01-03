#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"     
#include "usmart.h"			
#include "usart3.h"
#include "gps.h"
//ATK-S1216F8 GPS模块测试
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//串口1,发送缓存区
nmea_msg gpsx; 											//GPS信息
__align(4) u8 dtbuf[50];   								//打印缓存器
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 
int localTime_year,localTime_month,localTime_date,localTime_hour,timezone;

//计算时区函数，根据经度计算
static int calculateTimezone(double lat,double lon){
    int a,b,c,timezone;
    a = (int)(fabs(lon)+0.5);//对经度进行四舍五入，且取正整数
    b = a/15; //商
    c = a%15; //余数
    if((lat >=17.9 && lat <=53 && lon>=75 && lon<=125) || (lat>=40 && lat <=53 && lon>=125 && lon<= 135)){
			//如果经纬度处于中国版图内，则都化为东八区
    timezone = 8;
    }
    else{

        if(c > 7.5)
            timezone = b+1;
        else
            timezone = b;
        if(lon > 0.0f)
            timezone = timezone;
        else
            timezone = (-1)*timezone;
    }
    return timezone;
}

// UTC时间转化为本地时间函数
static int UTCTOLocalTime(int timezone){
    int year,month,day,hour;
	  //int localTime_year,localTime_month,localTime_date,localTime_hour;
    int lastday = 0;// 月的最后一天日期
    int lastlastday = 0;// 上月的最后一天日期

    year = gpsx.utc.year; 
    month = gpsx.utc.month;
    day = gpsx.utc.date;
    hour = gpsx.utc.hour + timezone; 

    if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12){
        lastday = 31;
        if(month == 3){
            if((year%400 == 0)||(year%4 == 0 && year%100 != 0))//判断是否为闰年，年号能被400或4整除，而不能被100整除
                lastlastday = 29;//闰年的2月29天，平年28天
            else
                lastlastday = 28;
        }
        if(month == 8)
            lastlastday = 31;
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11){
        lastday = 30;
        lastlastday = 31;
    }
    else{
        lastlastday = 31;
        if((year%400 == 0)||(year%4 == 0 && year%100 != 0))// 闰年的2月29天，平年28天
            lastday = 29;
        else
            lastday = 28;
    }

    if(hour >= 24){//当算出时区大于或等于24时，应减去24,日期加一天
            hour -= 24;
            day += 1; 
            if(day > lastday){ 
                day -= lastday;
                month += 1;

                if(month > 12){
                    month -= 12;
                    year += 1;
                }
            }
        }
    if(hour < 0){//当算出时区为负数时，应加上24:00,日期减一天
            hour += 24;
            day -= 1; 
            if(day < 1){ //日期为0时，日期变为上一月最后一天，月份减一个月
                day = lastlastday;
                month -= 1;
                if(month < 1){ //月份为0时，月份为12月，年份减1年
                    month = 12;
                    year -= 1;
                }
            }
        }
   //得到转换后的本地时间
   localTime_year = year;
   localTime_month = month;
   localTime_date = day;
   localTime_hour = hour;
	 return localTime_year;
	 return localTime_month;
	 return localTime_date;
	 return localTime_hour;
}

//显示GPS定位信息 
void Gps_Msg_Show(void)
{
 	//UTCTOLocalTime(8);
	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//得到经度字符串
 	LCD_ShowString(30,120,200,16,16,dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//得到纬度字符串
 	LCD_ShowString(30,140,200,16,16,dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//得到高度字符串
 	LCD_ShowString(30,160,200,16,16,dtbuf);	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//得到速度字符串	 
 	LCD_ShowString(30,180,200,16,16,dtbuf);	 				    
	if(gpsx.fixmode<=3)														//定位状态
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
	  LCD_ShowString(30,200,200,16,16,dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//用于定位的GPS卫星数
 	LCD_ShowString(30,220,200,16,16,dtbuf);	    
	sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//可见GPS卫星数
 	LCD_ShowString(30,240,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//可见北斗卫星数
 	LCD_ShowString(30,260,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//显示UTC日期
	LCD_ShowString(30,280,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
  LCD_ShowString(30,300,200,16,16,dtbuf);		  
	
	sprintf((char *)dtbuf,"Timezone:%04d     ",timezone);	     			//得到时区
 	LCD_ShowString(30,20,200,16,16,dtbuf);	
	sprintf((char *)dtbuf,"BeiJing Date:%04d/%02d/%02d   ",localTime_year,localTime_month,localTime_date);	//显示BeiJing日期
	LCD_ShowString(30,40,200,16,16,dtbuf);	
	sprintf((char *)dtbuf,"BeiJing Time:%02d:%02d:%02d   ",localTime_hour,gpsx.utc.min,gpsx.utc.sec);	//显示BeiJing时间
	LCD_ShowString(30,60,200,16,16,dtbuf);
}

int main(void)
{        
	//int localTime_year,localTime_month,localTime_date,localTime_hour;
	u16 i,rxlen;
	u16 lenx;
	u8 key=0XFF;
	u8 upload=0; 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);		//初始化串口波特率为115200
	usart3_init(384200);  //初始化串口3波特率为115200
	LED_Init();					//初始化LED  
 	LCD_Init();					//LCD初始化  
 	KEY_Init();					//按键初始化  
	usmart_dev.init(168);		//初始化USMART
	POINT_COLOR=RED;
	LCD_ShowString(30,80,200,16,16,"KEY0:Upload NMEA Data SW");   	 										   	   
  LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF"); 
	if(SkyTra_Cfg_Rate(5)!=0)	//设置定位信息更新速度为5Hz,顺便判断GPS模块是否在位. 
	{
   	LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Setting...");
		do
		{
			usart3_init(9600);			//初始化串口3波特率为9600
	  	SkyTra_Cfg_Prt(3);			//重新设置模块的波特率为38400
			usart3_init(38400);			//初始化串口3波特率为38400
      key=SkyTra_Cfg_Tp(100000);	//脉冲宽度为100ms
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);//配置SkyTraF8-BD的更新速率为5Hz
	  LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//清除显示 
	}
	while(1) 
	{		
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//接收到一次数据了
		{
			rxlen=USART3_RX_STA&0X7FFF;	//得到数据长度
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//启动下一次接收
			USART1_TX_BUF[i]=0;			//自动添加结束符
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符串
			calculateTimezone(gpsx.latitude,gpsx.longitude);
			//UTCTOLocalTime(timezone);
			UTCTOLocalTime(8);
			Gps_Msg_Show();				//调用函数，显示信息	
			if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//发送接收到的数据到串口1
 		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)
		{
			upload=!upload;
			POINT_COLOR=RED;
			if(upload)LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:ON ");
			else LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
 		}
		if((lenx%500)==0)
			LED0=!LED0;
		lenx++;	
	}
}













