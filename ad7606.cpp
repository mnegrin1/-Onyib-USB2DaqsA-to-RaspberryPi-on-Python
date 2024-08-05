#include<stdio.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include "type.h"

#define DEVICE_CTX(dev) ((dev)->ctx)
#define HANDLE_CTX(handle) (DEVICE_CTX((handle)->dev))

#define DllExport

WORD gRecBuff[3][3072/2];
WORD SamplesData[655360*16/2];
BYTE CancelCode;
bool bCanceled;
extern float MaxVol;
extern bool bSampled;

bool UIS_Rx(HANDLE pDevHand,BYTE* pbyRxBuf,DWORD wRxSize,WORD wTimeOut);
bool UIS_Tx(HANDLE pDevHand,BYTE* pbyTxBuf,DWORD wTxSize,WORD wTimeOut);
void  IntTransCallback0(struct libusb_transfer *transfer);
void  IntTransCallback1(struct libusb_transfer *transfer);
void  IntTransCallback2(struct libusb_transfer *transfer);
int M3F20xm_CreateInterruptTransfer(HANDLE pDevHand);
void M3F20xm_CancelInterruptTransfer(HANDLE pDevHand);
libusb_transfer* pIntTrans[3] = {NULL,NULL,NULL};
extern libusb_context *ctx;
extern volatile DWORD cycles;
extern ADC_CONFIG cfg;
extern void request_exit(sig_atomic_t code);
bool UIS_Rx(HANDLE pDevHand,BYTE* pbyRxBuf,DWORD wRxSize,WORD wTimeOut)
{
    int r;
    int rx = 0;
    r = libusb_bulk_transfer(pDevHand,BULK_IN_EP,pbyRxBuf,(int)wRxSize,&rx,wTimeOut);
    // printf( "\n\n%s\n","UIS_Rx");
    // printf("&rx = %d\n",&r);
    // printf("pbyTxBuf = %d\n",pbyRxBuf);
    // printf("TxSize = %d\n\n",wRxSize);
    return !r;
}

bool UIS_Tx(HANDLE pDevHand,BYTE* pbyTxBuf,DWORD wTxSize,WORD wTimeOut)
{
    int r;
    int tx = 0;
    r = libusb_bulk_transfer(pDevHand,BULK_OUT_EP,pbyTxBuf,(int)wTxSize,&tx,wTimeOut);

    // printf( "%s\n","UIS_Tx");
    // printf("&tx = %d\n",&tx);
    // printf("pbyTxBuf = %d\n",pbyTxBuf);
    // printf("TxSize = %d\n",wTxSize);
    return !r;
}

bool UIS_ctrl_tx(HANDLE pDevHand,BYTE* pbyTxBuf,DWORD wTxSize,WORD wTimeOut)
{
    int r;
    uint16_t wValue;
    wValue = (uint16_t)(pbyTxBuf[0] << 8) + pbyTxBuf[1];
    // printf( "%s\n","UIS_ctrl_tx");    
    r = libusb_control_transfer(pDevHand, LIBUSB_ENDPOINT_OUT |
		LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_ENDPOINT,
		0, wValue,0, NULL, 0, 1000);
    return !r;
}

DllExport HANDLE  M3F20xm_OpenDevice(libusb_context *pCtx)
   /*++
Routine Description:
   open a USBIO dev
Arguments:
   void
Return Value:
  a usb dev index
--*/
{
    int r;
    HANDLE pDevHand;
    //printf( "%s\n","M3F20xm_USBIO_OpenDevice");
    pDevHand = libusb_open_device_with_vid_pid(pCtx,USBD_VID,USBD_PID);
   if(pDevHand==NULL)
    {
       printf("libusb_open_device_with_vid_pid error\n");
       return NULL;
     }
  
    r=libusb_claim_interface(pDevHand,0);
    if(r<0)
    {
        printf("cannot claim interface\n");

    }
 M3F20xm_CreateInterruptTransfer(pDevHand);
 return pDevHand;
}

DllExport bool  M3F20xm_CloseDevice(HANDLE pDevHand)
   /*++
Routine Description:
   close a USBIO dev
Arguments:
   byIndex - device No.
Return Value:
  none
--*/
{
   bool re ;
   // printf( "%s\n","M3F20xm_CloseDevice");
 // usleep(100000);
   M3F20xm_CancelInterruptTransfer(pDevHand);
   while(pIntTrans[0] || pIntTrans[1] || pIntTrans[2])
   {
   	if(libusb_handle_events(ctx) < 0)
   		{
   				 printf( "%s\n","libusb_handle_events fail");
   				 break;
   		}
   	
   	
   }
   libusb_release_interface(pDevHand,0);
   //libusb_attach_kernel_driver(pDevHand,0);
   libusb_close( pDevHand );
   printf( "%s\n","Closed");
   return true;

}

DllExport bool M3F20xm_GetVersion(HANDLE pDevHand,BYTE byType,LPVOID lpBuffer)
{
//   printf( "%s\n","M3F20xm_GetVersion");
  bool bResult = false;
	BYTE len;
	char* pBuff = (char*)lpBuffer;
	if(byType == 0)   //get dll version
	{
	 //len = strlen(BUILD_VERSION);
	// memcpy(pBuff,BUILD_VERSION,len);
	 pBuff[len++]= ';';
	// memcpy(&pBuff[len],BUILD_DATE,strlen(BUILD_DATE));
	// len = len + strlen(BUILD_DATE);
	// pBuff[len++]= ' ';
	 //memcpy(&pBuff[len],BUILD_TIME,strlen(BUILD_TIME));
	 //len = len + strlen(BUILD_TIME);
	 //pBuff[len++]= 0;
	 bResult = true;
	}
	else if(byType == 1)   //get sys version
	{
    memset(pBuff,'-',20);
    pBuff[20]= 0;
    bResult = true;
	}
	else if(byType == 2)   //ger firmware version
	{
	  BYTE buff[64];
	  buff[0] = DEV_ALL;
	  buff[1] = OP_VERSION;
	//buff[2] = byDevAddr;
	//memcpy(&buff[3],&wRate,2);
      bResult = UIS_Tx(pDevHand,buff,64,100);
	  if(!bResult)
	  {    
       printf( "%s\n","UIS_Tx fail");
       //printf("buff %x\n",buff[0]);
       return false;
	  }
      bResult = UIS_Rx(pDevHand,buff,64,100);
      //printf( "buff[0] = %x\n",buff[0]);
	  if(bResult && buff[0]==DEV_ALL)
	  {
		 memcpy(pBuff,&buff[1],50);
	  }
	  else
	  {
		  bResult = false;             
          printf( "%s\n","UIS_Rx fail GetVersion");
	
	  }
	}
}

DllExport bool M3F20xm_GetSerialNo(HANDLE pDevHand,char* lpBuff)
{
  int r;
//    printf( "%s\n","M3F20xm_GetSerialNo");
   memset(lpBuff,0,12);
   r = libusb_get_string_descriptor_ascii(pDevHand,3,(unsigned char*)lpBuff,12);
   if(r < 0)
   {
     printf("%s\n","failed to get serial number ");
     return false;
   }
    return true;
}


DllExport bool M3F20xm_ADCSetConfig(HANDLE pDevHand, ADC_CONFIG* pCfg)
{
 	// printf("%s\n", "M3F20xm_ADCSetConfig");
	bool bResult;
	BYTE buff[64];
	buff[0] = IOCTL_SET_ADC >> 8;
	buff[1] = IOCTL_SET_ADC & 0xFF;

	/*	buff[8] = pCfg->byTrigOptions;
	buff[9] = pCfg->byIOTrigType;
	buff[10] = pCfg->wTimerTrigInternal;
	buff[11] = pCfg->wTimerTrigInternal >> 8;

	buff[12] = pCfg->wRWSize;
	buff[13] = pCfg->wRWSize >> 8;
	buff[14] = pCfg->wTrigSize;
	buff[15] = pCfg->wTrigSize >> 8;
	buff[16] = pCfg->byCmdSize;*/
	memcpy(&buff[8], pCfg, sizeof(ADC_CONFIG));
	bResult = UIS_Tx(pDevHand, buff, 64, 100);
	return bResult;
}

DllExport bool M3F20xm_ADCGetConfig(HANDLE pDevHand, ADC_CONFIG* pCfg)
{
	// printf("%s\n","M3F20xm_ADCGetConfig");
	bool bResult;
	BYTE buff[64];
	buff[0] = IOCTL_GET_ADC >> 8;
	buff[1] = IOCTL_GET_ADC & 0xFF;
	bResult = UIS_Tx(pDevHand, buff, 64, 100);
	if (!bResult)
	{
		printf("%s fail\n", "UIS_Tx");		
		return FALSE;
	}
	bResult = UIS_Rx(pDevHand, buff, 64, 100);
	if (bResult && buff[0] == DEV_ADC)
	{
		//*pbySelect         = buff[1];		
		memcpy(pCfg, &buff[1], sizeof(ADC_CONFIG));
	}
	else
	{
		bResult = false;
		printf("%s fail\n", "UIS_Rx - ADCGeConfig");
	}
	return bResult;
}



DllExport bool  M3F20xm_ADCStart(HANDLE pDevHand)
{
//   printf( "%s\n","M3F20xm_ADCStart");
 	BYTE buff[64]; 	
 	CancelCode = 0;
 	bCanceled = false;
	buff[0] = DEV_ADC;
	buff[1] = OP_START;
	if (!UIS_Tx(pDevHand, buff, 64, 100))
	{
		printf("UIS_Tx fail\n");
		return false;
	}
	/*if(!UIS_ctrl_tx(pDevHand,buff,64,100))
	{
		printf("UIS_ctrl_tx fail\n");
		return false;
	}*/
    return true;
 }

DllExport bool    M3F20xm_ADCStop(HANDLE pDevHand)
{
    bool bResult;
    // printf( "%s\n","M3F20xm_ADCStop");
  	BYTE buff[64];
	buff[0] = DEV_ADC;
	buff[1] = OP_STOP;
	if (!UIS_Tx(pDevHand, buff, 64, 100))
	{
		printf("UIS_Tx fail\n");
		return false;
	}
	/*if(!UIS_ctrl_tx(pDevHand,buff,64,100))
	{
		printf("UIS_ctrl_tx fail\n");
		return false;
	}*/
    return true;
}


DllExport bool M3F20xm_ADCRead(HANDLE pDevHand,WORD* lpReadBuffer)
{
//   printf("%s++\n", "M3F20xm_ADCRead");
	//	bool bResult;
	BYTE buff[64];
	buff[0] = DEV_ADC;
	buff[1] = OP_READ;
	buff[2] = 0;
	buff[3] = 0;
	buff[4] = 0;
	buff[5] = 0;
	buff[6] = 0;
	buff[7] = 0;
	if (!UIS_Tx(pDevHand, buff, 64, 100))
	{
		printf("UIS_Tx fail\n");
		return false;
	}
	if (!UIS_Rx(pDevHand, (BYTE*)lpReadBuffer, 64, 100))
	{
		printf("UIS_Rx fail ADC_Read\n");
		return false;
	}
	
	return true;
   
}

int M3F20xm_CreateInterruptTransfer(HANDLE pDevHand)
{
//   printf("CreateInterruptTransfer\n");
  int completed[3];
  pIntTrans[0]  = libusb_alloc_transfer(0);
  pIntTrans[1]  = libusb_alloc_transfer(0);
  pIntTrans[2]  = libusb_alloc_transfer(0);
  
  if(!pIntTrans[0] || !pIntTrans[1] || !pIntTrans[2])
  {
   printf("libusb_alloc_transfer fail\n");
   return 0;
  }
  libusb_fill_interrupt_transfer(pIntTrans[0],pDevHand,INT_IN_EP,(BYTE*)gRecBuff[0],3072,IntTransCallback0,&completed[0],0);
  libusb_fill_interrupt_transfer(pIntTrans[1],pDevHand,INT_IN_EP,(BYTE*)gRecBuff[1],3072,IntTransCallback1,&completed[1],0);
  libusb_fill_interrupt_transfer(pIntTrans[2],pDevHand,INT_IN_EP,(BYTE*)gRecBuff[2],3072,IntTransCallback2,&completed[2],0);
  pIntTrans[0]->type = LIBUSB_TRANSFER_TYPE_INTERRUPT;
  pIntTrans[1]->type = LIBUSB_TRANSFER_TYPE_INTERRUPT;
  pIntTrans[2]->type = LIBUSB_TRANSFER_TYPE_INTERRUPT;
  if(libusb_submit_transfer(pIntTrans[0]) < 0 ||
  	 libusb_submit_transfer(pIntTrans[1]) < 0 ||
  	 libusb_submit_transfer(pIntTrans[2]) < 0 
  	)
 {
    libusb_free_transfer(pIntTrans[0]);
    libusb_free_transfer(pIntTrans[1]);
    libusb_free_transfer(pIntTrans[2]);
    printf("CreateInterruptTransfer fail\n");
    return 0;
  }
  return 1;
}

void M3F20xm_CancelInterruptTransfer(HANDLE pDevHand)
{
//   printf("M3F20xm_CancelInterruptTransfer\n");
  int r;
  bCanceled = true;
  if((CancelCode & 0x01) == 0)
  	{
  	 libusb_cancel_transfer(pIntTrans[0]);
  	 //libusb_cancel_transfer(pIntTrans[2]);
  	 //libusb_cancel_transfer(pIntTrans[1]);	
  	}
  if((CancelCode & 0x02) == 0)
   {
  	libusb_cancel_transfer(pIntTrans[1]);
  
  	}
  	if((CancelCode & 0x04) == 0)
  	{  		
  	 libusb_cancel_transfer(pIntTrans[2]);  
   }
  
  	 #if 0
  if(pIntTrans[2])
  {
  	   printf("libusb_cancel_transfer 2 \n");
      libusb_cancel_transfer(pIntTrans[2]);
     /* while(1)
      {
      	r = libusb_handle_events(ctx);
      	printf("r = %d\n",r);
      	if (r < 0)
			    break;
      }*/
  }
  if(pIntTrans[1])
  {
  	   printf("libusb_cancel_transfer 1\n");
      libusb_cancel_transfer(pIntTrans[1]);
     
  }
  if(pIntTrans[0])
  {
  	   printf("libusb_cancel_transfer 0\n");
      libusb_cancel_transfer(pIntTrans[0]);
   
  }
  #endif
}

void  IntTransCallback0(struct libusb_transfer *transfer)
{
    int r;
    float realVol;
    // printf("IntTransCallback 0\n");
    switch (transfer->status)
    {
    case LIBUSB_TRANSFER_COMPLETED:
        printf("LIBUSB_TRANSFER_COMPLETED 0\n");
        r = transfer->actual_length;
        //if(r == 0)
        //{
        		//libusb_submit_transfer(transfer);
        	//	return;
        //}
        
        
        /*for(int i = 0; i < r / 16; i++)
        {
         printf("cycles %d",cycles + i);
         for(int j = 0; j < 8;j++)
         {
         	if(gRecBuff[i*8+j]&0x8000)
         		{
         			gRecBuff[i*8+j] = ~gRecBuff[i*8+j];
         			realVol = -1 * MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		} 
         	else
         		{
         			realVol = MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		}
         	//printf("% 2.6f,   ",realVol);
         	
         }
         printf("\n");
        }*/
        /*if(transfer == pIntTrans[0])
          memcpy(&SamplesData[cycles * 8],&gRecBuff[0],r); 
        else if(transfer == pIntTrans[1])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[1],r); 
         else if(transfer == pIntTrans[2])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[2],r); 
         else	
         	return;*/
        cycles += r / 16;
        printf("cycles %d,r = %d\n",cycles,r);
        if(cfg.dwMaxCycles && cfg.dwMaxCycles == cycles)
        {        	
         if(bSampled)
         	{ 
         		bSampled = false;
           
            request_exit(1);
          }
           CancelCode |= 0x01;
           libusb_free_transfer(transfer);
           pIntTrans[0]  = NULL;
        }	  
        else 
        {
         if(!bCanceled)
         	libusb_submit_transfer(transfer);	
         else
         	{
         		 libusb_free_transfer(transfer);
             pIntTrans[0]  = NULL;
         	}
         	
        }        
        return;
    case LIBUSB_TRANSFER_TIMED_OUT:
        printf("LIBUSB_TRANSFER_TIMED_OUT\n");
        r = LIBUSB_ERROR_TIMEOUT;
        break;
    case LIBUSB_TRANSFER_STALL:
        printf("LIBUSB_TRANSFER_STALL\n");
        r = LIBUSB_ERROR_PIPE;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        printf("LIBUSB_TRANSFER_NO_DEVICE\n");
        r = LIBUSB_ERROR_NO_DEVICE;
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        printf("LIBUSB_TRANSFER_OVERFLOW\n");
        r = LIBUSB_ERROR_OVERFLOW;
        break;
    case LIBUSB_TRANSFER_ERROR:
         printf("LIBUSB_TRANSFER_ERROR\n");
         r = LIBUSB_ERROR_IO;
         break;
    case LIBUSB_TRANSFER_CANCELLED:
        printf("LIBUSB_TRANSFER_CANCELLED\n");
        r = LIBUSB_ERROR_IO;
        break;
    default:
        printf("LIBUSB_ERROR_OTHER\n");
        r = LIBUSB_ERROR_OTHER;
    }

    libusb_free_transfer(transfer);
    //if(transfer == pIntTrans[0])
    {   pIntTrans[0]  = NULL;
    	  printf("0 released \n");
    }    
}

void  IntTransCallback1(struct libusb_transfer *transfer)
{
    int r;
    float realVol;
    // printf("IntTransCallback 1\n");
    switch (transfer->status)
    {
    case LIBUSB_TRANSFER_COMPLETED:
        printf("LIBUSB_TRANSFER_COMPLETED 1\n");
        r = transfer->actual_length;
        //if(r == 0)
        //{
        		//libusb_submit_transfer(transfer);
        	//	return;
        //}
        
        
        /*for(int i = 0; i < r / 16; i++)
        {
         printf("cycles %d",cycles + i);
         for(int j = 0; j < 8;j++)
         {
         	if(gRecBuff[i*8+j]&0x8000)
         		{
         			gRecBuff[i*8+j] = ~gRecBuff[i*8+j];
         			realVol = -1 * MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		} 
         	else
         		{
         			realVol = MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		}
         	//printf("% 2.6f,   ",realVol);
         	
         }
         printf("\n");
        }*/
        /*if(transfer == pIntTrans[0])
          memcpy(&SamplesData[cycles * 8],&gRecBuff[0],r); 
        else if(transfer == pIntTrans[1])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[1],r); 
         else if(transfer == pIntTrans[2])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[2],r); 
         else	
         	return;*/
        cycles += r / 16;
        printf("cycles %d,r = %d\n",cycles,r);
        if(cfg.dwMaxCycles && cfg.dwMaxCycles == cycles)
        {        	
        	if(bSampled)
         	{ 
         		bSampled = false;
           
            request_exit(1);
          }
          CancelCode |= 0x02;
          libusb_free_transfer(transfer);
          pIntTrans[1]  = NULL;
        }	  
        else 
        {
        	if(!bCanceled)
           libusb_submit_transfer(transfer);	
          else
          {
         		 libusb_free_transfer(transfer);
             pIntTrans[1]  = NULL;
         	}
        }        
        return;
        return;
    case LIBUSB_TRANSFER_TIMED_OUT:
        printf("LIBUSB_TRANSFER_TIMED_OUT\n");
        r = LIBUSB_ERROR_TIMEOUT;
        break;
    case LIBUSB_TRANSFER_STALL:
        printf("LIBUSB_TRANSFER_STALL\n");
        r = LIBUSB_ERROR_PIPE;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        printf("LIBUSB_TRANSFER_NO_DEVICE\n");
        r = LIBUSB_ERROR_NO_DEVICE;
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        printf("LIBUSB_TRANSFER_OVERFLOW\n");
        r = LIBUSB_ERROR_OVERFLOW;
        break;
    case LIBUSB_TRANSFER_ERROR:
         printf("LIBUSB_TRANSFER_ERROR\n");
         r = LIBUSB_ERROR_IO;
         break;
    case LIBUSB_TRANSFER_CANCELLED:
        printf("LIBUSB_TRANSFER_CANCELLED\n");
        r = LIBUSB_ERROR_IO;
        break;
    default:
        printf("LIBUSB_ERROR_OTHER\n");
        r = LIBUSB_ERROR_OTHER;
    }

    libusb_free_transfer(transfer);
    {   pIntTrans[1]  = NULL;
    	  printf("1 released \n");
    }  
}

void  IntTransCallback2(struct libusb_transfer *transfer)
{
    int r;
    float realVol;
    // printf("IntTransCallback 2\n");
    switch (transfer->status)
    {
    case LIBUSB_TRANSFER_COMPLETED:
        printf("LIBUSB_TRANSFER_COMPLETED 2\n");
        r = transfer->actual_length;
        //if(r == 0)
        //{
        		//libusb_submit_transfer(transfer);
        	//	return;
        //}
        
        
        /*for(int i = 0; i < r / 16; i++)
        {
         printf("cycles %d",cycles + i);
         for(int j = 0; j < 8;j++)
         {
         	if(gRecBuff[i*8+j]&0x8000)
         		{
         			gRecBuff[i*8+j] = ~gRecBuff[i*8+j];
         			realVol = -1 * MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		} 
         	else
         		{
         			realVol = MaxVol * (gRecBuff[i*8+j] + 1) / 32768;
         		}
         	//printf("% 2.6f,   ",realVol);
         	
         }
         printf("\n");
        }*/
        /*if(transfer == pIntTrans[0])
          memcpy(&SamplesData[cycles * 8],&gRecBuff[0],r); 
        else if(transfer == pIntTrans[1])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[1],r); 
         else if(transfer == pIntTrans[2])
        	memcpy(&SamplesData[cycles * 8],&gRecBuff[2],r); 
         else	
         	return;*/
        cycles += r / 16;
        printf("cycles %d,r = %d\n",cycles,r);
        if(cfg.dwMaxCycles && cfg.dwMaxCycles == cycles)
        {        	
        if(bSampled)
         	{ 
         		bSampled = false;
           
            request_exit(1);
          }
           CancelCode |= 0x04;
           libusb_free_transfer(transfer);
           pIntTrans[2]  = NULL;
        }	  
        else 
        {
         if(!bCanceled)
         	 libusb_submit_transfer(transfer);	
         else
          {
         		 libusb_free_transfer(transfer);
             pIntTrans[2]  = NULL;
         	}
        }             
        return;
    case LIBUSB_TRANSFER_TIMED_OUT:
        printf("LIBUSB_TRANSFER_TIMED_OUT\n");
        r = LIBUSB_ERROR_TIMEOUT;
        break;
    case LIBUSB_TRANSFER_STALL:
        printf("LIBUSB_TRANSFER_STALL\n");
        r = LIBUSB_ERROR_PIPE;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        printf("LIBUSB_TRANSFER_NO_DEVICE\n");
        r = LIBUSB_ERROR_NO_DEVICE;
        break;
    case LIBUSB_TRANSFER_OVERFLOW:
        printf("LIBUSB_TRANSFER_OVERFLOW\n");
        r = LIBUSB_ERROR_OVERFLOW;
        break;
    case LIBUSB_TRANSFER_ERROR:
         printf("LIBUSB_TRANSFER_ERROR\n");
         r = LIBUSB_ERROR_IO;
         break;
    case LIBUSB_TRANSFER_CANCELLED:
        printf("LIBUSB_TRANSFER_CANCELLED\n");
        r = LIBUSB_ERROR_IO;
        break;
    default:
        printf("LIBUSB_ERROR_OTHER\n");
        r = LIBUSB_ERROR_OTHER;
    }

    libusb_free_transfer(transfer);
   {   pIntTrans[2]  = NULL;
    	  printf("2 released \n");
    }  
}
