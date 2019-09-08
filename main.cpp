/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cc
 * Copyright (C) 2018 Andrey Ivanov <EthicalTerminal@gmail.com>
 * 
 * I2C_Relay is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * I2C_Relay is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <bitset>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>
#include <ctype.h>
#include <type_traits>

//add libconfig support
//#include <libconfig.h>
//using namespace libconfig;
using namespace std;

#define I2C_ADDR 0x27           //Адрес устройства



// LCD Address
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00
// set init LCD BACKLIGHT ON or OFF

//define control bits
#define En 0b00000100 // Enable bit
#define Rw 0b00000010 // Read/Write bit
#define Rs 0b00000001 // Register select bit
//define control bits

//Refresh ratio
#define REFRESH_TIMEOUT 0.01
//Refresh ratio

// DEFINE BASICAL VARIABLES
int lcd_bl;
int lsdbl;
int data;
int cmd;
float refreshtime;
//

int lcd_backlight(int lcdbl=1){
if(lcdbl == 0){
    return (LCD_NOBACKLIGHT);
}
    return (LCD_BACKLIGHT);

}




int write_cmd(std::bitset<8> buffer){
int i2cfd;
char i2cbuffer[1];

i2cfd = open("/dev/i2c-1", O_RDWR); //Открываем файл
if (i2cfd < 0) {                    //Проверка возможности открытия
	printf("Error opening device file: %s\n", strerror(errno));
    exit(0);// !!!                       //Выход из программы, если устройство не определено.
}
if (ioctl(i2cfd, I2C_SLAVE, I2C_ADDR) < 0) {  //Проверяем устройство на наличие
	printf("ioctl error: %s\n", strerror(errno));
    exit(0); //!!!                       //Выход из программы, если устройство не определено.
}    
i2cbuffer[0]=(int)(buffer.to_ulong());

//cout<<i2cbuffer<<endl;
write(i2cfd,i2cbuffer, 1);
close (i2cfd);
sleep(0.001) ; 


}



int backlight(int set_lcdbl){
lcd_bl = set_lcdbl;
//return(lcd_bl);
}





// clocks EN to latch command
int lcd_strobe(int data) {
        //int ENABLE = En;
        write_cmd(data | En | lcd_backlight(lcd_bl));
        sleep(.0005);
        write_cmd(((data & ~En) | lcd_backlight(lcd_bl)));
        sleep(.0001);   
}

int lcd_write_four_bits(int data){
        write_cmd(data | lcd_backlight(lcd_bl));
        lcd_strobe(data);
        sleep(.0005);
}

int lcd_write(int cmd, int mode=0){
        
        lcd_write_four_bits(mode | (cmd & 0xF0));
        lcd_write_four_bits(mode | ((cmd << 4) & 0xF0));
        
}

int lcd_display_string( std::string string, int raw=1, int col=1){ 
      ////
      int col_new;
      if (raw == 1){
          col_new = col;
      };
      if (raw == 2){
          col_new = 0x40 + col;
        };
      if (raw == 3){
          col_new = 0x14 + col;
          
    };
      if (raw == 4){
          col_new = 0x54 + col;
          
    };

      lcd_write(0x80 + col_new);
      /////  
        
        for(char& charapters : string) {
        //cout<<(int)charapters<<endl;
        lcd_write((int)charapters, Rs);

        }
      //  for char in string:
        
}


float check_refresh_time(float time){
if (time < REFRESH_TIMEOUT){
refreshtime=REFRESH_TIMEOUT;
};
if (time > REFRESH_TIMEOUT){
refreshtime=time;
}
return (refreshtime);
}
//////////////

int lcd_display_text( std::string string){
      lcd_write(LCD_CLEARDISPLAY);    
      int i=0;
      int col=0;
      int col_new;
      col_new = col;
      lcd_write(0x80 + col_new);
        for(char& charapters : string) {
        //cout<<(int)charapters<<endl;
        lcd_write((int)charapters, Rs);
        i++;
            if(i==20-col){
            lcd_write(LCD_RETURNHOME);
            col_new = 0x40;
            lcd_write(0x80 + col_new);
            }
            if(i==40-col){
            lcd_write(LCD_RETURNHOME);
            col_new = 0x14;
            lcd_write(0x80 + col_new);
            }
            if(i==60-col){
            lcd_write(LCD_RETURNHOME);
            col_new = 0x54;
            lcd_write(0x80 + col_new);
            }
            if(i==80-col){
                
            float timeout=0;
            timeout=check_refresh_time(2);
            sleep(timeout);
            i=0;
            lcd_write(LCD_CLEARDISPLAY);
            lcd_write(LCD_RETURNHOME);
            col_new = 0x00;
            lcd_write(0x80 + col_new);
            }
            
            

            
        }
        //  for char in string:
        
}


//////////////

int lcd_clear(void){
        lcd_write(LCD_CLEARDISPLAY);
        lcd_write(LCD_RETURNHOME);
}


int init_lcd(int lsb_bl){

lcd_bl = lcd_bl;
lcd_write(0x03);
//lcd_write(0x03);
//lcd_write(0x03);
lcd_write(0x02);
lcd_write(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
lcd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
lcd_write(LCD_CLEARDISPLAY);
lcd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT);

sleep(0.0001);
}

/****Main****/
int main(int argc, char **argv){
    

init_lcd(1);
backlight(1) ;

lcd_display_string("Test1", 1,1);
lcd_display_string("Test2", 2, 2);
lcd_display_string("Test3", 3, 3);
lcd_display_string("Test4", 4, 4);
sleep(2);
lcd_clear();
lcd_display_text("A locomotive is moving. Someone asks: 'What moves it?' A peasant says the devil moves it. Another man says the locomotive moves because  its wheels go round. A third asserts that the cause of its movement lies in the smoke which the wind carries away. The peasant is irrefutable. He has devised a complete explanation. To refute him someone would have to prove to him that there is no devil, or another peasant would have to explain to him that it is not the devil but a German, who moves the locomotive. Only then, as a result of the contradiction, will they see that they are both wrong. But the man who says that the movement of the wheels is the cause refutes himself, for having once begun to analyze he ought to go on and explain further why the wheels go round; and till he has reached the ultimate cause of the movement of the locomotive in the pressure of steam in the boiler, he has no right to stop in his search for the cause. ");
sleep(4);
lcd_clear();
lcd_display_text("AAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAADAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAAEAAAAAAAAAAAAAAAAAAADAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAZ");

}



