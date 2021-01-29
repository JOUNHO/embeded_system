동작 방법
1) sudo minicom -o -s를 입력 후 패스워드 입력 후 Serial port setup선택 후 A를 눌러 1을 0으로 바꿔준 후 Save setup as dfl를 눌러 저장 후 exit선택
2) 첨부된 paint.c와 UI.bmp파일을 zmodem에 업로드 합니다.(ctrl+a+z, (UI.bmp, paint.c)파일을 space bar로 선택 후 엔터)
2-1) ls 명령어를 이용해 파일이 업로드 되었는지 확인하여 주시기 바랍니다.
3) 업로드가 되었다면 vim /etc/udev/rules.d/95-ads7846.rules 를 입력하여 Udev파일을 생성해 주시기 바랍니다.
4) 명령어를 입력하면 파일이 생성되는데 i를 눌러 insert모드로 전환합니다.
5) insert모드를 확인하시고 SUBSYSTEM==“input”, ATTRS{name}=="ADS7846 Touchscreen", ENV{DEVNAME}=="*event*", SYMLINK+="input/touchscreen" 를 입력해줍니다.
6) 입력이 되었다면 esc를 누르고 :wq를 입력하면 저장하고 나오게 됩니다.
7) sudo modprobe spicc 를 입력해줍니다.
8) sudo modprobe fbtft_device name=odroidc_tft32 rotate=270 gpios=reset:116,dc:115 speed=32000000 cs=0을 입력해줍니다.
9) 안정성을 위해 sudo modprobe –r ads7846 입력해줍니다.
10) sudo modprobe ads7846를 입력후 ls –l /dev/input/touchscreen 으로 확인하시기 바랍니다. (밑의 확인사항 참조)
11) 확인이 되었다면 gcc paint.c -o paint -lwiringPi -lwiringPiDev -lpthread -lcrypt -lm -lrt 를 입력하여 컴파일을 해줍니다.
12) ./paint 로 실행을 해줍니다.
13) +(십자가)가 뜨면 펜촉으로 +(십자가)가운데를 정확히 클릭하여 주시기 바랍니다.
14) 7)번 과정을 3번 반복하면 UI.bmp에 나와있는 화면이 뜨게됩니다.
15) 그림을 그리시기 위해선 READ_ME 동영상을 참고하시기 바랍니다.


10)번의 확인사항
*********************************
lrwxrwxrwx 1 root root 6 Jan  1 10:36 /dev/input/touchscreen -> event4

event는 기기에 따라 1이나 4가 될 수 있습니다.
*********************************


동작 시 주의사항
**********************************
항상 화면을 터치할 때에는 펜촉같은 것을 이용하시고, 항상 펜촉을 세워서 터치해주시기 바랍니다. 
펜촉을 이용하지 않고 손으로 터치할 시 화면 동기화가 제대로 되지 않아 터치오류가 발생할 수 있습니다.
+(십자가) 모양이 뜨게 되면 꼭 +(십자가) 모양의 정 가운데를 터치하여 주시기 바랍니다.
정가운데를 터치하지 않고 다른 데를 터치할 시 그림판이용을 정상적으로 하실 수 없습니다. 
또한, 그림판 UI가 뜨게 되면 각 메뉴를 적당한 힘으로 꾹 눌러 주시기 바랍니다. 
살짝만 터치를 하시게 되면 하드웨어 상태에 따라 터치가 인식하지 못할 수도 있습니다. 
**********************************  
