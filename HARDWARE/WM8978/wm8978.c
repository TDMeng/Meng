#include "wm8978.h"
#include "myiic.h"
#include "delay.h"

static u16 WM8978_REGVAL_TBL[58]=
{
	0X0000,0X0000,0X0000,0X0000,0X0050,0X0000,0X0140,0X0000,
	0X0000,0X0000,0X0000,0X00FF,0X00FF,0X0000,0X0100,0X00FF,
	0X00FF,0X0000,0X012C,0X002C,0X002C,0X002C,0X002C,0X0000,
	0X0032,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,0X0000,
	0X0038,0X000B,0X0032,0X0000,0X0008,0X000C,0X0093,0X00E9,
	0X0000,0X0000,0X0000,0X0000,0X0003,0X0010,0X0010,0X0100,
	0X0100,0X0002,0X0001,0X0001,0X0039,0X0039,0X0039,0X0039,
	0X0001,0X0001
}; 

u8 WM8978_Init(void)
{ 
	u8 res;
	IIC_Init();             
	res=WM8978_Write_Reg(0,0);	
	if(res)return 1;		
	WM8978_Write_Reg(1,0X1B);	
	WM8978_Write_Reg(2,0X1B0);
	WM8978_Write_Reg(3,0X6C);
	WM8978_Write_Reg(6,0);		
	WM8978_Write_Reg(43,1<<4);
	WM8978_Write_Reg(47,1<<8);	
	WM8978_Write_Reg(48,1<<8);
	WM8978_Write_Reg(49,1<<1);	 
	WM8978_Write_Reg(49,1<<2);	
	WM8978_Write_Reg(10,1<<3);
	WM8978_Write_Reg(14,1<<3);
	return 0;
} 


u8 WM8978_Write_Reg(u8 reg,u16 val)
{ 
	IIC_Start(); 
	IIC_Send_Byte((WM8978_ADDR<<1)|0);
	if(IIC_Wait_Ack())return 1;	
    IIC_Send_Byte((reg<<1)|((val>>8)&0X01));
	if(IIC_Wait_Ack())return 2;
	IIC_Send_Byte(val&0XFF);
	if(IIC_Wait_Ack())return 3;	
    IIC_Stop();
	WM8978_REGVAL_TBL[reg]=val;	
	return 0;	
}  

u16 WM8978_Read_Reg(u8 reg)
{  
	return WM8978_REGVAL_TBL[reg];	
} 

void WM8978_ADDA_Cfg(u8 dacen,u8 adcen)
{
	u16 regval;
	regval=WM8978_Read_Reg(3);
	if(dacen)regval|=3<<0;	
	else regval&=~(3<<0);	
	WM8978_Write_Reg(3,regval);	
	regval=WM8978_Read_Reg(2);
	if(adcen)regval|=3<<0;	
	else regval&=~(3<<0);	
	WM8978_Write_Reg(2,regval);
}

void WM8978_Input_Cfg(u8 micen,u8 lineinen,u8 auxen)
{
	u16 regval;  
	regval=WM8978_Read_Reg(2);
	if(micen)regval|=3<<2;	
	else regval&=~(3<<2);		
 	WM8978_Write_Reg(2,regval);
	
	regval=WM8978_Read_Reg(44);	
	if(micen)regval|=3<<4|3<<0;	
	else regval&=~(3<<4|3<<0);
	WM8978_Write_Reg(44,regval);
	
	if(lineinen)WM8978_LINEIN_Gain(7);
	else WM8978_LINEIN_Gain(0);
	if(auxen)WM8978_AUX_Gain(7);
	else WM8978_AUX_Gain(0);
}

void WM8978_Output_Cfg(u8 dacen,u8 bpsen)
{
	u16 regval=0;
	if(dacen)regval|=1<<0;
	if(bpsen)
	{
		regval|=1<<1;	
		regval|=5<<2;	
	} 
	WM8978_Write_Reg(50,regval);
	WM8978_Write_Reg(51,regval);
}

void WM8978_MIC_Gain(u8 gain)
{
	gain&=0X3F;
	WM8978_Write_Reg(45,gain);		
	WM8978_Write_Reg(46,gain|1<<8);
}


void WM8978_LINEIN_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);	
	regval&=~(7<<4);
 	WM8978_Write_Reg(47,regval|gain<<4);
	regval=WM8978_Read_Reg(48);	
	regval&=~(7<<4);		
 	WM8978_Write_Reg(48,regval|gain<<4);
} 


void WM8978_AUX_Gain(u8 gain)
{
	u16 regval;
	gain&=0X07;
	regval=WM8978_Read_Reg(47);
	regval&=~(7<<0);		
 	WM8978_Write_Reg(47,regval|gain<<0);
	regval=WM8978_Read_Reg(48);
	regval&=~(7<<0);	
 	WM8978_Write_Reg(48,regval|gain<<0);
}  

void WM8978_I2S_Cfg(u8 fmt,u8 len)
{
	fmt&=0X03;
	len&=0X03;
	WM8978_Write_Reg(4,(fmt<<3)|(len<<5));	
}	


void WM8978_HPvol_Set(u8 voll,u8 volr)
{
	voll&=0X3F;
	volr&=0X3F;
	if(voll==0)voll|=1<<6;
	if(volr==0)volr|=1<<6;
	WM8978_Write_Reg(52,voll);	
	WM8978_Write_Reg(53,volr|(1<<8));	
}

void WM8978_SPKvol_Set(u8 volx)
{ 
	volx&=0X3F;
	if(volx==0)volx|=1<<6;
 	WM8978_Write_Reg(54,volx);	
	WM8978_Write_Reg(55,volx|(1<<8));
}

void WM8978_3D_Set(u8 depth)
{ 
	depth&=0XF;
 	WM8978_Write_Reg(41,depth);
}

void WM8978_EQ_3D_Dir(u8 dir)
{
	u16 regval; 
	regval=WM8978_Read_Reg(0X12);
	if(dir)regval|=1<<8;
	else regval&=~(1<<8); 
 	WM8978_Write_Reg(18,regval);
}


void WM8978_EQ1_Set(u8 cfreq,u8 gain)
{ 
	u16 regval;
	cfreq&=0X3;
	if(gain>24)gain=24;
	gain=24-gain;
	regval=WM8978_Read_Reg(18);
	regval&=0X100;
	regval|=cfreq<<5;	 
	regval|=gain;		
 	WM8978_Write_Reg(18,regval);
}

void WM8978_EQ2_Set(u8 cfreq,u8 gain)
{ 
	u16 regval=0;
	cfreq&=0X3;
	if(gain>24)gain=24;
	gain=24-gain; 
	regval|=cfreq<<5;
	regval|=gain;
 	WM8978_Write_Reg(19,regval);
}

void WM8978_EQ3_Set(u8 cfreq,u8 gain)
{ 
	u16 regval=0;
	cfreq&=0X3;
	if(gain>24)gain=24;
	gain=24-gain; 
	regval|=cfreq<<5;	
	regval|=gain;
 	WM8978_Write_Reg(20,regval);
}

void WM8978_EQ4_Set(u8 cfreq,u8 gain)
{ 
	u16 regval=0;
	cfreq&=0X3;
	if(gain>24)gain=24;
	gain=24-gain; 
	regval|=cfreq<<5;
	regval|=gain;	
 	WM8978_Write_Reg(21,regval);	
}

void WM8978_EQ5_Set(u8 cfreq,u8 gain)
{ 
	u16 regval=0;
	cfreq&=0X3;
	if(gain>24)gain=24;
	gain=24-gain; 
	regval|=cfreq<<5;	
	regval|=gain;	
 	WM8978_Write_Reg(22,regval);
}
