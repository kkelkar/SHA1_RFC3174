#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<inttypes.h>

#define WordSize 32
#define TotalOctets 64
#define TotalBits 512

uint64_t ceiling(uint64_t x)
{
	uint64_t res=(x%TotalBits==0) ? (x/TotalBits) : (x/TotalBits)+1;
	return(res);
}

uint32_t rotl(uint32_t val, uint32_t pos)
{
	uint32_t res=val;
	return((res<<pos) | (res>>(sizeof(uint32_t)*CHAR_BIT-pos)));
}

uint32_t f(int t, uint32_t B, uint32_t C, uint32_t D)
{
	switch (t)
	{
		case 0:
		return((B&C) | ((~B)&D));
		break;
		
		case 1:
		return(B^C^D);
		break;

		case 2:
		return((B&C) | (B&D) | (C&D));
		break;

		case 3:
		return(B^C^D);
		break;
	}
}

void compute_digest(uint32_t *d, uint32_t *m)
{
	const uint32_t K[4]={0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6};
	uint32_t *buffer=(uint32_t *)calloc(80,sizeof(uint32_t));
	int i;
	memcpy(buffer,m,TotalOctets);
	for(i=16;i<80;i++)
	{
		buffer[i]=rotl((buffer[i-3])^(buffer[i-8])^(buffer[i-14])^(buffer[i-16]),1);
	}
	
	uint64_t V=0;
	/*begin*/
	for(i=0;i<80;i++)
	{	
		if(i>=0 && i<=19)
			V=d[4]+rotl(d[0],5)+buffer[i]+K[0]+f(0,d[1],d[2],d[3]);
		if(i>=20 && i<=39)
			V=d[4]+rotl(d[0],5)+buffer[i]+K[1]+f(1,d[1],d[2],d[3]);
		if(i>=40 && i<=59)	
			V=d[4]+rotl(d[0],5)+buffer[i]+K[2]+f(2,d[1],d[2],d[3]);
		if(i>=60 && i<=79)
			V=d[4]+rotl(d[0],5)+buffer[i]+K[3]+f(3,d[1],d[2],d[3]);
		d[4]=d[3];
		d[3]=d[2];
		d[2]=rotl(d[1],30);	
		d[1]=d[0];
		d[0]=V%4294967296;
	}
}

void main(int argc, char *argv[])
{
	int i;
	uint8_t *message=argv[1];
	uint64_t MsgLength=strlen(message);
	printf("Input string:\n%s\n",message);	

	/*message padding*/
	uint64_t NumBlocks=(MsgLength*sizeof(uint8_t)*CHAR_BIT)+64;
	NumBlocks=(NumBlocks%TotalBits==0) ? ceiling(NumBlocks)+1 : ceiling(NumBlocks);
	uint8_t *OldMsg=(uint8_t *)calloc((TotalOctets*NumBlocks),sizeof(uint8_t));
	strcpy(OldMsg,message);	
	OldMsg[MsgLength]=0x80;
	MsgLength=strlen(message)*sizeof(uint8_t)*CHAR_BIT;
	for(i=0;i<8;i++)
	{
		OldMsg[TotalOctets*NumBlocks-i-1]=(MsgLength>>(i*8)) & 0xff;
	}
	
	/*converting to 32 bit array*/
	MsgLength=TotalOctets*NumBlocks*CHAR_BIT/WordSize;
	uint32_t *PaddedMsg=(uint32_t *)calloc(MsgLength,sizeof(uint32_t));
	int j;
	for(i=0,j=0;i<MsgLength;i++,j+=4)
	{
		PaddedMsg[i]=(PaddedMsg[i]) | OldMsg[j];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j+1];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j+2];
		PaddedMsg[i]=(PaddedMsg[i]<<8) | OldMsg[j+3];
	}
	
	/*initialize variables*/
	uint32_t digest[5]={0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};
	uint32_t dd[5]={0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000};

	/*digest 512 bits at a time*/
	for(i=0;i<MsgLength;i+=16)
	{
		/*get previous result*/
		dd[0]=digest[0];
		dd[1]=digest[1];
		dd[2]=digest[2];
		dd[3]=digest[3];
		dd[4]=digest[4];
		uint32_t *CurrentBlock=(uint32_t *)calloc(TotalBits/WordSize,sizeof(uint32_t));
		for(j=0;j<16;j++)
		{
			CurrentBlock[j]=PaddedMsg[j+i];
		}
		compute_digest(digest,CurrentBlock);
		
		digest[0]+=dd[0];
		digest[1]+=dd[1];
		digest[2]+=dd[2];
		digest[3]+=dd[3];
		digest[4]+=dd[4];
	}
	
	printf("Hash:\n%"PRIx32"%"PRIx32"%"PRIx32"%"PRIx32"%"PRIx32"\n", digest[0],digest[1],digest[2],digest[3],digest[4]);
}
