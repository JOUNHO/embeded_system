// CLCD CALCULATOR

#include <stdio.h>                                                              
#include <stdlib.h>
#include <string.h> 
#include <wiringPi.h>

#define LCD_D4 2
#define LCD_D5 3
#define LCD_D6 1
#define LCD_D7 4
#define LCD_RS 7 
#define LCD_EN 0                                                                
#define NUM0 23
#define NUM1 22 
#define NUM2 21
#define NUM3 27
#define NUM4 14
#define NUM5 13
#define NUM6 26
#define NUM7 12
#define NUM8 11                                                                 
#define NUM9 10
#define SUM 5
#define SUB 6
#define EQU 24

char c; //To stroe number(0~9)and character(+,-,=)
char buf[100];  //Array to store entire formula
int cnt = 0;    // Declare count to put each character in an array
int result = 0; // Formula result
int i; // to use in for
int resultlength; //result length

void btn_push();
void print_result();

void write4bits(unsigned char command)
{
        digitalWrite(LCD_D4, (command & 1));
        command >>= 1;
        digitalWrite(LCD_D5, (command & 1));
        command >>= 1;
        digitalWrite(LCD_D6, (command & 1));
        command >>= 1;
        digitalWrite(LCD_D7, (command & 1));
        digitalWrite(LCD_EN, 1);
        delayMicroseconds(10);
        digitalWrite(LCD_EN, 0);
        delayMicroseconds(10);
}

void sendDataCmd4(unsigned char data)
{
        write4bits(((data >> 4) & 0x0f));
        write4bits((data & 0x0f));
        delayMicroseconds(100);
}

void putCmd4(unsigned char cmd)
{
        digitalWrite(LCD_RS, 0);
        sendDataCmd4(cmd);
}

void putChar(char c) {
        digitalWrite(LCD_RS, 1);
        sendDataCmd4(c);
}

void initialize_textlcd() {
        pinMode(LCD_RS, OUTPUT); pinMode(LCD_EN, OUTPUT);
        pinMode(LCD_D4, OUTPUT); pinMode(LCD_D5, OUTPUT);
        pinMode(LCD_D6, OUTPUT); pinMode(LCD_D7, OUTPUT);
        digitalWrite(LCD_RS, 0); digitalWrite(LCD_EN, 0);
        digitalWrite(LCD_D4, 0); digitalWrite(LCD_D5, 0);
        digitalWrite(LCD_D6, 0); digitalWrite(LCD_D7, 0);

        pinMode(NUM0, INPUT); pullUpDnControl(NUM0, PUD_DOWN);
        pinMode(NUM1, INPUT); pullUpDnControl(NUM1, PUD_DOWN);
        pinMode(NUM2, INPUT); pullUpDnControl(NUM2, PUD_DOWN);
        pinMode(NUM3, INPUT); pullUpDnControl(NUM3, PUD_DOWN);
        pinMode(NUM4, INPUT); pullUpDnControl(NUM4, PUD_DOWN);
        pinMode(NUM5, INPUT); pullUpDnControl(NUM5, PUD_DOWN);
        pinMode(NUM6, INPUT); pullUpDnControl(NUM6, PUD_DOWN);
        pinMode(NUM7, INPUT); pullUpDnControl(NUM7, PUD_DOWN);
        pinMode(NUM8, INPUT); pullUpDnControl(NUM8, PUD_DOWN);
        pinMode(NUM9, INPUT); pullUpDnControl(NUM9, PUD_DOWN);
        pinMode(SUM, INPUT); pullUpDnControl(SUM, PUD_UP);
        pinMode(SUB, INPUT); pullUpDnControl(SUB, PUD_UP);
        pinMode(EQU, INPUT); pullUpDnControl(EQU, PUD_DOWN);

        delay(35);

        putCmd4(0x28); // Function Set(4bit, 2rows)
        putCmd4(0x28); putCmd4(0x28);
        putCmd4(0x0f); // display ON, cursor ON, cursor BLINK
        putCmd4(0x02); // Return Home 
        delay(2);
        putCmd4(0x01); // Clear Display
        delay(2);
}


void btn_push() {
        if (digitalRead(NUM0) || digitalRead(NUM1) || digitalRead(NUM2) || digitalRead(NUM3) || digitalRead(NUM4) || digitalRead(NUM5) || digitalRead(NUM6) || digitalRead(NUM7) || digitalRead(NUM8) || digitalRead(NUM9) || !digitalRead(SUM)|| !digitalRead(SUB) || digitalRead(EQU)) {     //When the button is pressed
                if (digitalRead(NUM0)) c = '0';
                if (digitalRead(NUM1)) c = '1';
                if (digitalRead(NUM2)) c = '2';
                if (digitalRead(NUM3)) c = '3';
                if (digitalRead(NUM4)) c = '4';
                if (digitalRead(NUM5)) c = '5';
                if (digitalRead(NUM6)) c = '6';
                if (digitalRead(NUM7)) c = '7';
                if (digitalRead(NUM8)) c = '8';
                if (digitalRead(NUM9)) c = '9';
                if (!digitalRead(SUM)) c = '+';
                if (!digitalRead(SUB)) c = '-';
                if (digitalRead(EQU)) c = '=';  //What button was pressed

                buf[cnt] = c; //Enter entered value
                if(cnt>0){
                if ((buf[cnt-1] == '+' || buf[cnt-1] == '-' || buf[cnt-1]=='=')&&(buf[cnt] == '+' || buf[cnt] == '-'|| buf[cnt] == '=')) {      //When +,- is pressed continuously
                        char over[100] = "Invalid operation"; //String to print
                        initialize_textlcd(); //reset
                        int k;
                        for (k = 0; k < 17; k++) {
                                putChar(over[k]);       //String to print
                        }
                        delay(2000);    //for 2seconds 
                        initialize_textlcd();   //reset
                        for (i = 0; i < cnt+1; i++) {
                                buf[i] = '0';   //buf reset
                        }
                        cnt = 0; //count reset
                        return;
                }

                }
                cnt++; // count increase
                if (c == '=') { //When '=' input
                        putChar(c); //'=' print
                        delay(200);
                        print_result(); //go to print_result()
                }
                else if (buf[cnt-resultlength - 2] == '=' && (('0' <= c && c <= '9')||c=='+'||c=='-')) {        //if the previous input is '=' and the current input in number or +,-
                        initialize_textlcd(); // reset
                        for (i = 0; i < cnt; i++) {
                                buf[i] = '0';   //buf reset
                        }
                        cnt = 0;        //cnt reset
                        buf[cnt] = c;   //Enter entered value
                        cnt++;  //count increase
                        putChar(c); // output on LCD
                        delay(200);
                }
                else {
                        putChar(c); // output on LCD
                        delay(200);
                }
         if (cnt == 16) {       //When the LCD first line is exceeded
                putCmd4(0xC0); // Set DDRAM Address to 40H
                delay(40);
                putCmd4(0x06);
        }
        if (cnt >32) {  //When the LCD second line is exceeded
                char over[100]="Overflow";      //String to print
                initialize_textlcd();   //reset
                int k;
                for(k=0;k<8;k++){
                        putChar(over[k]);       //String output
                }
                        delay(2000);    //for 2seconds
                initialize_textlcd();   //reset
                for (i = 0; i < cnt; i++) {
                                buf[i] = '0';   //buf reset
                        }
                        cnt = 0;        //cnt reset

        }

        }
}
void print_result() {
        int num[100]; // Array to contain only numbers
        char op[100]; // Array to contain only +,-
        int numCnt = 0; // num[] index
        int opCnt = 0; // op[] index
        int temp = 0; //Variable stored when numbers are consecutive
        char s[10]; // String to use when casting

        for (i = 0; i < cnt; i++) {
                if ('0' <= buf[i] && buf[i] <= '9') { //when it is a number
                        temp *= 10; //Increase one digit
                        temp += buf[i] - '0'; //char->int
                }
                else if (buf[i] == '+' || buf[i] == '-') { // when it is +,-
                        num[numCnt] = temp;//Store number before operator
                        temp = 0; // reset to zero
                        numCnt++; // count increase
                        op[opCnt] = buf[i]; //store operator
                        opCnt++; // count inrease
                }
        }

        num[numCnt] = temp; //store last number of formulas

        result = num[0]; //Store first number of formulas
        for (i = 0; i < opCnt; i++) { //Repeat as many operators
                if (op[i] == '+') {     //when '+'
                        result += num[i + 1]; //Add number after operator
                }
                else if (op[i] == '-') {        //when '-'
                        result -= num[i + 1];//Subtract number after operator
                }
        }

        sprintf(s, "%d", result); // int->String
        resultlength=strlen(s); //result length
        for (i = 0; i < strlen(s); i++) {
                        if (cnt == 16) {//When the LCD first line is exceeded
                                putCmd4(0xC0); // Set DDRAM Address to 40H
                                delay(40);
                                putCmd4(0x06);
                        }
                        if (cnt >32) {//When the LCD second line is exceeded
                         char over[100]="Overflow";     //String to print
                         initialize_textlcd();  //reset
                         int k;
                        for(k=0;k<8;k++){
                                putChar(over[k]);       //String output
                        }
                                delay(2000);    //for 2seconds
                                initialize_textlcd();   //reset
                        for (i = 0; i < cnt; i++) {
                                buf[i] = '0';   //buf reset
                        }
                        cnt = 0;        //cnt reset
                        return;         //end function

        }
                putChar(s[i]); //Result output
                cnt++;  //count increase

        }
}

int main(int argc, char **argv) {
        wiringPiSetup();

        initialize_textlcd(); // reset

        while (1) {
        btn_push(); //When the button is pressed
        }

        return 0;
}

