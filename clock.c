#include <msp430.h> // MSP430 giris ve cikislari icin kutuphane.
#define SEGMENT0 BIT0 // P2.0
#define SEGMENT1 BIT1 // P2.1 // Segment Display portlari

#define SEGMENT2 BIT2 // P2.2 icin isim atandi.
#define SEGMENT3 BIT3 // P2.3
const unsigned char segment[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,
0x07,0x7F,0x6F}; //Segment te rakamlari yazdirmak icin gerekli olan dizi.

unsigned int i=0;
unsigned volatile int saniye=0, dakika=8, saat=23, count=0,a,b=0; // Gerekli degiskenker.

int sicaklik;
void adc_init(void); // Sicaklik sensorumuz icin ADC kurulum fonksiyonu.

void main( void )
{
 WDTCTL = WDTPW + WDTHOLD; // Watch Dog timeri kapattik.
 P2REN = BIT4 + BIT5; // P2 Portu icin Pullup/Pulldown u aktif ettik

 P2OUT = ~BIT4 + ~BIT5; // P2 Pullup olarak atadik.
 P2IE |= BIT4 + BIT5; // P2 portu icin kesmeler aktif edildi.

 P2IES |= BIT4 + BIT5; // P2 poru icin dusen/cikan kenar secildi.
 
 P2IFG &= ~(BIT4 + BIT5); // P2IFG bayrak temizlendi.
 
 BCSCTL1=CALBC1_1MHZ + DIVA_3;
 BCSCTL1 = CALBC1_1MHZ; // Clock ayarlandi.
 DCOCTL = CALDCO_1MHZ;
 P1DIR = 0xFF; // P1 Tum Pinler Çikis
 P2DIR = BIT0 + BIT1 + BIT2 + BIT3; // Segment Seçme Pinleri
 P1OUT = 0x00; // P1 ve P2 portlari temizlendi.
 P2OUT = 0x00;
 TA0CTL = MC_1 + TASSEL_2;
 TA0CCR0 = 5000 – 1; // TIMER0 Kesme ayari.
 TA0CCTL0 = CCIE; // 5 ms de bir kesme.
 TA1CTL = MC_1 + TASSEL_2;
 TA1CCR0 = 50000 – 1; // TIMER1 Kesme ayari
 TA1CCTL0 = CCIE; // 50 ms de bir kesme.
 adc_init(); // adc kurulum fonksiyonu cagrildi.
 
 while(1)
 {
 ADC10CTL0 |= ENC + ADC10SC;
 __bis_SR_register(CPUOFF + GIE);
 sicaklik=(ADC10MEM*33)/1023; // ADC den okunan deger sicakliga cevrilip sicaklik degiskenine atandi.


 }
 }
#pragma vector=PORT2_VECTOR //Port2 buton kesmesi.
__interrupt void Port_2(void)
{
if((P2IFG & 0x20) == 0x20) //Port 2.4 e bagli butona basildimi basilmadimi kontrolu.

 {
 b++;
 if(b==3)
 b=0;
 for(a=0;a<500; a++) // Butona bastigimizda bir den fazla basmamasi icin gecikme.

 P2IFG =0X00;
 }
 if((P2IFG & 0x10) == 0x10) //Port 2
 {
 if(b==1)                //Mod butonuna bir kere basildiysa saati 1 kere arttirip saniyeyi sifirliyor ve saat degeri 24 ise  saati sifirliyor.


 {
 saat++; 

 saniye=0;
 if(saat==24)
 saat=0;
 }
 else                    // eger mod butonuna 2 kere basildiysa ve 2. butona basildigi taktirde dakikayi arttiracaktir.
 { 

 dakika++; 

 Saniye=0;
if(dakika== 60) // dakika 60 a esitse dakikayi sifirlar.
dakika=0; 
 }
 for(a=0;a<500; a++) // Butona basildiginda birden fazla  saymamasi icin gecikme.


 
 }
}
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
 __bic_SR_register_on_exit(CPUOFF);
}
void adc_init()
{
 ADC10CTL0 &= ~ENC;
 ADC10CTL0 = ADC10SHT_3 + ADC10ON + ADC10IE;
 ADC10CTL1 = INCH_7 + ADC10SSEL_1 + ADC10DIV_5;
 ADC10AE0 = BIT7;
}
#pragma vector=TIMER1_A0_VECTOR  // Her 50 ms de kesme uretir.
                                 // Saat icin 1 snlik islemlemler.

__interrupt void TA1_A0_ISR(void)
{
 count++;
 if (count==20)                  // 1 sn lik periyot yapabilmek icin gerekli for dongusu.

 { 
 saniye++;
 count=0;
 }
 if(saniye==60)
 {
 saniye=0;
 dakika++;
 }
 if(dakika==60)
 {
 dakika=0;
 saat++;
 }
 if(saat==24)
 saat=0;
 }
 #pragma vector=TIMER0_A0_VECTOR // 5 ms de bir kesme uretir
                                // Segmentleri sirali olarak acip kapar.

__interrupt void TA0_A0_ISR(void)
{
 i++;
 if(b>0) // mod butonuna basildiysa
 {
 if(i==1)
 {
 P2OUT = 0;
 P1OUT = segment[saat/10];
 P2OUT = SEGMENT0;
 }
 else if(i==2)
 {
 P2OUT = 0;
 P1OUT = segment[saat%10];
 P2OUT = SEGMENT1;
 }
 else if(i==3)
 
 P2OUT = 0;
 P1OUT = segment[(dakika/10)%10];
 P2OUT = SEGMENT2;
 }
 else if(i==4)
 {
 P2OUT = 0;
 P1OUT = segment[(dakika%10)];
 P2OUT = SEGMENT3;
 i = 0;
 }
 }
 
  else if(b!=1) // Mod butonuna basilmadiysa. 5 snde bir sicakligi ve saati gosterir.
 { 
 if(i==1) 
 {
 P2OUT = 0;
 if(saniye%10<=5)
 {
 P1OUT = segment[sicaklik/10];
 P2OUT = SEGMENT0;
 }
 else if(5<saniye%10<=9 )
 {
 P1OUT = segment[saat/10];
 P2OUT = SEGMENT0;
 }
 }
 else if(i==2)
 {
 P2OUT = 0;
 if(saniye%10<=5)
 {
 P1OUT = segment[sicaklik%10];
 P2OUT = SEGMENT1;
 }
 else if(5<saniye%10<=9 )
 {
 P1OUT = segment[saat%10];
 P2OUT = SEGMENT1;
 }
}
else if(i==3)
 {
 P2OUT = 0;
 if(saniye%10<=5)
 {
 P1OUT = 0x63;
 P2OUT = SEGMENT2;
 }
 else if(5<saniye%10<=9 )
 {
 P1OUT = segment[(dakika/10)%10];
 P2OUT = SEGMENT2;
 }
 }
 else if(i==4){
 P2OUT = 0;
 if(saniye%10<=5)
 {
 P1OUT = 0x39;
 P2OUT = SEGMENT3;
 }
 else if(5<saniye%10<=9 )
 {
 P1OUT = segment[(dakika%10)];
 P2OUT = SEGMENT3;
 }
 i = 0;
}
 }
}
