#include <sd-reader_config.h>
#include <sd_raw.h>
#include <sd_raw_config.h>
#include "arduino/wiring.h"

int print_disk_info();
int sample();
int readDisk();

byte incomingByte;
void printWelcome();
long int address;
byte tempBytes[2];


void setup()
{
  
  Serial2.begin(9600);
  delay(1000);

  printWelcome();
  if(!sd_raw_init())
  {
     Serial2.println("MMC/SD initialization failed");      
  }
  print_disk_info();
}



void loop()
{
    int i;    
    
    if(Serial2.available()>0)
    {
         incomingByte=Serial2.read();
     
       switch(incomingByte)
       {
           case 114:
                     readDisk();
                     break;
           case 115:
                     sample();
                     break;
           default:
                     break;
       }
    }    
}

int sample()
{
    int i,j;
    int temp;
    byte low;
    byte high;
    byte inByte;
    
    Serial2.println();
    Serial2.println();
    Serial2.println("Sampling..");
    for(i=0;i<500;i=i+2)
    {
        if(Serial2.available()>0)
        {
            inByte=Serial2.read();
            if(inByte==113) return 0;
        }
        temp=analogRead(0);
        Serial2.print(temp,DEC);
        Serial2.print(" ");
        
        //Convert int to 2 bytes
        low=temp&0xFF;
        high=temp>>8;
       // Serial2.print(temp,DEC);
        //Serial2.print(low,DEC);
        //Serial2.print(high,DEC);
    
        tempBytes[0]=low;
        tempBytes[1]=high;
        
        if(!sd_raw_write(i,tempBytes,2))
        {
          Serial2.print("Write error");
        }
        //sd_raw_sync();
        delay(5000);
        
  Serial2.println(); 
       
    }
    
    return 1;
}


int readDisk()
{
    byte low;
    byte high;
    byte info[2];
    int i;
    int  result;
    Serial2.println();
    for(i=0;i<50;i=i+2)
    {
        sd_raw_read(i,info,2);
   
        //Serial2.print(info[0],DEC);
        //Serial2.print(" ");
        //Serial2.print(info[1],DEC);
        low=info[0];
        high=info[1];
        result=high<<8;
        //result<<8;
        Serial2.print(" ");
        Serial2.print(result+low,DEC);
        Serial2.print(" ");
    }
   
}

{
    Serial2.println("------------------------");
    Serial2.println("Data sampling system");
    Serial2.println("send r to read disk");
    Serial2.println("send s to start sampling");
    Serial2.println("send q to stop sampling");
    Serial2.println("Ready.....");
    Serial2.println("-------------------------");
}


int print_disk_info()
{
 

    struct sd_raw_info disk_info;
    if(!sd_raw_get_info(&disk_info))
    {
        return 0;
    }
    
    Serial2.println();
    Serial2.print("rev:    "); 
    Serial2.print(disk_info.revision,HEX); 
    Serial2.println();
    Serial2.print("serial: 0x"); 
    Serial2.print(disk_info.serial,HEX); 
    Serial2.println();
    Serial2.print("date:   "); 
    Serial2.print(disk_info.manufacturing_month,DEC); 
    Serial2.println();
    Serial2.print(disk_info.manufacturing_year,DEC); 
    Serial2.println();
    Serial2.print("size:   "); 
    Serial2.print(disk_info.capacity,DEC); 
    Serial2.println();
    Serial2.print("copy:   "); 
    Serial2.print(disk_info.flag_copy,DEC); 
    Serial2.println();
    Serial2.print("wr.pr.: ");
    Serial2.print(disk_info.flag_write_protect_temp,DEC); 
    Serial2.print('/');
    Serial2.print(disk_info.flag_write_protect,DEC); 
    Serial2.println();
    Serial2.print("format: "); 
    Serial2.print(disk_info.format,DEC); 
    Serial2.println();
    Serial2.print("free:   "); 
   
    return 1;
}


