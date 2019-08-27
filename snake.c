#include <STC15F2K60S2.h>
#define uint unsigned int
#define uchar unsigned char
uint reset[15]={0,1908,1701,1515,1433,1276,1136,1012,956,852,759,716,638,568,506};	     // ����Ƶ��
              //0   1    2    3    4    5    6    7    1  2   3   4   5   6   7  
xdata uchar music[204]={0,2, 10,1, 10,1, 10,1, 10,1, 12,1, 12,3, 0,4, 0,2, 9,1, 9,1, 10,1, 9,1.5, 0,7.5, 0,2, 8,1, 8,1, 8,1, 8,1, 8,1, 10,2, 10,1, 10,1, 11,2, 10,3,
                        0,8, 8,1, 8,1, 9,1, 8,1, 9,2, 0,2, 8,1, 8,2, 6,1, 6,1, 5,3, 0,8, 9,1, 9,1, 10,1, 11,1, 11,2, 0,2, 9,1, 8,1, 10,1, 9,0.5, 9,2, 0,6,
					    10,1, 10,1, 10,1, 10,1, 12,1, 12,3, 0,4, 10,1, 10,1, 12,1, 12,2,  10,1.5, 0,6, 8,1, 8,1, 8,1, 8,1, 8,1, 10,2, 10,1, 0,1, 10,1, 10,1, 10,2, 10,1, 
					    10,1, 11,1, 10,4, 0,6,  8,1, 8,1, 9,1, 8,1, 8,2, 6,1.5, 8,1,  8,2, 6,1, 5,3, 0,5, 9,1, 9,1, 10,1, 11,1, 11,2, 0,1, 10,1, 9,1, 8,1,10,1, 9,1  };
			                             //

sbit key1 = P3 ^ 2;
sbit key2 = P3 ^ 3;
sbit beep=P3^4;   // ������
	
//�����ʼ��ֵ
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};   //����ܶ�ѡ
uchar idf;
uint time;
uint cntKey;
uint cntMusic;
uint delayMusic;
int adc_value;
int foodTemp;
//��¼��Ϸ�Ľ�����ʼ
int isStart;
int isEdge;
int isPress; //�ж��Ƿ��˰���������������
int musicFlag = 0;
int moveFlag = 0;
//��¼snake������λ�ã���Ϣ
int x[50], oldX[50];
int y[50], oldY[50];
int len = 3;
int dir = 1;    //��������  0 1 2 3

int foodX[2];
int foodY[2];


//һЩ�õ��ĺ���
void delay_ms(uint n);    //��ʱ����  ��λΪms
void snakeInit();
void paintSnake();
void ADCInit();
void time0Init();
void primeEdge();
void makeFood();
void paintFood();
void eatFood();
void primeHeader();
void Delay100us();
void time1Init();
void play() {
		//delay_ms(180*music[++cntMusic]);               
//	TR1=0;
//	delay_ms(60*music[cntMusic]);				
  // 	cntMusic++;
	if(!musicFlag){
			TR1=1;
		TH1=(65536-reset[music[cntMusic]])/256;  //
		TL1=(65536-reset[music[cntMusic]])%256; 
		musicFlag = 1;	
	}
}


void playMove(){
	if(!moveFlag){
		TR1=1;
		TH1=(65536-reset[music[3]])/256;  //
		TL1=(65536-reset[music[3]])%256; 
		moveFlag = 1;	
	}
}

	

void Init() {
	//��ʼ��  ����ܵ����
	P0M1 = 0x00;
	P0M0 = 0xff;
	P2M1 = 0x00;
	P2M0 = 0x08;
	P3M0=0x10;							  //????
	P3M1=0x00;
	EA = 1;  //�ܵ��ж����������
  time0Init();	
  time1Init();
  ADCInit();
	snakeInit();	
	//�����õ�
	foodX[0] = 0;
	foodX[1] = 1;
	foodY[0] = 1;
	foodY[1] = 1;
	
	//��ʼ�����˿�ʼ��Ϸ��֮������ͨ���������ж�
	isStart = 1;
	isEdge = 0;
  isPress = 0;
	beep = 0;
	cntMusic = 0;
	delayMusic = 0;
}
void ADC_Handler() interrupt 5{
	//�ж�֮��flag ��Ҫ��������
	ADC_CONTR &= 0xEF;   //(1110 1111)  flag ����	
}

void start(){
	//
	makeFood();
	while(1){
		if(isStart){
		  paintSnake();
			paintFood();
		}else {
			//����ײǽ֮ǰ������
			if(isEdge){
				for(idf = 0; idf <= len; idf++){
				x[idf] = oldX[idf];
				y[idf] = oldY[idf];
			}			
			}
			
			paintSnake();
			//���ﲥ������
			play();
			
			}	
	}	
}
void time1() interrupt 3{
	beep = ~beep;
}

//********************************************�жϺ��������е����ݴ���������
/**************�����ж�ʵ�ֶ��߳�**************/
void time0() interrupt 1{
	TH0 = 0xfc;
	TL0 = 0x18;		
	
	if(moveFlag){
			delayMusic++;
			if(delayMusic > 180){
				moveFlag = 0;
				delayMusic = 0;
				TR1 = 0;
			}
	}	
	
	if(musicFlag){
			delayMusic++;
			if(delayMusic > 180*music[cntMusic]){
				musicFlag = 0;
				delayMusic = 0;
			}
	}	
	//�жϱ߽�
	if(time == 1000){  //��ֹ��˸�� һ���Ӽ��һ�ι���,�պ���һ������
			//�жϱ߽磬������ɸ�������֮ǰ��
	 primeEdge();
	//�ж��Ƿ�ײ���Լ�
  	primeHeader();
	}
	//�����ʼ��Ϸ����ʼ��������		
	if(isStart){
//ÿһ�����⣬�ߵķ���
	//���key1�����£����ҷ���������������

		if(isPress){		
			cntKey++;
			if(cntKey > 300){
				isPress = 0;
				cntKey = 0;
			}	
		}
		if(time%50 == 0 && isPress == 0){ //�����жϵĴ����� һ���˵ķ�Ӧ��0.2ms
			if(dir != 2 && key1 == 0 ){
					dir = 3;		
					isPress = 1;
			}else if(key2 ==0 && dir != 0){
				dir = 1;		
				isPress = 1;
			}
			//����һ��mmp�ĵط��� key�����͵������������
			//����û�й�ϵ��������������adc�����key3
			else { 
					adc_value = -1;
					adc_value = ((ADC_RES & 0x03) << 8) | ADC_RESL;
					adc_value = adc_value >> 7;
			//1 ����  2 ����  3�� ����
				if( adc_value == 3 && dir != 1){
						dir = 0;
					isPress = 1;
				}	else if( adc_value == 0 && dir != 3){
					dir = 2;
					isPress = 1;
				}
			//�ж���֮�����¼�ʱ��    ע����� 1 ms�ж�һ�η���
				
			//adc�Ǵ��500��ָ������һ�������500/12M����һ��С��
			//1ms������ ����û������
		    ADC_CONTR |= 0x08; 				
		}
			
		}
		
		
		
		
	//���ݵĸ��ķ��������Ȼ������һ���ʱ���ж��ˣ�
		if(time == 1000){
			uchar j;
			for(j = 0; j <=len; j++){
				oldX[j] = x[j];
				oldY[j] = y[j];
			}		
			eatFood();
			for(j = len; j > 0; j--){
								x[j] = x[j-1];
							  y[j] = y[j-1];
 			}
			if(dir == 3){		
						x[0] += 1;	   
			}else if(dir == 2){		
						x[0] -= 1;			
			}else if(dir == 1){
						y[0] += 1;		
			}else{			
						y[0] -= 1;
			}				
			
				playMove();//������ʾ��
				 time = 0;		
		}
		
	 time++;	
	
}
}
//********************************************************������
void main(){
		   Init(); 
			 start();
}
void snakeInit(){
			//��ʼ��  �ߵ���״
   //Ϊʲô���ﲻ�ܳ�ʼ���ֲ�������
		for(idf = 0; idf < 49; idf++){
					x[idf] = y[idf] = -1;		
			}
	x[0] = 3;
	x[1] = 2;
	x[2] = 1;
	x[3] = 0;
	y[0] = y[1] = y[2] = y[3]= 0;	
	len = 3;
	dir = 3;
} 
void primeHeader(){
	//����ע��߽�β���ǿ�������
	for(idf = 1; idf < len ; idf ++){
		if(x[0] == x[idf] && y[0] == y[idf]){
			isStart = 0;
			break;
		}	
	}
}
void time0Init(){		
	//��ʱ��0 �ĳ�ʼ��
  TMOD = 0x01;
	TH0 = 0xfc;
	TL0 = 0x18;
	TR0 = 1;
	
	ET0 = 1;
	time = 0;
}
void time1Init(){
	ET1 = 1;
		TR1=1;
}

void ADCInit(){
		//	ADC ��ʼ��	
	P1ASF = 0x80;
	ADC_CONTR = 0x87; 
	//1000 0111 
	//ADC_POWER(0��)  SPEED1 SPEED0 ADC_FLAG       ADC_START CHS2 CHS1 CHS0
	//ADC_FLAG ÿ��Ҫ����   START Ҫ������1
	CLK_DIV |= 0x20; //����res����λ�� resl ��
	EADC = 1;	
		
}
void primeEdge(){
	//��������˱߽磬�Բ�����Ϸ����
	for(idf = 0; idf <= len ; idf ++){
		if(x[idf] < 0 || x[idf] > 8 || y[idf] <0 || y[idf] > 2){
			isStart = 0;
			isEdge = 1;
			break;
		}	
	}
}
void makeFood(){
	//�������һ�¶�ʱ������ȡ��ʱ������ļ�ʱ��ֵ
	//�����Ϳ���ʵ�������  ^_^
	int tempflag = 0;
	while(1){		
		int tempy = TH0 % 3;
		int tempx = TL0 % 9;
		int tempdir = (TH0 >> 2) % 2;  //0 ������� 1��������
		//�����ж��ڲ����ߵ�������
	  for(idf = 0; idf < len; idf++){
			if(x[idf] - x[idf+1] > 0 ){
				if(dir == 0 && tempx == x[idf+1] && tempy == y[idf+1]){			
					break;
				}
			}else if(x[idf] - x[idf+1] < 0){
				if(dir == 0 && tempx == x[idf] && tempy == y[idf]){
					break;
				}		
			}else if(y[idf] - y[idf+1] > 0){
				 if(dir == 1 && tempx == x[idf+1] && tempy == y[idf+1] ){				
					 break;
				 }			
			}else if(y[idf] - y[idf+1] < 0){
				 if(dir == 1 && tempx == x[idf] && tempy == y[idf] ){			
					 break;
				 }			
			}
			if(idf = len - 1){
				if(!( dir == 0 && tempx == 8) && !(dir == 1&& tempy == 2)){
			     tempflag = 1;				
				}
			}	
		}	
		if(tempflag == 1){
			foodX[0] = tempx;	
			foodX[1] = dir == 0? tempx : tempx + 1;
			foodY[0] = tempy;
			foodY[1] = dir == 1? tempy : tempy + 1;
		   break;
		}	
	}
}
void eatFood(){
	uchar j;
	   //�������ҳ�
			if(x[0] == foodX[0] && y[0] == foodY[0]){
	     	if(dir == 3 && foodX[1] - foodX[0] == 1){
					len += 1;
					for(j = len; j > 0; j--){
							x[j] = x[j-1];
							 y[j] = y[j-1];
					}
					x[0] += 1;
					makeFood();
					}		
			}
			if(x[0] == foodX[1] && y[0] == foodY[0]){  //���������
				if(dir == 2 && foodX[1] - foodX[0] == 1){
					len += 1;
					for(j = len; j > 0; j--){
							x[j] = x[j-1];
							 y[j] = y[j-1];
					}
					x[0] -= 1;
					makeFood();
					}			
			}
	
}

void paintFood(){
	P0 = 0;
	foodTemp = 0;
	if(foodX[0] - foodX[1] == 0 && foodX[0] != 0){
		foodTemp += foodY == 0 ? (1<<5) : (1<<4);
		P2 = arrDigitSelect[foodX[0]-1];	
	}else{	
		  if(foodX[0] - foodX[1] == 0 && foodX[0] == 0){
				foodTemp += foodY == 0 ? (1<<5) : (1<<4);	
			}else {
							if(foodY[0] == 0) foodTemp += 1;
							else if(foodY[0] == 1) foodTemp += (1<<6);
							else if(foodY[0] == 2) foodTemp += (1<<3);	 //ʹ��else�����жϴ���
			}					
	  	P2 = arrDigitSelect[foodX[0]];		
	}
	P0 = foodTemp;
	Delay100us();
	//delay_ms(1);
}
void paintSnake() {
		uchar temp,i,j;
		for(i = 0;i < 8; i++) {
			P0 = 0;
			P2 = arrDigitSelect[i];	
			temp = 0;
			for(j = 0; j <= len; j++) {
				if(x[j] ==i && y[j] == 0) {
					if(j - 1 >= 0 && x[j-1] == i+1 && y[j-1] == 0) {
						temp += 1;
					} else if(j + 1 <= len && x[j+1] == i+1 && y [j+1] == 0) {
						temp += 1;
					}
				}
				if(i == 7 &&x[j] ==i + 1 && y[j] == 0) {
					if(j - 1 >= 0 && x[j-1] == i+1 && y[j-1] == 1) {
						temp += (1<<1);
					} else if(j + 1 <= len && x[j+1] == i+1 && y [j+1] == 1) {
						temp += (1<<1);
					}
				}
				if(i == 7 && x[j] ==i + 1 && y[j] == 1) {
					if(j - 1 >= 0 && x[j-1] == i+1 && y[j-1] == 2) {
						temp += (1<<2);
					} else if(j + 1 <= len && x[j+1] == i+1 && y [j+1] == 2) {
						temp += (1<<2);
					}
				}
				if(x[j] ==i + 1 && y[j] == 2) {
					if(j - 1 >= 0 && x[j-1] == i && y[j-1] == 2) {
						temp += (1<<3);
					} else if(j + 1 <= len && x[j+1] == i && y [j+1] == 2) {
						temp += (1<<3);
					}
				}
				if(x[j] ==i  && y[j] == 2) {
					if(j - 1 >= 0 && x[j-1] == i  && y[j-1] == 1) {
						temp += (1<<4);
					} else if(j + 1 <= len && x[j+1] == i && y [j+1] == 1) {
						temp += (1<<4);
					}
				}
				if(x[j] ==i  && y[j] == 1) {
					if(j - 1 >= 0 && x[j-1] == i && y[j-1] == 0) {
						temp += (1<<5);
					} else if(j + 1 <= len && x[j+1] == i && y [j+1] == 0) {
						temp += (1<<5);
					}
				}
				if(x[j] ==i  && y[j] == 1) {
					if(j - 1 >= 0 && x[j-1] == i+1 && y[j-1] == 1) {
						temp += (1<<6);
					} else if(j + 1 <= len && x[j+1] == i+1 && y [j+1] == 1) {
						temp += (1<<6);
					}
				}
			}	
			
			P0 = temp;
			//ÿһλ�������ʾ1ms��
			//delay_ms(1);
			Delay100us();
		}
}
//��ʱ����  ��λΪms
void delay_ms( uint n ) {
	while( n ) {
		uchar i, j;
		i = 11;
		j = 190;
		do {
			while ( --j );
		} while ( --i );
		n--;
	}
}
void Delay100us()		//@11.0592MHz
{
	unsigned char i, j;

	i = 2;
	j = 15;
	do
	{
		while (--j);
	} while (--i);
}