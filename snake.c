#include <STC15F2K60S2.h>
#define uint unsigned int
#define uchar unsigned char
uint reset[15]={0,1908,1701,1515,1433,1276,1136,1012,956,852,759,716,638,568,506};	     // 设置频率
              //0   1    2    3    4    5    6    7    1  2   3   4   5   6   7  
xdata uchar music[204]={0,2, 10,1, 10,1, 10,1, 10,1, 12,1, 12,3, 0,4, 0,2, 9,1, 9,1, 10,1, 9,1.5, 0,7.5, 0,2, 8,1, 8,1, 8,1, 8,1, 8,1, 10,2, 10,1, 10,1, 11,2, 10,3,
                        0,8, 8,1, 8,1, 9,1, 8,1, 9,2, 0,2, 8,1, 8,2, 6,1, 6,1, 5,3, 0,8, 9,1, 9,1, 10,1, 11,1, 11,2, 0,2, 9,1, 8,1, 10,1, 9,0.5, 9,2, 0,6,
					    10,1, 10,1, 10,1, 10,1, 12,1, 12,3, 0,4, 10,1, 10,1, 12,1, 12,2,  10,1.5, 0,6, 8,1, 8,1, 8,1, 8,1, 8,1, 10,2, 10,1, 0,1, 10,1, 10,1, 10,2, 10,1, 
					    10,1, 11,1, 10,4, 0,6,  8,1, 8,1, 9,1, 8,1, 8,2, 6,1.5, 8,1,  8,2, 6,1, 5,3, 0,5, 9,1, 9,1, 10,1, 11,1, 11,2, 0,1, 10,1, 9,1, 8,1,10,1, 9,1  };
			                             //

sbit key1 = P3 ^ 2;
sbit key2 = P3 ^ 3;
sbit beep=P3^4;   // 蜂鸣器
	
//这里初始化值
uchar arrDigitSelect[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};   //数码管段选
uchar idf;
uint time;
uint cntKey;
uint cntMusic;
uint delayMusic;
int adc_value;
int foodTemp;
//记录游戏的结束开始
int isStart;
int isEdge;
int isPress; //判断是否按了按键，处理消抖动
int musicFlag = 0;
int moveFlag = 0;
//记录snake的坐标位置，信息
int x[50], oldX[50];
int y[50], oldY[50];
int len = 3;
int dir = 1;    //上下左右  0 1 2 3

int foodX[2];
int foodY[2];


//一些用到的函数
void delay_ms(uint n);    //延时函数  单位为ms
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
	//初始化  数码管的输出
	P0M1 = 0x00;
	P0M0 = 0xff;
	P2M1 = 0x00;
	P2M0 = 0x08;
	P3M0=0x10;							  //????
	P3M1=0x00;
	EA = 1;  //总的中断器在这里打开
  time0Init();	
  time1Init();
  ADCInit();
	snakeInit();	
	//测试用的
	foodX[0] = 0;
	foodX[1] = 1;
	foodY[0] = 1;
	foodY[1] = 1;
	
	//初始化完了开始游戏，之后设置通过距离来判断
	isStart = 1;
	isEdge = 0;
  isPress = 0;
	beep = 0;
	cntMusic = 0;
	delayMusic = 0;
}
void ADC_Handler() interrupt 5{
	//中断之后flag ，要重新清零
	ADC_CONTR &= 0xEF;   //(1110 1111)  flag 清零	
}

void start(){
	//
	makeFood();
	while(1){
		if(isStart){
		  paintSnake();
			paintFood();
		}else {
			//返回撞墙之前的数据
			if(isEdge){
				for(idf = 0; idf <= len; idf++){
				x[idf] = oldX[idf];
				y[idf] = oldY[idf];
			}			
			}
			
			paintSnake();
			//这里播放音乐
			play();
			
			}	
	}	
}
void time1() interrupt 3{
	beep = ~beep;
}

//********************************************中断函数，所有的数据处理都在这里
/**************利用中断实现多线程**************/
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
	//判断边界
	if(time == 1000){  //防止闪烁， 一秒钟检测一次够了,刚好有一个动画
			//判断边界，这个换成更新数据之前。
	 primeEdge();
	//判断是否撞到自己
  	primeHeader();
	}
	//如果开始游戏，开始更新数据		
	if(isStart){
//每一毫秒检测，蛇的方向
	//如果key1键按下，而且反向不是在这个方向的

		if(isPress){		
			cntKey++;
			if(cntKey > 300){
				isPress = 0;
				cntKey = 0;
			}	
		}
		if(time%50 == 0 && isPress == 0){ //减少判断的次数， 一般人的反应是0.2ms
			if(dir != 2 && key1 == 0 ){
					dir = 3;		
					isPress = 1;
			}else if(key2 ==0 && dir != 0){
				dir = 1;		
				isPress = 1;
			}
			//这是一个mmp的地方， key三键和导航键共用输出
			//不过没有关系，反正导航键的adc输出有key3
			else { 
					adc_value = -1;
					adc_value = ((ADC_RES & 0x03) << 8) | ADC_RESL;
					adc_value = adc_value >> 7;
			//1 不动  2 是下  3是 里面
				if( adc_value == 3 && dir != 1){
						dir = 0;
					isPress = 1;
				}	else if( adc_value == 0 && dir != 3){
					dir = 2;
					isPress = 1;
				}
			//判断完之后重新计时，    注意的是 1 ms判断一次方向
				
			//adc是大概500个指令周期一个输出，500/12M，是一个小于
			//1ms的数， 所以没有所以
		    ADC_CONTR |= 0x08; 				
		}
			
		}
		
		
		
		
	//数据的更改放在这里，不然更换到一半的时候被中断了？
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
			
				playMove();//播放提示音
				 time = 0;		
		}
		
	 time++;	
	
}
}
//********************************************************主函数
void main(){
		   Init(); 
			 start();
}
void snakeInit(){
			//初始化  蛇的形状
   //为什么这里不能初始化局部变量？
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
	//这里注意边界尾部是可以捧得
	for(idf = 1; idf < len ; idf ++){
		if(x[0] == x[idf] && y[0] == y[idf]){
			isStart = 0;
			break;
		}	
	}
}
void time0Init(){		
	//定时器0 的初始化
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
		//	ADC 初始化	
	P1ASF = 0x80;
	ADC_CONTR = 0x87; 
	//1000 0111 
	//ADC_POWER(0关)  SPEED1 SPEED0 ADC_FLAG       ADC_START CHS2 CHS1 CHS0
	//ADC_FLAG 每次要清零   START 要重新置1
	CLK_DIV |= 0x20; //设置res高两位， resl 低
	EADC = 1;	
		
}
void primeEdge(){
	//如果超出了边界，对不起，游戏结束
	for(idf = 0; idf <= len ; idf ++){
		if(x[idf] < 0 || x[idf] > 8 || y[idf] <0 || y[idf] > 2){
			isStart = 0;
			isEdge = 1;
			break;
		}	
	}
}
void makeFood(){
	//这里借用一下定时器，获取定时器里面的计时数值
	//这样就可以实现随机啦  ^_^
	int tempflag = 0;
	while(1){		
		int tempy = TH0 % 3;
		int tempx = TL0 % 9;
		int tempdir = (TH0 >> 2) % 2;  //0 代表横着 1代表竖着
		//这里判断在不在蛇的身体上
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
	   //从左往右吃
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
			if(x[0] == foodX[1] && y[0] == foodY[0]){  //从右往左吃
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
							else if(foodY[0] == 2) foodTemp += (1<<3);	 //使用else减少判断次数
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
			//每一位数码管显示1ms；
			//delay_ms(1);
			Delay100us();
		}
}
//延时函数  单位为ms
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