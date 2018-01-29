#include "hal.h"
#include "system.h"
#include "uart.h"
#include "pin.h"
/*#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "exti.h"*/

#include <cstring>
#include <cmath>
#include <algorithm>

#include "drivers/lis3dsh/lis3dsh.h"
/*#include "filesystem/filesystem_sdio.h"
#include "drivers/ili9341/ili9341.h"
#include "drivers/nunchuck/nunchuck.h"
#include "drivers/ssd2119/ssd2119.h"
*/
#if 0
#include "drivers/sn8200/sn8200_api.h"
#include "drivers/sn8200/sn8200.h"


uint8_t key;
uint8_t seqNo = 0;

int8_t mysock = -1;

extern int ipok, joinok;
extern int destIP, srcIP;
extern long int destPort, srcPort;
extern int32u pktcnt;

extern char domain[100];
extern char Portstr[8];
char uri[100]={0};
char sockConnected = -1;
char sockClosed = -1;
int timeout1 = 5;
extern bool IsCreateSocketResponsed ;
extern int32u timeout;
extern bool IsWIFIJoinResponsed ;

void ShowMenu(void)
{
    printf("---------------------\n\r");
    printf("0 Get WiFi status\n\r");
    printf("1 Wifi Scan\n\r");
    printf("2 Wifi Join\n\r");
    printf("3 Get IP\n\r");
    printf("4 TCP client\n\r");
    printf("5 TCP sever\n\r");
    printf("6 Send from sock\n\r");
    printf("7 WiFi Leave\n\r");
    printf("8 AP On/Off\n\r");
    printf("9 UDP client\n\r");
    printf("a UDP server\n\r");
    printf("b Wifi Off\n\r");
    printf("c Wifi On\n\r");
    printf("d HTTP get req\n\r");
    printf("e HTTP post req\n\r");
    printf("f HTTP post Json req\n\r");
    printf("g HTTP chunked post req\n\r");
    printf("h HTTPS get req\n\r");
    printf("i TLS client\n\r");
    printf("j TLS server (HTTPS server)\n\r");
    printf("m: Show Menu\n\r");
    printf("q: press q to Quit \n\r");
    printf("---------------------\n\r");
}

void ProcessUserInput(void)
{
    char tmp[100];
    key = uart2.read();
    printf("\n\r");

    switch(key) {
    case '0':
        GetStatus(seqNo++);
        break;

    case '1':
        WifiScan(seqNo++);
        break;

    case '2':
        WifiDisconn(seqNo++);
        WifiJoin(seqNo++);
        SnicInit(seqNo++);
        SnicIPConfig(seqNo++);
        break;

    case '3':
        SnicInit(seqNo++);
        SnicGetDhcp(seqNo++);
        break;

    case '4':
     /*   mysock = -1;
        tcpCreateSocket(0, 0xFF, 0xFF, seqNo++, SNIC_TCP_CREATE_SOCKET_REQ);
        if (mysock != -1) {
            if (getTCPinfo() == CMD_ERROR) {
                printf("Invalid Server\n\r");
                break;
            }
            // This connection can receive data upto 0x0400=1K bytes at a time.
            tcpConnectToServer(mysock, destIP, (unsigned short)destPort, 0x0400, 0x5, seqNo++);
        }*/
        break;

    case '5':
     /*   if (setTCPinfo() == CMD_ERROR) {
            printf("Invalid Server to create\n\r");
            break;
        }
        mysock = -1;
        tcpCreateSocket(1, srcIP, (unsigned short)srcPort, seqNo++, SNIC_TCP_CREATE_SOCKET_REQ);
        if (mysock != -1) {
            // This connection can receive data upto TEST_BUFFERSIZE at a time.
            tcpCreateConnection(mysock, TEST_BUFFERSIZE, 0x5, seqNo++);
        }*/
        break;

    case '6': {
 /*       char tempstr[2] = {0};
        int8u datamode;
        char sockstr[8];
        int32u sock;
        char teststr[128];
        int len;

        printf("Enter socket number to send from: \n\r");
        scanf("%s", sockstr);
        sock = strtol(sockstr, NULL, 0);

        printf("Content Option? (0: Default  1: User specific) \n\r");
        scanf("%s", tempstr);
        datamode = atoi(tempstr);

        if (datamode) {
            printf("Enter payload to send (up to 128 bytes): \n\r");
            scanf("%s", teststr);
            len = (int)strlen(teststr);
            sendFromSock(sock, (int8u*)teststr, len, 2, seqNo++);
        } else {
            sendFromSock(sock, TxBuf, TEST_BUFFERSIZE, 2, seqNo++);
            pktcnt = 0;
        }*/
        break;
    }

    case '7':
        SnicCleanup(seqNo++);
        WifiDisconn(seqNo++);
        break;

    case '8':
        ApOnOff(seqNo++);
        break;

    case '9': {//udp send
      /*  int i;
        udpCreateSocket(0, 0, 0, seqNo++);
        if (mysock != -1) {
            if (getUDPinfo() == CMD_ERROR) {
                printf("Invalid Server\n\r");
                break;
            }
            printf("Send %d\n\r", UDP_NUM_PKT);
            for (i=0; i<UDP_NUM_PKT; i++) {
                int si = i % TEST_BUFFERSIZE + 1;
                SendSNIC(TxBuf, si);
                printf("%d %d\n\r", i, si);
            }

            closeSocket(mysock,seqNo++);
        }*/
        break;
    }

    case 'a': {//udp recv
        int16u port = 43211;
        int32u ip = 0xAC1F0001; // 172.31.0.1
        udpCreateSocket(1, ip, port, seqNo++);
        udpStartRecv(mysock, 2048, seqNo++);
        break;
    }

    case 'b':
        SnicCleanup(seqNo++);
        WifiOff(seqNo++);
        break;

    case 'c':
        WifiOn(seqNo++);
        break;

    case 'd':
    /*    printf("Enter server name:  %s\n\r", domain);
        scanf("%s", tmp);
        printf("\n\r");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpReqTest(domain, 0);*/
        break;

    case'e':
     /*   printf("Enter server name: ([CR] to accept %s)\n\r", domain);
        scanf("%s", tmp);
        printf("\n\r");
        if (strlen(tmp)) 
        strcpy(domain, tmp);
        sendHttpPostDemo(domain);*/
        break;

    case 'f':
    /*    printf("Make sure STA is connected to SN8200 soft AP.\n\r");
        strcpy(domain, "sn8200.com");
        printf("Enter server name: ([CR] to accept %s)\n\r", domain);
        scanf("%s", tmp);
        printf("\n\r");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpJsonPostDemo(domain);*/
        break;
#if 0

    case 'g':
        strcpy(domain, "192.168.10.100");
        printf("Enter server name (or the peer testclient IP, peer testclient should start TCP server on port 80): ([CR] to accept %s)\n\r", domain);
        scanf("%s", tmp);
        printf("\n\r");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpChunkReqTest(domain);
        break;
#endif

    case 'h':
     /*   printf("Enter server name: ([CR] to accept %s)\n\r", domain);
        scanf("%s", tmp);
        printf("\n\r");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpReqTest(domain, 1);*/
        break;

    case 'i':
      /*  timeout1 = 5;
        mysock = -1;
        tcpCreateSocket(0, 0xFF, 0xFF, seqNo++, SNIC_TCP_CREATE_SIMPLE_TLS_SOCKET_REQ);  // use less memory in SN8200
        mdelay(500);
        if (mysock != -1) {
            strcpy(Portstr, "443");
            if (getTCPinfo() == CMD_ERROR) {
                printf("Invalid Server\n\r");
                break;
            }
            
            tcpConnectToServer(mysock, destIP, (unsigned short)destPort,0x0000,timeout1,seqNo++);
            while ((sockConnected == -1) && timeout1) {
                mdelay(500);
                timeout1--;
                if (sockClosed == mysock) {
                    sockClosed = -1;
                    break;
                }
            }
            
            if (sockConnected == mysock) {
                sendFromSock(mysock, (int8u*)GET_REQUEST, sizeof(GET_REQUEST)-1, 2, seqNo++);
                sockConnected = -1;
            }
            else printf("Connect failed.\n\r");
        }*/
        break;

    case 'j': //ssl server
    /*    strcpy(Portstr, "443");
        if (setTCPinfo() == CMD_ERROR) {
            printf("Invalid Server to create\n\r");
            break;
        }
        mysock = -1;
        tcpCreateSocket(1, srcIP, (unsigned short)srcPort, seqNo++, SNIC_TCP_CREATE_ADV_TLS_SOCKET_REQ);
        if (mysock != -1) {
            // This connection can receive data upto TEST_BUFFERSIZE at a time. 
            tcpCreateConnection(mysock, TEST_BUFFERSIZE, 0x5, seqNo++);
        }*/
        break;

    case 'm':
        ShowMenu();
        break;

    default:
        break;
    }

}
#endif

int main()
{
	if(System::init())
		asm("bkpt 255");
    
    /* Delay to avoid some artifacts on UART when flashing the UC - can be safely removed in prod */
    HAL_Delay(1000);
   
    Pin led(GPIOC, 13);
	led.init(GPIO_MODE_OUTPUT_PP);

    /* TX: PA9, RX: PA10 */
 //   uart1.init(921600, true);

    /* TX: PA2, RX: PA3 */
    /* Remapped: TX: PD5, RX: PD6 */
    uart2.init(2000000);
/*
    if(spi1.init(10'000'000, SPI::Mode::MODE_0))
        asm("bkpt 255");
        */
    printf("Hello world!\r\n");

#if 0
    LIS3DSH mems(spi1, Pin(GPIOE, 3));

    if(mems.init(LIS3DSH::Config(LIS3DSH::Config::ODR_12)))
        asm("bkpt 255");
#endif  
#if 0
    /*
    SN8200 sn8200(uart1, Pin(GPIOE, 0));
    */
    SN8200_API_Init(921600);

    /*WifiOn(seqNo++);
    printf("\n\r");
    
    ShowMenu();

    while (1)
    {
        if(uart2.dataAvailable())
            ProcessUserInput();

        if(SN8200_API_HasInput())
            ProcessSN8200Input();
    }
    */
#endif
#if 0
	/* SDA: PB7, SCL: PB6 */
	if(i2c1.init())
		asm("bkpt 255");

	Nunchuck nunchuck(i2c1);

    if(nunchuck.init())
        asm("bkpt 255");

#endif
#if 0
	SSD2119 display(Pin(GPIOD, 3), Pin(GPIOD, 13));

	if(display.init(Orientation::LANDSCAPE_2))
		asm("bkpt 255");

#endif
/*
	if(rtc.init())
		asm("bkpt 255");

	rtc.setDateTime(DateTime(DateTime::Date(22,8,2017), DateTime::Time(12,0,0)));
    */
//    rtc.setAlarm(DateTime::Time(12,0,45));

  //  display.doBenchmark();
#if 0
    const int32_t speed = 20;
    const int32_t radius = 10;

    Point2D dsize = display.size();

    float y = 0;

    float x = 150;
    float dx = x;

    int32_t bx = dsize.x()-2;

    int32_t v = speed;

    uint32_t tm = System::millis();
    uint32_t tn = System::millis();
    
	for(;;)
	{
        if(System::millis() - tm > 25)
        {
            tm = System::millis();

            display.fillRect(bx, dsize.y()-dsize.y()/2, 2, dsize.y()/2, Color16::Black);

            bx -= 3;

            if(bx <= 0)
                bx = dsize.x()-2;

            display.fillRect(bx, dsize.y()-dsize.y()/2, 2, dsize.y()/2, Color16::Blue);

            display.fillRect(dx-radius-1, dsize.y()-(int)(y+2*radius+1), (radius+2)*2, (radius+2)*2, Color16::Black);

            dx = x;

            y += v;
            if(y < 0)
            {
                y = 0;
                v = speed;
            }
            --v;

            display.fillCircle(x, dsize.y()-(int)(y+radius), radius, 0xFFFF);
        }

        if(System::millis() - tn > 5)
        {
            tn = System::millis();
            Nunchuck::Data d = nunchuck.getData();

            if(d)
            {
                int32_t jx = (int32_t)(d.joyX)-127;

                if(abs(jx) > 5)
                    x += (float)(jx)/20.f;
               
                if(d.cButtonDown)
                {
                    display.setOrientation(Orientation::LANDSCAPE_2);
                    display.fillScreen(0x0000);
                    dsize = display.size();
                }
                else if (d.zButtonDown)
                {
                    display.setOrientation(Orientation::PORTRAIT_2);
                    display.fillScreen(0x0000);
                    dsize = display.size();
                }

                if(x < radius)
                    x = radius;
                if(x > dsize.x()-radius)
                    x = dsize.x()-radius;

            }
         }
	}
#endif
#if 0
    if(spi1.init(0, SPI::Mode::MODE_0))
        asm("bkpt 255");

    Pin displayCs   (GPIOB, 11);
    Pin displayDc   (GPIOC, 1);
    Pin displayRst  (GPIOC, 3);

    ILI9341 display(spi1, displayCs, displayDc, displayRst);

    if(display.init(Orientation::LANDSCAPE_2))
        asm("bkpt 250");

    display.fillScreen(0x0);
#endif
#if 0
    Filesystem filesystem;

    if(filesystem.mount())
	{
		display.setCursor(0, 50);
		display.write("FATAL: cannot mount filesystem");
        asm("bkpt 255");
	}
    
    File file;

    if(file.open("test3.bmp"))
        asm("bkpt 255");

	display.setCursor(0, 0);
	display.write("Hello world!");

    
    if(display.drawBmpFromFile(file, 50, 50, true))
		asm("bkpt 255");

//	display.doBenchmark();
#endif

    uint32_t t = System::millis();
	for (;;)
	{
  /*      auto tm = rtc.getDateTime().getTime();
        printf("%02d:%02d:%02d\r\n", tm.h, tm.m, tm.s);
        */
        led.setHigh();
    	System::delay(200);
        led.setLow();
        System::delay(200);
    }

}


