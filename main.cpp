#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 

//typedef long INT32;
//typedef unsigned short int INT16;
typedef unsigned char U_CHAR;

#define UCH(x)	((int) (x))
#define GET_2B(array,offset)  ((INT16) UCH(array[offset]) + \
			       (((INT16) UCH(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((INT32) UCH(array[offset]) + \
			       (((INT32) UCH(array[offset+1])) << 8) + \
			       (((INT32) UCH(array[offset+2])) << 16) + \
			       (((INT32) UCH(array[offset+3])) << 24))
#define FREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

#define RATIO 8

void set_4B(U_CHAR *array, int offset, INT32 value);

int ReadDataSize(char *name);
void ReadImageData(char *name, U_CHAR *bmpfileheader, U_CHAR *bmpinfoheader, U_CHAR *color_table, U_CHAR *data);

int rgbToGray(int);
int compare(int[],int[]);


int main()
{
   //專案目錄底下的src資料夾 路徑
   //char InputPath[65535] = "D:\\program homework\\Perceptual hash algorithm\\Perceptual hash algorithm\\src\\"; 
   char InputPath[65535] = "src\\";
   char szDir[65535]; 
   char dir[65535];
   char newdir[65535];
   WIN32_FIND_DATA FileData; 
   HANDLE          hList; 

   FILE *input_file = 0 ;
   FILE *output_file = 0 ;
   
   U_CHAR bmpfileheader1[14] = { 0 } ;
   U_CHAR bmpinfoheader1[40] = { 0 } ;
 
   U_CHAR bmpfileheader2[14] = { 0 } ;
   U_CHAR bmpinfoheader2[40] = { 0 } ;

   U_CHAR *data1, *data2, *new_data1,*new_data2,color_table1[1024], color_table2[1024];
   
   U_CHAR new_bmpfileheader1[14] = { 0 } ;
   U_CHAR new_bmpinfoheader1[40] = { 0 } ;

   U_CHAR new_bmpfileheader2[14] = { 0 } ;
   U_CHAR new_bmpinfoheader2[40] = { 0 } ;
 
 
   INT32 biWidth1 = 0 ;		
   INT32 biHeight1 = 0 ;

   INT32 biWidth2 = 0 ;		
   INT32 biHeight2 = 0 ;

   int i, j, k, temp,counter;
   int histo_table[256] = { 0 };

//掃目錄
	sprintf(szDir, "%s\\*", InputPath );

	if ( (hList = FindFirstFile(szDir, &FileData)) == INVALID_HANDLE_VALUE )
		printf("No files found\n\n");
	else {
		counter = 0;
		while (1) {
		    if (!FindNextFile(hList, &FileData)) {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                    break;
            }
			//把src資料夾路徑下找檔案
			sprintf(dir, "%s\\%s", InputPath, FileData.cFileName);
			//printf("%s\n", dir);

//第一次掃不到路徑下檔案，所以從第二次開始讀
if(counter>0){
   i = ReadDataSize("image.bmp");
   data1 = (U_CHAR *)malloc( i );
   if (data1 == NULL) {
      exit(0);
   }
  
   ReadImageData("image.bmp", bmpfileheader1, bmpinfoheader1, color_table1, data1);
   biWidth1           =   GET_4B(bmpinfoheader1,4);
   biHeight1          =   GET_4B(bmpinfoheader1,8);

   //


   new_data1 = (U_CHAR *)malloc( RATIO*RATIO );
   if (new_data1 == NULL) {
      exit(0);
   }
   
   i = ReadDataSize(dir);
   data2 = (U_CHAR *)malloc( i );
   if (data2 == NULL) {
      exit(0);
   }

   ReadImageData(dir, bmpfileheader2, bmpinfoheader2, color_table2, data2);
   biWidth2           =   GET_4B(bmpinfoheader2,4);
   biHeight2          =   GET_4B(bmpinfoheader2,8);

   new_data2 = (U_CHAR *)malloc( RATIO*RATIO );
   if (new_data2 == NULL) {
      exit(0);
   }
   

   // Process the file

/****** 1.將圖片縮小到8x8的尺寸，總共64個像素。******/
//這一步的作用是去除圖片的細節，只保留結構、明暗等基本信息，摒棄不同尺寸、比例帶來的圖片差異。
   int p = 0;
   int RATIOH=biHeight1/(RATIO-1);
   int RATIOW=biWidth1/(RATIO-1);
   for (i=0; i < biHeight1; i++)
   {
       if (i%RATIOH == 0)
	   {
			k = i* ((biWidth1*1 +3)/4 *4);
			for (j=0; j < biWidth1; j++)
			{
				if (j%RATIOW == 0)
				{
					new_data1[p] = data1[k];
					p = p+1;
				}
	            k = k+1;
			}
       }
   }

//第二張

   p = 0;
   RATIOH=biHeight2/(RATIO-1);
   RATIOW=biWidth2/(RATIO-1);
   for (i=0; i < biHeight2; i++)
   {
       if (i%RATIOH == 0)
	   {
			k = i* ((biWidth2*1 +3)/4 *4);
			for (j=0; j < biWidth2; j++)
			{
				if (j%RATIOW == 0)
				{
					new_data2[p] = data2[k];
					p = p+1;
				}
	            k = k+1;
			}
       }
   }

 /******第二步，簡化色彩。將縮小後的圖片，轉為64級灰度。也就是說，所有像素點總共只有64種顏色。******/
   int pixels1[RATIO*RATIO];
   for(int i=0;i<RATIO*RATIO;i++)
	   pixels1[i] = rgbToGray(new_data1[i]);
//第二張
   int pixels2[RATIO*RATIO];
   for(int i=0;i<RATIO*RATIO;i++)
	   pixels2[i] = rgbToGray(new_data2[i]);

/******第三步，計算平均值。計算所有64個像素的灰度平均值。******/
   int avgPixel = 0;  
   int m = 0; 

   for(int i=0;i<RATIO*RATIO;i++)
	   m = m + pixels1[i];

   avgPixel = m/(RATIO*RATIO);
//第二張
   int avgPixel2 = 0;  
   int m2 = 0; 

   for(int i=0;i<RATIO*RATIO;i++)
	   m2 = m2 + pixels2[i];

   avgPixel2 = m2/(RATIO*RATIO);

/******第四步，比較像素的灰度。將每個像素的灰度，與平均值進行比較。大於或等於平均值，記為1；小於平均值，記為0。******/
   int comps[RATIO*RATIO];
   for (int i = 0; i <RATIO*RATIO; i++) {  
      if(pixels1[i] >= avgPixel) {  
          comps[i] = 1;  
      }else {  
          comps[i] = 0;  
      }  
    } 
//第二張
   int comps2[RATIO*RATIO];
   for (int i = 0; i <RATIO*RATIO; i++) {  
      if(pixels2[i] >= avgPixel2) {  
          comps2[i] = 1;  
      }else {  
          comps2[i] = 0;  
      }  
    } 
/******第五步，計算hash值。看看64位中有多少位是不一樣的。即計算"漢明距離"（Hammingdistance）。******/
   for(int i=0;i<RATIO*RATIO;i++)
	   printf("%d",comps[i]);
   printf("\n");
//第二張
   for(int i=0;i<RATIO*RATIO;i++)
	   printf("%d",comps2[i]);
   printf("\n");
/******如果不相同的數據位不超過5，就說明兩張圖片很相似；如果大於10，就說明這是兩張不同的圖片。******/
   int Hammingdistance;
   Hammingdistance = compare(comps,comps2);

   /*
   for (i=0; i < 14; i++)
		new_bmpfileheader1[i] = bmpfileheader1[i];
   for (i=0; i < 40; i++)
		new_bmpinfoheader1[i] = bmpinfoheader1[i];
   set_4B(new_bmpfileheader1, 2, 64+1078);
   set_4B(new_bmpinfoheader1, 4, 8);
   set_4B(new_bmpinfoheader1, 8, 8);
   set_4B(new_bmpinfoheader1, 20, 64);

   for (i=0; i < 14; i++)
		new_bmpfileheader2[i] = bmpfileheader2[i];
   for (i=0; i < 40; i++)
		new_bmpinfoheader2[i] = bmpinfoheader2[i];
   set_4B(new_bmpfileheader2, 2, 64+1078);
   set_4B(new_bmpinfoheader2, 4, 8);
   set_4B(new_bmpinfoheader2, 8, 8);
   set_4B(new_bmpinfoheader2, 20, 64);


   // 開啟新檔案

   if( ( output_file = fopen("1.bmp","wb") ) == NULL ){
      fprintf(stderr,"Output file can't open.\n");
      exit(0);
   }

   fwrite(new_bmpfileheader1, sizeof(bmpfileheader1), 1, output_file);
   fwrite(new_bmpinfoheader1, sizeof(bmpinfoheader1), 1, output_file);

   fwrite(color_table1, 1024, 1, output_file);
 
   fwrite(new_data1,64, 1, output_file);
 
   fclose (output_file);

   */


//如果圖片 極相似 則輸出此檔案( 0 <= Hammingdistance < 5 到dst1)
   if(Hammingdistance >= 0 && Hammingdistance < 5){
   
	//把檔案丟到dst資料夾路徑下
	sprintf(newdir, "dst1\\%s", FileData.cFileName);
	//printf("%s\n", dir);

	//來源檔案,目的檔案,若巳存在則不要覆蓋
	CopyFile(dir,newdir,false);
   }

//如果圖片 很相似 則輸出此檔案( 5 <= Hammingdistance < 10 到dst2)
   if(Hammingdistance >= 5 && Hammingdistance < 10){
   
	//把檔案丟到dst資料夾路徑下
	sprintf(newdir, "dst2\\%s", FileData.cFileName);
	//printf("%s\n", dir);

	//來源檔案,目的檔案,若巳存在則不要覆蓋
	CopyFile(dir,newdir,false);
   }

//如果圖片 有些相似 則輸出此檔案( 10 <= Hammingdistance < 15 到dst3)
   if(Hammingdistance >= 10 && Hammingdistance < 15){
   
	//把檔案丟到dst資料夾路徑下
	sprintf(newdir, "dst3\\%s", FileData.cFileName);
	//printf("%s\n", dir);

	//來源檔案,目的檔案,若巳存在則不要覆蓋
	CopyFile(dir,newdir,false);
   }



/*
   if( ( output_file = fopen(dir,"wb") ) == NULL ){
      fprintf(stderr,"Output file can't open.\n");
      exit(0);
   }

   fwrite(bmpfileheader2, sizeof(bmpfileheader2), 1, output_file);
   fwrite(bmpinfoheader2, sizeof(bmpinfoheader2), 1, output_file);

   fwrite(color_table2, 1024, 1, output_file);
 
   fwrite(data2,biWidth2*biHeight2, 1, output_file);
 
   fclose (output_file);
*/
   free(data2);
   free(new_data2);

  

            
			} //end else
			counter++;
		} //end while
	} //end if
	FindClose(hList);
//掃目錄end

   free(data1);
   free(new_data1);

   system("pause");
   return 0;
}

int ReadDataSize(char *name)
{
   FILE *input_file = 0 ;
   U_CHAR bmpfileheader[14] = { 0 } ;
   U_CHAR bmpinfoheader[40] = { 0 } ;
   
   INT32 biWidth = 0 ;		
   INT32 biHeight = 0 ;
   INT16 BitCount = 0 ;

   /* 開啟檔案 */
   if( ( input_file = fopen(name,"rb") ) == NULL ){
      fprintf(stderr,"File can't open.\n");
      exit(0);
   }

   FREAD(input_file,bmpfileheader,14);
   FREAD(input_file,bmpinfoheader,40);

   if (GET_2B(bmpfileheader,0) == 0x4D42) /* 'BM' */
      fprintf(stdout,"BMP file.\n");
   else{
      fprintf(stdout,"Not bmp file.\n");
      exit(0);
   }

   biWidth           =   GET_4B(bmpinfoheader,4);
   biHeight          =   GET_4B(bmpinfoheader,8);
   BitCount          =   GET_2B(bmpinfoheader,14);

   if (BitCount != 8) {
      fprintf(stderr,"Not a 8-bit file.\n");
      fclose (input_file);
      exit(0);
   }

   // 
   fclose (input_file);

   return ((biWidth*1 +3)/4 *4)*biHeight*1;
}

void ReadImageData(char *name, U_CHAR *bmpfileheader, U_CHAR *bmpinfoheader, U_CHAR *color_table, U_CHAR *data)
{
   FILE *input_file = 0 ;
   
   INT32 FileSize = 0 ;
   INT32 bfOffBits =0 ;
   INT32 headerSize =0 ;
   INT32 biWidth = 0 ;		
   INT32 biHeight = 0 ;
   INT16 biPlanes = 0 ;
   INT16 BitCount = 0 ;
   INT32 biCompression = 0 ;
   INT32 biImageSize = 0;
   INT32 biXPelsPerMeter = 0 ,biYPelsPerMeter = 0 ;
   INT32 biClrUsed = 0 ;
   INT32 biClrImp = 0 ;

   /* 開啟檔案 */
   if( ( input_file = fopen(name,"rb") ) == NULL ){
      fprintf(stderr,"File can't open.\n");
      exit(0);
   }

   FREAD(input_file,bmpfileheader,14);
   FREAD(input_file,bmpinfoheader,40);

   if (GET_2B(bmpfileheader,0) == 0x4D42) /* 'BM' */
      fprintf(stdout,"BMP file.\n");
   else{
      fprintf(stdout,"Not bmp file.\n");
      exit(0);
   }

   FileSize           =   GET_4B(bmpfileheader,2);
   bfOffBits         =   GET_4B(bmpfileheader,10);
   headerSize      =   GET_4B(bmpinfoheader,0);
   biWidth           =   GET_4B(bmpinfoheader,4);
   biHeight          =   GET_4B(bmpinfoheader,8);
   biPlanes          =   GET_2B(bmpinfoheader,12);
   BitCount          =   GET_2B(bmpinfoheader,14);
   biCompression   =   GET_4B(bmpinfoheader,16);
   biImageSize      =   GET_4B(bmpinfoheader,20);
   biXPelsPerMeter =   GET_4B(bmpinfoheader,24);
   biYPelsPerMeter =   GET_4B(bmpinfoheader,28);
   biClrUsed         =   GET_4B(bmpinfoheader,32);
   biClrImp          =   GET_4B(bmpinfoheader,36);

   printf("FileSize = %ld \n"
	"DataOffset = %ld \n"
           "HeaderSize = %ld \n"
	"Width = %ld \n"
	"Height = %ld \n"
	"Planes = %d \n"
	"BitCount = %d \n"
	"Compression = %ld \n"
	"ImageSize = %ld \n"
	"XpixelsPerM = %ld \n"
	"YpixelsPerM = %ld \n"
	"ColorsUsed = %ld \n"
	"ColorsImportant = %ld \n",FileSize,bfOffBits,headerSize,biWidth,biHeight,biPlanes,
	BitCount,biCompression,biImageSize,biXPelsPerMeter,biYPelsPerMeter,biClrUsed,biClrImp);

   if (BitCount != 8) {
      fprintf(stderr,"Not a 8-bit file.\n");
      fclose (input_file);
      exit(0);
   }

   FREAD(input_file,color_table,1024);

   //
   
   //
   fseek(input_file,bfOffBits,SEEK_SET);
   FREAD(input_file,data,((biWidth*1 +3)/4 *4)*biHeight*1);
   // 
   fclose (input_file);
}

void set_4B(U_CHAR *array, int offset, INT32 value)
{
	INT32 i;
	i = value;
	array[offset] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+1] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+2] = (U_CHAR) (i % 256);
	i = i >> 8;
	array[offset+3] = (U_CHAR) (i % 256);
}

int rgbToGray(int pixels) {  
       // int _alpha =(pixels >> 24) & 0xFF;  
       int _red = (pixels >> 16) & 0xFF;  
       int _green = (pixels >> 8) & 0xFF;  
       int _blue = (pixels) & 0xFF;  
       return (int) (0.3 * _red + 0.59 * _green + 0.11 * _blue);  
}  

int compare(int image1[RATIO*RATIO],int image2[RATIO*RATIO]){

	int count = 0;

	for(int i=0;i<RATIO*RATIO;i++){
		if(image1[i]!=image2[i])
			count++;
	}
	return count;
}