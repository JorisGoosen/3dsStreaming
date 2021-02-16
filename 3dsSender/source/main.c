#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <setjmp.h>
#include <3ds.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <stdbool.h>


#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);

#define CONFIG_3D_SLIDERSTATE (*(volatile float*)0x1FF81080)
#define WAIT_TIMEOUT 1000000000ULL

#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE WIDTH * HEIGHT * 2
#define BUF_SIZE SCREEN_SIZE * 2


#define STAPPEN  400
#define MAXLINE BUF_SIZE / STAPPEN 

#define EXTRA_INFO 8 //4bytes voor stap en 4 byte voor frame ofzo?

static jmp_buf exitJmp;

inline void clearScreen(void) {
	u8 *frame = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(frame, 0, 320 * 240 * 3);
}

void hang(char *message) {
	clearScreen();
	printf("%s", message);
	printf("Press start to exit");

	while (aptMainLoop()) {
		hidScanInput();

		u32 kHeld = hidKeysHeld();
		if (kHeld & KEY_START) longjmp(exitJmp, 1);
	}
}

void cleanup() {
	camExit();
	gfxExit();
	acExit();
}

void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
	u8 *fb_8 	= (u8*)  fb;
	u16 *img_16 = (u16*) img;

	int i, j, draw_x, draw_y;

	for(j = 0; j < height; j++) 
	{
		for(i = 0; i < width; i++) 
		{
					draw_y 		= y + height - j;
					draw_x 		= x + i;
			u32 	v 			= (draw_y + draw_x * height) * 3;
			u16 	data 		= img_16[j * width + i];
			uint8_t b 			= ((data >> 11) & 0x1F) << 3;
			uint8_t g 			= ((data >> 5) 	& 0x3F) << 2;
			uint8_t r 			= ( data 		& 0x1F) << 3;
					fb_8[v] 	= r;
					fb_8[v+1] 	= g;
					fb_8[v+2] 	= b;
		}
	}
}

// TODO: Figure out how to use CAMU_GetStereoCameraCalibrationData
/*void takePicture3D(u8 *buf) {
	u32 bufSize;
	printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int) CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT));
	printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT));

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_OUT1_OUT2));

	Handle camReceiveEvent = 0;
	Handle camReceiveEvent2 = 0;

	printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_BOTH));
	printf("CAMU_SynchronizeVsyncTiming: 0x%08X\n", (unsigned int) CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2));

	printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_BOTH));

	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREEN_SIZE, (s16) bufSize));
	printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + SCREEN_SIZE, PORT_CAM2, SCREEN_SIZE, (s16) bufSize));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
	printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT));
//	printf("CAMU_PlayShutterSound: 0x%08X\n", (unsigned int) CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL));

	printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_BOTH));

	svcCloseHandle(camReceiveEvent);
	svcCloseHandle(camReceiveEvent2);

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));
}*/
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;
s32 sock = -1, csock = -1;


//---------------------------------------------------------------------------------
void socShutdown() {
//---------------------------------------------------------------------------------
	printf("waiting for socExit...\n");
	socExit();
}


struct sockaddr_in si_me, si_other;
int s, i, blen, slen = sizeof(si_other);
u8 *udpbuf, *buf, * udpbufR;

#define STACK_SIZE 1024 * 64

volatile bool runThreads = true;

void verzendShit(void * arg)
{

	int ret;


		// allocate buffer for SOC service
	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

	if(SOC_buffer == NULL) {
		failExit("memalign: failed to allocate\n");
	}

	// Now intialise soc:u service
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	failExit("socInit: 0x%08X\n", (unsigned int)ret);
	}

	// register socShutdown to run at exit
	// atexit functions execute in reverse order so this runs before gfxExit
	atexit(socShutdown);


	udpbuf  = malloc(BUF_SIZE);
	udpbufR = malloc(MAXLINE + EXTRA_INFO); //Eerste getal is "regelnummer"

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == -1)
		failExit("Ik kon niet socketen!");
		

	memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(1234);
    si_me.sin_addr.s_addr = gethostid();

	memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family 		= AF_INET;
    si_other.sin_port 			= htons(1234);
    si_other.sin_addr.s_addr 	= inet_addr("192.168.1.100"); //htonl(INADDR_ANY);;

    if (bind(s, (struct sockaddr*) &si_me, sizeof(si_me))==-1)
    	failExit("Ik kon niet binden!");

	//const u32 stappen = 100;
	//const u32 subBufSize = BUF_SIZE / stappen;

	printf("Even camera op gang laten komen misschien\n");
	svcSleepThread(3000000000); 

	u32 frame = 0;

	while(runThreads)
	{
		//int zoveel = sprintf(udpbuf, "Hallo vanuit andere cpu!");
		//sendto(s, udpbuf, zoveel, 0, (struct sockaddr*) &si_other, slen);
		//printf("hoi!\n");
		memcpy(udpbuf, buf, BUF_SIZE);

		svcSleepThread(1000);
		printf("copied camera to udp buffer and sending frame %d\n", frame);

		for(u32 i=0; i < STAPPEN && runThreads; i++)
		{
			memcpy(udpbufR,     &frame,					4);
			memcpy(udpbufR + 4,	&i,						4);
			memcpy(udpbufR + 8, udpbuf + (i * MAXLINE), MAXLINE);

			size_t sentChars = sendto(s, udpbufR, MAXLINE + EXTRA_INFO, 0, (struct sockaddr*) &si_other, slen);
		//	printf("Managed to send %d chars\n", sentChars);

			if(i%100==0)
				svcSleepThread(1000);
		}

		frame++;
	}
}



int main() {
	// Initializations
	acInit();
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	printf("Main thread prio: 0x%lx\n", prio);


	Thread hendel;
	
	Result resultaat =  threadCreate(verzendShit, 0, STACK_SIZE, prio-1, -2, false);
	
	printf("svcCreateThread: 0x%08X hendel: 0x%08X\n", (unsigned int)resultaat, (unsigned int) hendel);

	
	// Enable double buffering to remove screen tearing
	gfxSetDoubleBuffering(GFX_TOP, 		true);
	gfxSetDoubleBuffering(GFX_BOTTOM, 	false);

	// Save current stack frame for easy exit
	if(setjmp(exitJmp)) {
		cleanup();
		return 0;
	}

	u32 kDown;

	printf("Initializing camera\n");

	printf("camInit: 0x%08X\n", (unsigned int) camInit());

	printf("CAMU_SetSize: 0x%08X\n", (unsigned int) CAMU_SetSize(SELECT_OUT1_OUT2, SIZE_CTR_TOP_LCD, CONTEXT_A));
	printf("CAMU_SetOutputFormat: 0x%08X\n", (unsigned int) CAMU_SetOutputFormat(SELECT_OUT1_OUT2, OUTPUT_RGB_565, CONTEXT_A));
-
	// TODO: For some reason frame grabbing times out above 10fps. Figure out why this is.
	printf("CAMU_SetFrameRate: 0x%08X\n", (unsigned int) CAMU_SetFrameRate(SELECT_OUT1_OUT2, FRAME_RATE_10));

	printf("CAMU_SetNoiseFilter: 0x%08X\n", (unsigned int) CAMU_SetNoiseFilter(SELECT_OUT1_OUT2, true));
	printf("CAMU_SetAutoExposure: 0x%08X\n", (unsigned int) CAMU_SetAutoExposure(SELECT_OUT1_OUT2, true));
	printf("CAMU_SetAutoWhiteBalance: 0x%08X\n", (unsigned int) CAMU_SetAutoWhiteBalance(SELECT_OUT1_OUT2, true));
	// TODO: Figure out how to use the effects properly.
	//printf("CAMU_SetEffect: 0x%08X\n", (unsigned int) CAMU_SetEffect(SELECT_OUT1_OUT2, EFFECT_SEPIA, CONTEXT_A));

	printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int) CAMU_SetTrimming(PORT_CAM1, false));
	printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int) CAMU_SetTrimming(PORT_CAM2, false));
	//printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

	buf = malloc(BUF_SIZE);
	if(!buf) {
		hang("Failed to allocate memory!");
	}

	u32 bufSize;
	printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int) CAMU_GetMaxBytes(&bufSize, WIDTH, HEIGHT));
	printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_BOTH, bufSize, WIDTH, HEIGHT));

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_OUT1_OUT2));

	Handle camReceiveEvent = 0;
	Handle camReceiveEvent2 = 0;

	printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_BOTH));
	printf("CAMU_SynchronizeVsyncTiming: 0x%08X\n", (unsigned int) CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2));

	printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_BOTH));
	printf("CAMU_PlayShutterSound: 0x%08X\n", (unsigned int) CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_MOVIE));

	gfxFlushBuffers();
	gspWaitForVBlank();
	gfxSwapBuffers();

	printf("\nUse slider to enable/disable 3D\n");
	printf("Press Start to exit to Homebrew Launcher\n");

	int benBezig = 0, nietAltijd =0;

//	const int stappen = 129, deelstap = BUF_SIZE / (stappen-1);

	// Main loop
	while (aptMainLoop()) {
		//printf("Cameraloop!\n");

		// Read which buttons are currently pressed or not
		hidScanInput();
		kDown = hidKeysDown();

		// If START button is pressed, break loop and quit
		if (kDown & KEY_START) {
			break;
		}

		//printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) 
		CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, SCREEN_SIZE, (s16) bufSize); //);
		CAMU_SetReceiving(&camReceiveEvent2, buf + SCREEN_SIZE, PORT_CAM2, SCREEN_SIZE, (s16) bufSize);

		//printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) 
		svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT); //);
		svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT);
			
		if(CONFIG_3D_SLIDERSTATE > 0.0f) {
			gfxSet3D(true);
			writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf + SCREEN_SIZE, 0, 0, WIDTH, HEIGHT);
			writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), buf , 0, 0, WIDTH, HEIGHT);
		} else {
			gfxSet3D(false);	
			writePictureToFramebufferRGB565(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, 0, 0, WIDTH, HEIGHT);
		}

		//printf("svcCloseHandle: 0x%08X\n", (unsigned int) 
		svcCloseHandle(camReceiveEvent); //);
		svcCloseHandle(camReceiveEvent2);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();



		svcSleepThread(10000);


		/*if(benBezig != 0)
		{
		
			if(nietAltijd % stappen == 0)
				memcpy(udpbuf, buf, BUF_SIZE);
			else	
				sendto(s, udpbuf + deelstap * (nietAltijd % (stappen - 1)), deelstap, 0, (struct sockaddr*) &si_other, slen);

			nietAltijd++;
		}

		benBezig = 1;*/
	}

	printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_BOTH));

	printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));

	runThreads = false;

	// Exit
	free(buf);
	free(udpbuf);
	cleanup();

	// Return to hbmenu
	return 0;
}


//---------------------------------------------------------------------------------
void failExit(const char *fmt, ...) {
//---------------------------------------------------------------------------------

	if(sock>0) close(sock);
	if(csock>0) close(csock);

	va_list ap;

	printf(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(CONSOLE_RESET);
	printf("\nPress B to exit\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_B) exit(0);
	}
}