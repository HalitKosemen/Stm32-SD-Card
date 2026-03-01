/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
#include "fonts.h"
#include <inttypes.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint32_t Kontrol_Bitleri = 0x00000000;
FATFS FatFs;
FIL Fil;
FRESULT FR_Status;
FATFS *FS_Ptr;
UINT RWC, WWC; // Read/Write Word Counter
DWORD FreeClusters;
uint32_t TotalMb, FreeMb;
char RW_Buffer[200];
char TxBuffer[250];
char ReadBuffer[512];

uint32_t x[100];
uint32_t ReadBufferData[512];

uint16_t data_res = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t SD_Card_Kontrol();
static void UART_Print(char* str);
void SD_Card_Bilgi();
void SD_Card_ListDirectory_Rec(const char *path, int depth);
void SD_CardListFile(void);
FRESULT SD_CardCreateFolder(const char *path);
FRESULT SD_CardDeleteFolder(const char *path);
FRESULT SD_CardCreateExtensionFile(const char *path);
FRESULT SD_CardCreateExtensionFileAlways(const char *path);
FRESULT SD_CardDeleteExtensionFile(const char *path);
FRESULT SD_CardOnlyWriteString(const char *filename, const char *text);
FRESULT SD_CardOnlyReadString(const char *filename, char *buffer, uint32_t buffersize);
FRESULT SD_CardOnlyWriteData(const char *filename, const uint32_t *values, uint32_t count);
FRESULT SD_CardOnlyReadData(const char *filename, uint32_t *buffer, uint32_t buffersize);
FRESULT SD_CardClearFileData(const char *filename);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void UART_Print(char* str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *) str, strlen(str), 100);
}

/*BİLGİ
 * İLK OLARAK BU FONKSİYONU ÇAĞIRMASILIN SD CARD OKEY Mİ DİYE !
 * EĞER FONKSİYON 0 DÖNDÜRÜRSE SD CARD HATASI !
 * EĞER FONKSİYON 1 DÖNDÜRÜRSE SD CARD BAŞARILI
 * Kontrol_Bitleri  ->  0. BİTİ KONTROL AMACI İLE KONTROL EDEBİLİRSİN  0-> HATA , 1->OKEY
 * OLUŞAN HATADA FRESULT STRUCT YAPISINA GİDİP HANGİ HATADAN DOLAYI OLUŞTUĞUNU İNCELEYEBİLİRSİN*/
uint8_t SD_Card_Kontrol(){
	FR_Status = f_mount(&FatFs, "", 1);
	if (FR_Status != FR_OK){
		sprintf(TxBuffer, "Error! While Mounting SD Card, Error Code: (%i)\r\n", FR_Status);
		UART_Print(TxBuffer);
		return 0;
	}
	sprintf(TxBuffer, "SD Card Mounted Successfully! \r\n\n");
	UART_Print(TxBuffer);
	return 1;
}
/*BİLGİ
 * SD KART HAKKINDA BİLGİ VEREN FONKSİYON
 * TotalMb ve FreeMb değerleri ile MB cinsinden SD Card Boyutunu printf eden fonksiyonlar
 * Herhangi bi boyut kontrolü yapıcaksan TotalMb ve FreeMb kontrol edebilirsin.
 * */
void SD_Card_Bilgi(){
    f_getfree("", &FreeClusters, &FS_Ptr);
    uint64_t TotalSize = (uint64_t)(FS_Ptr->n_fatent - 2) * (uint64_t)FS_Ptr->csize * (uint64_t)FS_Ptr->ssize;
    uint64_t FreeSpace = (uint64_t)FreeClusters * (uint64_t)FS_Ptr->csize * (uint64_t)FS_Ptr->ssize;
    TotalMb = (uint32_t)(TotalSize /(1024UL * 1024UL));
    FreeMb = (uint32_t)(FreeSpace / (1024UL * 1024UL));
    sprintf(TxBuffer, "SD Kart Toplam: %lu MB\r\n", TotalMb);  // ≈ 14927 MB (15.65 GB'ye denk)
    UART_Print(TxBuffer);
    sprintf(TxBuffer, "SD Kart Bos: %lu MB\r\n\n", FreeMb);
    UART_Print(TxBuffer);
}

/*BİLGİ
 *SD_Card_ListDirectory_Rec ve  SD_CardListFile fonksiyonları birdir.
 *Bu fonksiyonlar MicroSd kart içerisindeki Klasorleri ve Klasordeki
 *File yapılarının boyutları ve isimleri ile beraber infosunu verir.*/
void SD_Card_ListDirectory_Rec(const char *path, int depth){
	DIR dir;
	FILINFO fno;
	FRESULT res = f_opendir(&dir, path);
	if(res != FR_OK){
		sprintf(TxBuffer,"%*s[ERR] Cannot open: %s\r\n", depth * 2, "", path);
		UART_Print(TxBuffer);
		return;
	}
	while(1){
		res = f_readdir(&dir, &fno);
		if(res != FR_OK || fno.fname[0] == 0)break;
		const char *name = fno.fname;

		if(fno.fattrib & AM_DIR){
			if(strcmp(name,".") && strcmp(name,"..") == 0){
				continue;
			}
			sprintf(TxBuffer,"%*s📁 %s\r\n", depth * 2, "", name);
			UART_Print(TxBuffer);
			char newpath[256];
			snprintf(newpath, sizeof(newpath), "%s/%s", path, name);
			SD_Card_ListDirectory_Rec(newpath, depth + 1);
		}else{
			sprintf(TxBuffer,"%*s📄 %s (%lu bytes)\r\n",depth * 2, "", name, (unsigned long)fno.fsize);
			UART_Print(TxBuffer);
		}
	}
	f_closedir(&dir);
}
void SD_CardListFile(void){
	sprintf(TxBuffer, "📂 Files on SD Card:\r\n");
	UART_Print(TxBuffer);
	SD_Card_ListDirectory_Rec("0:/", 0);
	sprintf(TxBuffer, "\r\n\r\n");
	UART_Print(TxBuffer);
}


/*BİLGİ
 *SD_CardCreateFolder("0:/FOLDER");
 *genelde yukardaki gibi kullanılır 0:/ hangi depolama olduğunu belirtmek lazım
 *eğer zaten bir dosya adı kurulu ise Create directory Failed döndürüyor !
 *f_mkdir -> klasor oluşturmak için kullanılan fonksiyon düz klasor uzantısız. */
FRESULT SD_CardCreateFolder(const char *path){
	FRESULT res = f_mkdir(path);
	sprintf(TxBuffer,"Create directory %s: %s\r\n", path, (res == FR_OK ? "OK" : "Failed"));
	UART_Print(TxBuffer);
	return res;
}
/*BİLGİ
 * SD_CardDeleteFolder("0:/FOLDER");
 * genelde yukardaki gibi kullanılır 0:/ hangi depolama olduğunu belirtmek lazım
 * f_unlink -> klasor dosyalarını silmek için kullanılan fonksiyon.*/
FRESULT SD_CardDeleteFolder(const char *path){
	FRESULT res = f_unlink(path);
	sprintf(TxBuffer,"Delete directory %s: %s\r\n", path, (res == FR_OK ? "OK" : "Failed"));
	UART_Print(TxBuffer);
	return res;
}

/*BİLGİ
 * SD_CardCreateExtensionFile("0:/FOLDER/Extension.txt"); genel kullanımı
 * DOSYA YOKSA O DOSYAYI OLUŞTURUR
 * DOSYA VARSA FR_EXIST HATASI DÖNDÜRÜR
 * FONKSİYONDA EXTENSİON OLUŞTURDUKTAN SONRA HER ZAMAN f_close(&Fil) unutma yoksa orda kalır*/
FRESULT SD_CardCreateExtensionFile(const char *path){
	FIL Fil;
	FRESULT res = f_open(&Fil, path, FA_READ | FA_WRITE | FA_CREATE_NEW);
	if(res != FR_OK){
		sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", res);
		UART_Print(TxBuffer);
		return res;
	}
	sprintf(TxBuffer, "Extension File is Created.. \r\n\n");
	UART_Print(TxBuffer);
	f_close(&Fil);
	return res;
}
/*BİLGİ
 * AYNI MANTIK SADECE HER ZAMAN DOSYAYI OLUŞTURUR VAR OLSA BİLE SİLER TEKRARDAN OLUŞTURUR.*/
FRESULT SD_CardCreateExtensionFileAlways(const char *path){
	FIL Fil;
	FRESULT res = f_open(&Fil, path, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if(res != FR_OK){
		sprintf(TxBuffer, "Error! While Creating/Opening A New Text File, Error Code: (%i)\r\n", res);
		UART_Print(TxBuffer);
		return res;
	}
	sprintf(TxBuffer, "Extension File is Created..\r\n\n");
	UART_Print(TxBuffer);
	f_close(&Fil);
	return res;
}

FRESULT SD_CardDeleteExtensionFile(const char *path){
	FRESULT res = f_unlink(path);
	sprintf(TxBuffer,"Delete directory %s: %s\r\n", path, (res == FR_OK ? "OK" : "Failed"));
	UART_Print(TxBuffer);
	return res;
}

/*BİLGİ
 * var olan bir dosyayı açıp ona yazar okuma işlemi yapmaz -> FA_WRITE | FA_OPEN_EXISTING
 * Burda FA_OPEN_APPEND isteğe ihtiyaca göre değiştirilebilir
 * FA_OPEN_APPEND -> DOSYA YOKSA OLUŞTUR , VARSA DOSYA AÇ POİNTER OTOMATİK SONA KOY !*/
FRESULT SD_CardOnlyWriteString(const char *filename, const char *text){
	FIL Fil;
	FRESULT res;

	res = f_open(&Fil,filename,FA_WRITE | FA_OPEN_APPEND); //dosyayı aç imleci sona taşı
	if(res != FR_OK){
		sprintf(TxBuffer, "Error! While Opening A Text File, Error Code: (%i)\r\n", res);
		UART_Print(TxBuffer);
		return res;
	}

	res = f_puts(text, &Fil);
	if(res >= 0){
		sprintf(TxBuffer,"%s Dosyasına Yazıldı -> \%s\" (%d byte)\r\n",filename,text,res);
		UART_Print(TxBuffer);
	}else{
		sprintf(TxBuffer,"%s Dosyasına YAZMA HATASI -> f_puts = %d\r\n",filename,res);
		UART_Print(TxBuffer);
	}
	f_close(&Fil);
	return res;
}
/*BİLGİ
 * string verileri okumak için kullanılan fonksiyon
 * dikkat string buffer size [512] aşmamalı yoksa fonksiyon hata döndürür
 * [512] aşan string verileri için diğer fonksiyon kullanılmalı !*/
FRESULT SD_CardOnlyReadString(const char *filename, char *buffer, uint32_t buffersize){
	FIL Fil;
	FRESULT res;
	UINT br;

	if(buffer == NULL || buffersize == 0){
		sprintf(TxBuffer,"ReadBuffer is Not Clean First Clean The Buffer");
		UART_Print(TxBuffer);
		return FR_INVALID_PARAMETER;
	}
	buffer[0] = '\0';

	res = f_open(&Fil, filename, FA_READ | FA_OPEN_EXISTING);
	if(res != FR_OK){
		sprintf(TxBuffer, "Dosya acilamadi: %s → Hata: %d\r\n", filename, res);
		UART_Print(TxBuffer);
		return res;
	}

	FSIZE_t fsize = f_size(&Fil);
	if(fsize >= (buffersize - 1)){
		sprintf(TxBuffer, "Dosya cok buyuk! Boyut: %u, Buffer: %lu\r\n", (UINT)fsize, buffersize);
		UART_Print(TxBuffer);
		f_close(&Fil);
		return FR_DENIED;
	}

	res = f_read(&Fil, buffer, (UINT)fsize,&br);
	if((res != FR_OK) || (br != fsize)){
		sprintf(TxBuffer, "Okuma hatasi: %d (okunan: %u / beklenen: %u)\r\n", res, br, (UINT)fsize);
		UART_Print(TxBuffer);
		f_close(&Fil);
		return res;
	}

	buffer[br] = '\0';
	sprintf(TxBuffer, "%s dosyasindan okundu → \"%s\" (%u byte)\r\n", filename, buffer, br);
	UART_Print(TxBuffer);
	f_close(&Fil);
	return FR_OK;
}



/*BİLGİ
 * BU FONKSİYON BENİM İHTİYACIMA GÖRE DÜZENLENMİŞTİR
 * BU FONKSİYONU KULLANCAKSAN SD_CardOnlyWriteData("0:/FILE/Data.bin",x,sizeof(x))
 * burda x gönderceğin array uint32_t x[100] gibi , .bin dosyasına kaydet HxD progarmı ile de
 * .bin dosyasını okuyabilirsin !
 * */
FRESULT SD_CardOnlyWriteData(const char *filename, const uint32_t *values, uint32_t count){
	FIL Fil;
	FRESULT res;
	UINT bw;

	if(values == NULL || count == 0){
		sprintf(TxBuffer,"This function Be Wrong behavior fix this (0)!");
		UART_Print(TxBuffer);
		return FR_INVALID_PARAMETER;
	}
	res = f_open(&Fil, filename, FA_WRITE | FA_OPEN_APPEND);
	if(res != FR_OK){
		sprintf(TxBuffer,"This function Be Wrong behavior fix this (1)!");
		UART_Print(TxBuffer);
		return res;
	}

	UINT bytes_to_write = count; //array büyüklüğü kontrolü

	res = f_write(&Fil, values, bytes_to_write, &bw);
	if(res != FR_OK || bw != bytes_to_write){
		sprintf(TxBuffer,"This function Be Wrong behavior fix this (2)!");
		UART_Print(TxBuffer);
		f_close(&Fil);
		return res ? res : FR_INT_ERR;
	}
	f_close(&Fil);
	return FR_OK;
}


/*BİLGİ
 * BU FONKSİYON İLE uint32_t x[512] yani 2048 byte tan büyük bir data okucaksan
 * while() içindeki f_read den sonra bir kesme oluşturman lazım çünkü her [512] lik veride bir
 * buffer siliniyor ! kesme oluşturup orda [512] lik buffer değerini alıp işleyebilirsin.
 * */
FRESULT SD_CardOnlyReadData(const char *filename, uint32_t *buffer, uint32_t buffersize){
	FIL Fil;
	FRESULT res;
	UINT br;
	FSIZE_t kalan = 0;
	uint32_t toplam_eleman = 0;
	if(buffer == NULL || buffersize == 0){
		sprintf(TxBuffer,"ReadBuffer is Not Clean First Clean The Buffer");
		UART_Print(TxBuffer);
		return FR_INVALID_PARAMETER;
	}
	memset(buffer,0,buffersize);
	res = f_open(&Fil, filename, FA_READ | FA_OPEN_EXISTING);
	if(res != FR_OK){
		sprintf(TxBuffer, "Dosya acilamadi: %s → Hata: %d\r\n", filename, res);
		UART_Print(TxBuffer);
		return res;
	}


	FSIZE_t fsize = f_size(&Fil); // -> uint32_t'lik 50 veri varsa fsize = 50 * 4 ten = 200 olur.
	if(fsize % 4 != 0){
		sprintf(TxBuffer, "Dosya boyutu 4'un kati degil! Dosya içeriğini Kontrol Et (%lu byte)\r\n", fsize);
		UART_Print(TxBuffer);
		return fsize;
	}

	kalan = fsize;
	uint32_t buffer_byte = buffersize;

	while(kalan > 0){
		UINT bu_tur_okunan_byte = (kalan > buffer_byte) ? buffer_byte : (UINT)kalan;
		memset(buffer,0,buffersize);
		res = f_read(&Fil, buffer, bu_tur_okunan_byte, &br);
		//yukardaki f_read işleminden sonra [512] lik arrayler tek seferde buffer atılıyor
		//buffer okucaksan burda okuma işlemini yap bir sonraki while da 0 lanıyor !
		//yani bu kısımda bu while döngüsü içinde f_read den sonra bir kesme yapman şart!
		if(res != FR_OK || br != bu_tur_okunan_byte){
			sprintf(TxBuffer, "Dosya acilamadi:  → Hata: %d\r\n", res);
			UART_Print(TxBuffer);
			f_close(&Fil);
			return res;
			break;
		}
		uint32_t bu_tur_eleman = br / 4;

		sprintf(TxBuffer, "Parça: %lu eleman (toplam: %lu)\r\n", bu_tur_eleman, toplam_eleman + bu_tur_eleman);
		UART_Print(TxBuffer);

		toplam_eleman += bu_tur_eleman;
		kalan -= br;
	}
	f_close(&Fil);
	return FR_OK;
}



FRESULT SD_CardClearFileData(const char *filename){
	FIL Fil;
	FRESULT res;

	res = f_open(&Fil, filename, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
	if(res != FR_OK){
		sprintf(TxBuffer,"Dosya Temizlenmedi %s -> Hata %d\r\n",filename,res);
		UART_Print(TxBuffer);
		return res;
	}
	f_close(&Fil);
	sprintf(TxBuffer, "Dosya temizlendi: %s\r\n", filename);
	UART_Print(TxBuffer);
	return FR_OK;
}

//---------------------------------------------------------------------------------------




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

	for(uint32_t i = 0; i < 100; i++){
		x[i] = i * 1000 + 12345;
	}

	for(uint32_t i = 0; i < 512; i++){
		ReadBufferData[i] = 0xFFFFFFFF;
	}

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  Kontrol_Bitleri |= (SD_Card_Kontrol() << 0); //SD KART OKEY Mİ KONTROL (ÇALIŞIYOR)
  SD_Card_Bilgi(); //SD KART FREESPACE ve TOTALSPACE BİLGİ (ÇALIŞIYOR)

  SD_CardListFile(); //SD KART DOSYA BİLGİLERİ (ÇALIŞIYOR)
  SD_CardDeleteFolder("0:/GS");
  SD_CardDeleteFolder("0:/FENER");

  SD_CardCreateExtensionFileAlways("0:/WritePro.txt");
  //SD_CardCreateFolder("0:/FENER"); //SD KART CREATE FOLDER
  //SD_CardCreateFolder("0:/GS");

  SD_CardListFile();

  //SD_CardDeleteFolder("0:/GS");

  //SD_CardListFile();

  //SD_CardCreateExtensionFile("0:/FENER/Writes.txt");
  //SD_CardCreateExtensionFile("0:/FENER/Writes1.txt");

  //SD_CardListFile();

  //SD_CardDeleteExtensionFile("0:/FENER/Writes1.txt");
  //SD_CardListFile();
  //SD_CardCreateExtensionFileAlways("0:/FENER/Writes.txt");
  //SD_CardCreateExtensionFileAlways("0:/FENER/Data.bin");
  //SD_CardListFile();

  //SD_CardOnlyWriteString("0:/FENER/Writes.txt","MERHABA BENİM ADIM HALİT , SENİN ADIN NE ?"); //(ÇALIŞIYOR)
  //SD_CardListFile();

  //SD_CardOnlyReadString("0:/FENER/Writes.txt", ReadBuffer, sizeof(ReadBuffer));//(ÇALIŞIYOR)

  //SD_CardOnlyWriteData("0:/FENER/Data.bin", x, sizeof(x));
  //SD_CardListFile();

  //SD_CardOnlyReadData("0:/FENER/Data.bin",ReadBufferData,sizeof(ReadBufferData));
  //SD_CardListFile();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
