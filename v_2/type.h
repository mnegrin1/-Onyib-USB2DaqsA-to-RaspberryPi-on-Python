#ifndef TYPE_H
#define TYPE_H
#include <libusb-1.0/libusb.h>
typedef    unsigned char BYTE ;				
typedef    unsigned  short WORD;
typedef    unsigned  int     DWORD;
typedef    void          VOID;
typedef    void*         LPVOID;
typedef    libusb_device_handle* HANDLE;
#define FALSE  0
#define TRUE   1

#define USBD_VID                      0x0483
#define USBD_PID                      0x8001


#define BULK_IN_EP                                     0x81
#define BULK_OUT_EP                                    0x01
#define INT_IN_EP                                      0x82
#define INT_OUT_EP                                     0x02

#define DEV_UART     0x80
#define DEV_I2C      0x81
#define DEV_SPI      0x82
#define DEV_POLL     0x83
#define DEV_CAN      0x84
#define DEV_GPIO     0x85
#define DEV_ADC      0x86
#define DEV_PWM      0x87
#define DEV_PARALLEL 0x88
#define DEV_MEM      0x8D
#define DEV_TRIG     0x8E
#define DEV_ALL      0x8F

#define OP_READ      0x10
#define OP_WRITE     0x11
#define OP_CONFIG    0x12
#define OP_QUERY     0x13
#define OP_RESET     0x14
#define OP_VERSION   0x15
#define OP_QUERY_DFU       0x16
#define OP_VERIFY          0x17
#define OP_STATUS          0x18
#define OP_TEST            0x19

#define OP_START           0x20
#define OP_STOP            0x21
#define OP_ENTER_DFU       0x23

#define MODE_DMA     0x01
#define MODE_DIR     0x02

#define IOCTL_READ_DATA     0xF000
#define IOCTL_WRITE_DATA    0xF001
#define IOCTL_READ_TRIG     0xF002
#define IOCTL_READ_VER      0xF00E

#define IOCTL_GET_ADDRESS    (DEV_I2C<<8|OP_ADDRESS)
#define IOCTL_SET_I2C    (DEV_I2C<<8|OP_CONFIG)
#define IOCTL_GET_I2C    (DEV_I2C<<8|OP_QUERY)
#define IOCTL_SET_SPI    (DEV_SPI<<8|OP_CONFIG)
#define IOCTL_GET_SPI    (DEV_SPI<<8|OP_QUERY)
#define IOCTL_SET_TIMER  (DEV_TIMER<<8|OP_CONFIG)
#define IOCTL_GET_TIMER  (DEV_TIMER<<8|OP_QUERY)

#define IOCTL_SET_GPIO   (DEV_GPIO<<8|OP_CONFIG)
#define IOCTL_GET_GPIO   (DEV_GPIO<<8|OP_QUERY)
#define IOCTL_SET_TRIG   (DEV_TRIG<<8|OP_CONFIG)
#define IOCTL_GET_TRIG   (DEV_TRIG<<8|OP_QUERY)
#define IOCTL_START_TRIG (DEV_TRIG<<8|OP_START)
#define IOCTL_STOP_TRIG  (DEV_TRIG<<8|OP_STOP)
#define IOCTL_GET_PWM    (DEV_PWM<<8|OP_QUERY)
#define IOCTL_SET_PWM    (DEV_PWM<<8|OP_CONFIG)
#define IOCTL_START_PWM  (DEV_PWM<<8|OP_START)
#define IOCTL_STOP_PWM   (DEV_PWM<<8|OP_STOP)
#define IOCTL_GET_ADC    (DEV_ADC<<8|OP_QUERY)
#define IOCTL_SET_ADC    (DEV_ADC<<8|OP_CONFIG)
#define IOCTL_START_ADC  (DEV_ADC<<8|OP_START)
#define IOCTL_STOP_ADC   (DEV_ADC<<8|OP_STOP)
#define IOCTL_SET_BUFF   (DEV_MEM<<8|OP_CONFIG)
#define IOCTL_GET_BUFF   (DEV_MEM<<8|OP_QUERY)

#define FM_VER           (6)
#if FM_VER > 4
typedef struct
 {
		/*
	  byADCOptions:

	  bit5     for AD706 period unit            0- US, 1- MS
	  bit4     for AD7606 range:                1- in +10V ~ -10V,    0- in +5v ~ -5v	  
	  bit0~2   for AD7606 OS;
	*/
	BYTE  byADCOptions;
	/*
	  byTrigOptions:

	  bit7     for AD7606 samping:                1- in sampling,    0- stop
	  bit4     for AD706 trig vol selection       0- large or equal, 1- small or equal
	  bit2~3   for AD7606 IO selection:           00- falling, 01- Raising , 10- raising and falling
	  bit0~1   for AD7606 trig mode;              00- GPIO trig, 01- period trig, 10- GPIO + period��11- trigVol + period
	*/

	BYTE  byTrigOptions;
	WORD  wReserved1;
	BYTE  byMainCh;
	BYTE  byCheckCnt;
	WORD  wReserved2;
  /* wPeriod: Timer trig period, unit byADCOptions bit 5~6 */
	WORD  wPeriod;
	WORD  wTrigVol; //unit mv
 /* dwTrigCnt: current trig counter */
	DWORD dwCycleCnt;
  /* dwMaxCnt: Max Enabled trig number , trig will exit if dwTrigCnt is equal to dwMaxCnt */
	DWORD dwMaxCycles;
 }ADC_CONFIG;
#else 
typedef struct
 {
/*
	  byADCOptions:

	  bit5     for AD706 period unit            0- US, 1- MS
	  bit4     for AD7606 range:                1- in +10V ~ -10V,    0- in +5v ~ -5v
	  bit3     for AD7606 Ref selection:        1- Internal Ref,      0- External
	  bit0~2   for AD7606 OS;
	*/
	BYTE  byADCOptions;
	/*
	  byTrigOptions:

	  bit7     for AD7606 samping:                1- in sampling,    0- stop
	  bit2~3   for AD7606 IO selection:           00- falling, 01- Raising , 10- raising and falling
	  bit0~1   for AD7606 trig mode;              00- GPIO trig, 01- period trig, 10- GPIO + period
	*/

	BYTE  byTrigOptions;
	WORD  wTrigSize;
	DWORD dwABDelay;
  /* wPeriod: Timer trig period, unit byADCOptions bit 5~6 */
	WORD wPeriod;
	WORD wReserved;
 /* dwTrigCnt: current trig counter */
	DWORD dwCycleCnt;
  /* dwMaxCnt: Max Enabled trig number , trig will exit if dwTrigCnt is equal to dwMaxCnt */
	DWORD dwMaxCycles;

 }ADC_CONFIG;
#endif
#endif // TYPE_H
