#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VERBOSE 1
static int segment_num = 0;

void main(int argc, char*argv[]) {
	struct AIF {
		unsigned long int bl_decompress_code;
		unsigned long int bl_self_reloc_code;
		unsigned long int bl_dbg_int_zeroinit;
		unsigned long int bl_image_entry_point;
		unsigned long int program_exit_instr;
		unsigned long int image_readonly_size;
		unsigned long int image_readwrite_size;
		unsigned long int image_debug_size;
		unsigned long int image_zero_init_size;
		unsigned long int image_debug_type;
		unsigned long int image_base;
		unsigned long int work_space;
		unsigned long int address_mode;
		unsigned long int data_base;
		unsigned long int reserved1;
		unsigned long int reserved2;
		unsigned long int debug_init_instr;
	}	aif_header;
	struct FAT_AIF {
		unsigned long int file_offset_next;
		unsigned long int load_address;
		unsigned long int words;
		char			region_name[32];
	} fat_aif_header;

	char filename[80];
	FILE *aif_file, *out_file;
	unsigned long int i,j;
	int testi;
	unsigned long int checksum;
	unsigned char *checksum_ptr;
	unsigned long int count, next, address;
	unsigned int word;
	

	checksum = 0;
	printf("/************************************************/\n");
	printf("/*                                              */\n");
	printf("/*                                              */\n");
	printf("/*           Design: ZTE IWF GROUP              */\n");
	printf("/*           Author: wangjh                     */\n");
	printf("/*           IN:  Dir\\cv68011.ad                */\n"); /*understand the transferred meaning char*/
	printf("/*           OUT: CurDir\\cvms6dsp.h             */\n");
	printf("/*           press any key....                  */\n");
	printf("/*                                              */\n");
	printf("/*                                              */\n");
	printf("/************************************************/");
	getchar();
	if (argc != 2 ) 
	{
		printf("File Name?:");
		gets(filename);
	}
	else
	{
		strcpy (filename, *++argv);
	}
	printf("File = |%s|\n", filename);

	if ((aif_file = fopen(filename,"rb")) == NULL) 
	{
		printf("File not found\n");
		exit(1);
	}

	if ((out_file = fopen("csmv6dsp.h","w")) == NULL) 
	{
		printf("Could not be created\n");
		exit(1);
	}
	fprintf(out_file,"#ifndef _CSMV6_DSP_H\n");
	fprintf(out_file,"#define _CSMV6_DSP_H\n");
	fprintf(out_file,"/**************************************************************************/\n");
	fprintf(out_file,"/*        CSMV6DSP.h                                                      */\n");
	fprintf(out_file,"/*        Copyright 2001 by ZTE CDMA  All rights reserved.                */\n");
	fprintf(out_file,"/*        Author: wang jianhong                                           */\n");
	fprintf(out_file,"/*        Date:   2001.10.23                                              */\n");
	fprintf(out_file,"/*        Description: provide IWF CSMV6 DSP header file                  */\n");
	fprintf(out_file,"/*        NOTE: It execute as the IWF CSMV6 DSP header file               */\n");
	fprintf(out_file,"/*        Update:                                                         */\n");
	fprintf(out_file,"/**************************************************************************/\n");
	fprintf(out_file,"typedef unsigned short WORD;\n");
	fprintf(out_file,"typedef unsigned long  DWORD;\n\n");

	fprintf(out_file,"/**STARTx_DATA_SIZE is the size of word**/\n");
	fprintf(out_file,"WORD DSP_Checksum = ;\n\n");
	/*read the air_head and calculate the chksum of the header*/
	i = fread((char *) &aif_header,sizeof(aif_header),1, aif_file);
	checksum_ptr = (unsigned char *) &aif_header;
	for (i = 0; i < sizeof(aif_header); i++) {  
		checksum += *checksum_ptr++;
	}
#ifdef VERBOSE
	printf("the out file name is csmv6dsp.h \n");
#endif
	
#ifdef VERBOSE
	printf("Are you ready ?");
	getchar();
	printf("Please wait.");
	
#endif

	fseek(aif_file,(long ) 0x80,SEEK_SET);
	if(aif_header.image_readonly_size)/**only > 0 we deal it**/
	{
		segment_num++;
		count = 0;
		address = aif_header.image_base;
		fprintf(out_file,"static DWORD SDRAM_START1 = 0x%08x;  /*this is the code run address*/\n",address);
		fprintf(out_file,"DWORD START1_DATA_SIZE = (0x%08x + 14);\n",aif_header.image_readonly_size/2);	
    	fprintf(out_file,"WORD SDRAM_START1_DATA[0x%08x + 14] = { \n",(aif_header.image_readonly_size/2));
		fprintf(out_file,"/************ only for boot read ************/\n");
		for(i = 0;i < 6;i++)
				fprintf(out_file,"0x%d%d%d%d, ", segment_num,segment_num,segment_num,segment_num);
		fprintf(out_file,"\n0x%04x, 0x%04x, ",((address & 0xffff0000) >> 16),(address & 0xffff));
		fprintf(out_file,"0x%04x, 0x%04x, ",(((aif_header.image_readonly_size/2) & 0xffff0000) >> 16),((aif_header.image_readonly_size/2) & 0xffff));
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, \n", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"\n/************ only for boot read ************/\n");
	
		for (j = 0,checksum = 0 ; (unsigned)j < aif_header.image_readonly_size/2;j++) 
		{
			i = fread((char *)&word,2,1, aif_file);
			checksum_ptr = (unsigned char *) &word;
			/*for (i = 0; i < sizeof(word); i++) 
			{
				checksum += *checksum_ptr++;
			}*/	
			checksum ^= word; 
			/*printf("it is the 0x%x \n",word);*/
			fprintf(out_file,"0x%04x, ", (word & 0xffff));
			address +=4;
			if (++count == 6) 
			{
				count = 0;
				fprintf(out_file, "\n");
			}
		}
		fprintf(out_file,"\n};");
		fprintf(out_file,"\nstatic WORD Start1_Checksum = 0x%04x;\n\n", checksum);
	}
	printf(".");
	if(aif_header.image_readwrite_size) /**>0 we deal it**/
	{
		segment_num++;
		fprintf(out_file,"\n/*Read Write*/\n");
		address = aif_header.data_base;
		fprintf(out_file,"static DWORD SDRAM_START2 = 0x%08x;  \n",address);
		fprintf(out_file,"DWORD START2_DATA_SIZE = (0x%08x + 14);\n",aif_header.image_readwrite_size/2);		
    	fprintf(out_file,"WORD SDRAM_START2_DATA[0x%08x + 14] = { \n",(aif_header.image_readwrite_size/2));
		fprintf(out_file,"/************ only for boot read ************/\n");
		for(i = 0;i < 6;i++)
				fprintf(out_file,"0x%d%d%d%d, ", segment_num,segment_num,segment_num,segment_num);
		fprintf(out_file,"\n0x%04x, 0x%04x, ",((address & 0xffff0000) >> 16),(address & 0xffff));
		fprintf(out_file,"0x%04x, 0x%04x, ",(((aif_header.image_readwrite_size/2) & 0xffff0000) >> 16),((aif_header.image_readwrite_size/2) & 0xffff));
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, \n", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"\n/************ only for boot read ************/\n");
		count = 0;
	
		for (j = 0,checksum = 0; (unsigned)j < aif_header.image_readwrite_size/2;j++) 
		{
			i = fread((char *) &word,2,1, aif_file);
			//printf("%04x %08x\n", count,word);
			checksum ^= word;
			fprintf(out_file,"0x%04x, ", (word & 0xffff));
			address +=4;
			if (++count == 6) 
			{
				count = 0;
				fprintf(out_file, "\n");
			}
		}
		fprintf(out_file,"\n};\n");
		fprintf(out_file,"static WORD Start2_Checksum = 0x%04x;\n\n\n", checksum);
	}
	else
		testi = 1;
	printf(".");
	/*for debug we don't take care it*/
	if(aif_header.image_debug_size)
	{
		segment_num++;
		for (j = 0,checksum = 0; (unsigned)j < aif_header.image_debug_size/4;j++) 
		{
			i = fread((char *) &word,4,1, aif_file);
			checksum ^= word;
			count += 4;
		}
	}
	fseek(aif_file,(long ) aif_header.reserved1,SEEK_SET);
	
	next = aif_header.reserved1;
	if (next == 0) 
	{
		fprintf(out_file, "No scatter\n");
		printf("No scatter\n");
	}

	printf(".");
	testi = 1;
	while(next) 
	{
		i = fread((char *) &fat_aif_header,sizeof(fat_aif_header),1, aif_file);
		checksum_ptr = (unsigned char *) &fat_aif_header;
		for (i = 0,checksum = 0; i < sizeof(fat_aif_header); i++) 
		{
			checksum += *checksum_ptr++;
		}	
		if(fat_aif_header.words)
		{
			segment_num++;
			testi += 1;
			fprintf(out_file,"static DWORD SDRAM_START%d = 0x%08x;\n",testi,fat_aif_header.load_address);
			fprintf(out_file,"DWORD START%d_DATA_SIZE = (0x%08x + 14);\n",testi,fat_aif_header.words/2);	
			fprintf(out_file,"WORD SDRAM_START%d_DATA[0x%08x + 14] = {\n",testi,fat_aif_header.words/2);
			fprintf(out_file,"/************ only for boot read ************/\n");
		for(i = 0;i < 6;i++)
				fprintf(out_file,"0x%d%d%d%d, ", segment_num,segment_num,segment_num,segment_num);
		fprintf(out_file,"\n0x%04x, 0x%04x, ",((fat_aif_header.load_address & 0xffff0000) >> 16),(fat_aif_header.load_address & 0xffff));
		fprintf(out_file,"0x%04x, 0x%04x, ",(((fat_aif_header.words/2) & 0xffff0000) >> 16),((fat_aif_header.words/2) & 0xffff));
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, \n", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"0x%04x, ", 0x0000);
		fprintf(out_file,"\n/************ only for boot read ************/\n");
			count = 0;

			for (j = 0,checksum = 0; (unsigned)j < fat_aif_header.words/2;j++) 
			{
				i = fread((char *) &word,2,1, aif_file);
				checksum ^= word;

				fprintf(out_file,"0x%04x, ", (word & 0xffff));
				address +=4;
				if (++count == 6) 
				{
					count = 0;
					fprintf(out_file, "\n");
				}
			}
			fprintf(out_file,"\n};\n");
			fprintf(out_file,"static WORD Start%d_Checksum = 0x%04x;\n\n", testi,checksum);
			printf(".");
		}
		next = fat_aif_header.file_offset_next;
		fseek(aif_file,(long )next,SEEK_SET);
	}
	printf(".");
	fprintf(out_file,"static WORD all_segment_num = 0x%04x;\n",segment_num);
	fprintf(out_file,"#endif  /*#ifndef _CSMV6_DSP_H*/");
	fclose(out_file);
	fclose(aif_file);
	printf("..OK!");
	exit(0);
}
