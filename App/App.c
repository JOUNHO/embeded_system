#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>
// LED Pin - wiringPi pin 24 is Odroid-C1 GPIO 97.

int LED[7]={13,10,11,26,24,14,23};  //LED define                                
             

int LEDNUM=7;    //LED count

int digit[16][7]={ //0~9,a~f define
        {1,1,1,1,1,1,0}, //0 
        {0,1,1,0,0,0,0}, //1                                                    
        {1,1,0,1,1,0,1}, //2
        {1,1,1,1,0,0,1}, //3 
        {0,1,1,0,0,1,1}, //4
        {1,0,1,1,0,1,1}, //5 
        {1,0,1,1,1,1,1}, //6 
        {1,1,1,0,0,1,0}, //7
        {1,1,1,1,1,1,1}, //8 
        {1,1,1,1,0,1,1}, //9                                                    
        {1,1,1,0,1,1,1}, //A                                                    
        {0,0,1,1,1,1,1}, //b
        {1,0,0,1,1,1,0}, //c
        {0,1,1,1,1,0,1}, //d
        {1,1,0,1,1,1,1}, //e
        {1,0,0,0,1,1,1} //f 
};

void pinmodeset(){ //7LED pinMode
        int i;
        for(i=0;i<LEDNUM;i++){
            pinMode(LED[i],OUTPUT);
    }
}

int* randomnum(){  //Random number without duplication 0~15
    static int save[16];
    int i,tmp;
    int count=0;
    int same =0;
    srand(time(0));
    while(count<16){
        same=0;
        tmp=rand()%16;
        for(i=0;i<count;i++){
            if(tmp==save[i]){
                same=1;
                break;
            }
        }
        if(same==0){
           save[count]=tmp;
           count++;
        }
     }
    return save;

}

int main (int argc, char *argv[]){      //main start
  if(argc<2){printf("ERROR:TOO FEW OPTION\n"); return 0;} //Option is required
  wiringPiSetup();
  pinmodeset(); //7LED pinmode
  if(!strcmp(argv[1],"1")){ //if OPTION 1
    if(argc>2){printf("ERROR:TOO MANY OPTION\n"); return 0;} //Should be App 1
    int i,j;   //i=0~9,a~f   j=7LED
     for(i=0;i<16;i++){
         for(j=0;j<LEDNUM;j++){
             digitalWrite(LED[j],digit[i][j]); //0~9,a~f Output in order
         }
        delay(500);  //0.5 delay
      }
  int* rannum = randomnum(); //Random number without duplication 0~16
  for(i=0;i<16;i++){
         for(j=0;j<LEDNUM;j++){
             digitalWrite(LED[j],digit[rannum[i]][j]);  //0~9,a~f Output Random
         }
        delay(500); //0.5delay
      }

  }
  else if(!strcmp(argv[1],"2")){//if OPTION 2
  if(argc>3){printf("ERROR:TOO MANY OPTION\n"); return 0;} //MUST App 2 0x~
  else if(argc<3){printf("ERROR:TOO FEW OPTION\n"); return 0;} //MUST App 2 0x~
  char* s=argv[2]; //0x~
  if(s[0]!='0'||s[1]!='x'){printf("ERROR:This option is not valid\n"); return 0;
} // MUST App 2 0x~
  int i;
  int result[8]={0}; //Binary
  int num=strtol(argv[2],NULL,16); //16 String->10 Convert
  if(num>0x7f){ // 0x00 ~ 0x7f
  printf("ERROR:Number gerater than 0x7f entered\n");
  return 0;
  }
  else if(num<0x00){ //0x00 ~ 0x7f
  printf("ERROR:Number less than 0x00 entered\n");
  return 0;
  }
      for(i=0;num>0;i++){  //10 -> 2 Convert
        result[i]=num%2;  //result=binary
        num=num/2;
      }
      for(i=0;i<8;i++){
        if(result[i]==1){ //if binary index==1 
          digitalWrite(LED[i],HIGH);  //index LED ON
        }
        else{digitalWrite(LED[i],LOW);} //index LED OFF
      }

  }
  else printf("ERROR:This option is not valid\n"); //Only 1 and 2

  return 0;
}

