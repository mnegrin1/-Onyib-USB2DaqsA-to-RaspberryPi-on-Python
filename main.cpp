#include <stdio.h> 
// #include <pthread.h>
// #include <semaphore.h>
#include <libusb-1.0/libusb.h>
// #include <errnko.h>
#include <signal.h>
// #include <unistd.h>
#include "type.h"
//incluir delay
// #include <chrono> 
//#include "ad7606.cpp"


#define DllExport
DllExport HANDLE  M3F20xm_OpenDevice(libusb_context *pCtx);
DllExport bool M3F20xm_CloseDevice(HANDLE pDevHand);
DllExport bool M3F20xm_GetVersion(HANDLE pDevHand,BYTE byType,LPVOID lpBuffer);
DllExport bool M3F20xm_ADCGetConfig(HANDLE pDevHand, ADC_CONFIG* pCfge);
DllExport bool M3F20xm_ADCSetConfig(HANDLE pDevHand, ADC_CONFIG* pCfg);
DllExport bool M3F20xm_ADCRead(HANDLE pDevHand,WORD* lpReadBuffer);
DllExport bool M3F20xm_ADCStart(HANDLE pDevHand);
DllExport bool M3F20xm_ADCStop(HANDLE pDevHand);
DllExport bool M3F20xm_GetSerialNo(HANDLE pDevHand,char* lpBuff);
HANDLE pDevHandle = NULL;
volatile DWORD cycles;
ADC_CONFIG cfg = {0};
libusb_context *ctx;
libusb_hotplug_callback_handle callbackHandle;
bool bSampled;
float MaxVol;

// static void sighandler(int signum);

// static volatile sig_atomic_t do_exit = 0;

// static pthread_t poll_thread;
// static sem_t exit_sem;

/* Medición de tiempos */
// double time_start = 0, time_start_loop = 0, time_dif = 0, time_dif_loop = 0;

void request_exit(sig_atomic_t code)
{
	printf("request_exit = %d\n",code);
	if(bSampled)
	{
	 M3F20xm_ADCStop(pDevHandle); 	
	}
	// do_exit = code;
	// sem_post(&exit_sem);
}


// static void *poll_thread_main(void *arg)
// {
// 	int r = 0;
// 	printf("poll thread running\n");

// 	while (!do_exit) {
// 		struct timeval tv = { 1, 0 };
// 		r = libusb_handle_events_timeout(NULL, &tv);
// 		//printf("timeout = %d\n",r);
// 		if (r < 0) {
			
// 			request_exit(2);
// 			break;
// 		}
// 	}
// 	printf("poll thread shutting down\n");
// 	return NULL;
// }


int main()
{   
	// time_start = (double) clock();
	// time_start = time_start / CLOCKS_PER_SEC; /* En segundos*/

	// struct sigaction sigact;
	int r = 1;
	// r = sem_init(&exit_sem, 0, 0);
	// if (r) {
	// 	fprintf(stderr, "failed to initialise semaphore error %d", errno);
	// 	return -1;
	// }

	r = libusb_init(&ctx);

	if(r<0)
	{
		printf("libusb_init error\n");
		// sem_destroy(&exit_sem);
		return -1;
	}
	else{
		libusb_set_option(ctx,LIBUSB_OPTION_LOG_LEVEL);  
		pDevHandle = M3F20xm_OpenDevice(ctx);
	
		if(!pDevHandle)
		{
			printf("M3F20xm_OpenDevice fail\n"); 
			libusb_exit(ctx);		
			// sem_destroy(&exit_sem);	
			return -1;
		}

		/* async from here onwards */
		// sigact.sa_handler = sighandler;
		// sigemptyset(&sigact.sa_mask);
		// sigact.sa_flags = 0;
		// sigaction(SIGINT, &sigact, NULL);
		// sigaction(SIGTERM, &sigact, NULL);
		// sigaction(SIGQUIT, &sigact, NULL);
		
		// r = pthread_create(&poll_thread, NULL, poll_thread_main, NULL);

		// if (r)
		// {
		// 	printf("pthread_create fail\n"); 
		// 	libusb_exit(ctx);		
		// 	sem_destroy(&exit_sem);	
		// 	return -1;
		// }

		if(!M3F20xm_ADCGetConfig(pDevHandle,&cfg))//  read ADC
		{
			M3F20xm_CloseDevice(pDevHandle);
			pDevHandle = NULL;
			libusb_exit(ctx);
			// sem_destroy(&exit_sem);	
			return -1;
		}
	}

	#if FM_VER > 4
	{	
		cfg.byADCOptions = 0x10; //uint:us, rangle 10V~-10V, os 0, external ref
		cfg.byTrigOptions = 0x01; //period mode
		cfg.wPeriod = 10;    //100us
		cfg.dwMaxCycles = 7000; //continueous sampling
        }
	#else 
		cfg.wTrigSize = 3072;
		cfg.wPeriod = 100;    //100us
		cfg.dwMaxCycles = 0; //continueous sampling
	#endif

	if(!M3F20xm_ADCSetConfig(pDevHandle,&cfg))
	{
		printf("call M3F20xm_ADCSetConfig fail\n"); 
		M3F20xm_CloseDevice(pDevHandle);
		pDevHandle = NULL;
		libusb_exit(ctx);
		// sem_destroy(&exit_sem);	
		return -1;
	}	   

	M3F20xm_ADCGetConfig(pDevHandle,&cfg);
	printf("ADC Op = %02x\n",cfg.byADCOptions);
	printf("ADC Trigop = %02x\n",cfg.byTrigOptions);
	printf("ADC wPeriod = %d\n",cfg.wPeriod);	
	WORD adc_value[8];    
		
   if(cfg.byADCOptions & 0x10)
   	{
   		MaxVol = 10;
   	}
   else
   	{
   		MaxVol = 5;
   	}

    cycles = 0;

	int maxCiclos = 500; //======================== Lecturas máximas

	// time_start_loop = (double) clock()/CLOCKS_PER_SEC;


    // /* Tiempo antes de entrar al loop */
	// time_dif = ((double) clock() / CLOCKS_PER_SEC) - time_start;	

	

    bSampled = M3F20xm_ADCStart(pDevHandle);   //start ADC continuous sampling ;    

	while ((int)cycles < maxCiclos) {

    // Realizar la lectura del ADC en cada ciclo
		if (!M3F20xm_ADCRead(pDevHandle, adc_value)) {
			printf("call M3F20xm_ADCRead fail\n");
			// request_exit(1);
			break;
		}
		
		// Imprimir los valores leídos
		printf("0x: %04X %04X %04X %04X %04X %04X %04X %04X\n",
			adc_value[0], adc_value[1], adc_value[2], adc_value[3],
			adc_value[4], adc_value[5], adc_value[6], adc_value[7]);

		// // Vacía el buffer de adc_value
		// for(int i = 0 ; i < 8 ; i++)
		// {
		// 	adc_value[i] = 0;
		// }

		// Incrementar el contador de ciclos
		cycles++;
		
		// Imprimir el número de ciclos
		printf("cycles = %d\n", cycles);

		// Intentar obtener el semáforo de salida sin bloquear
		// if (sem_trywait(&exit_sem) == 0) 
		// {
		// 	break; // salir del bucle si el semáforo está disponible
		// }
		// clock_nanosleep((2280000/500)); 
		
	}

// Después de salir del bucle, verifica si se alcanzó el número máximo de ciclos
	if ((int)cycles >= maxCiclos) {
		//M3F20xm_ADCStop(pDevHandle);
		printf("Reached maximum cycles: %d\n", maxCiclos);
	 	M3F20xm_ADCStop(pDevHandle); 	
		// do_exit=1;
		// bSampled = 1;
		// request_exit(1);

		// printf("Tiempo antes de entrar al loop: %f\n", time_dif);
		// time_dif_loop = ((double) clock() / CLOCKS_PER_SEC) - time_start_loop;
		// printf("time_start = %lf, time_start_loop = %lf, time_dif = %lf, time_dif_loop = %lf\n ",time_start, time_start_loop, time_dif, time_dif_loop);
		// printf("Tiempo del loop: %lf\n", time_dif_loop);
	}

    printf("shutting down...\n");
    // pthread_join(poll_thread, NULL);

    M3F20xm_CloseDevice(pDevHandle);


    // if (do_exit == 1)
	// {
	// 	r = 0;
	// }
	// else
	// {
	// 	r = 1; 
		libusb_exit(ctx);
	// }

    // sem_destroy(&exit_sem);

	// double periodo = time_dif_loop / maxCiclos;
	// printf("Frecuencia: %lf\n", 1/periodo);

	return r >= 0 ? r : -r;
}

// static void sighandler(int signum)
// {
// 	request_exit(1);
// }
