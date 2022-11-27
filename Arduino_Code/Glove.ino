
/* The glove code used at Amplified Fencer.
 *  https://github.com/volksamt/Amplfied_Fencer
 *  Made possible with the support of V2_ Lab for Unstable Media www.v2.nl
 *  
 *  The code depends heavily on the work of Ubidefeo. Find the original version at
 *  https://gist.github.com/ubidefeo/e77a590bbbb24a326695bc92618def69
 *  
 *  This version lets you read the built in IMU data of the Arduini Nano IoT33. It also reads data from a QMC5883L digital compass hooked on the I2C bus  and send everything over OSC
 */




/* This Sketch is tested on 
 * - Nano 33 IoT

 
 To compile and use it requires
 
 Libraries:
 - CNMAT OSC Library (https://github.com/CNMAT/OSC) - Arduino Library Manager
 - Arduino WiFiNINA (https://github.com/arduino-libraries/WiFiNINA) - Arduino Library Manager
 - Arduino_ConnectionHandler (https://github.com/arduino-libraries/Arduino_ConnectionHandler) - Arduino Library Manager
 - QMC5883LCompass https://github.com/mprograms/QMC5883LCompass - Arduino Library Manager
 
 OSC Server:
 A Max/MSP patch is provided as a comment at the bottom of this Sketch
*/

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <Arduino_ConnectionHandler.h>

#include <WiFiNINA.h>
#include <WiFiUDP.h>
#include <OSCMessage.h>
#include <Arduino_LSM6DS3.h>
#include <QMC5883LCompass.h>


QMC5883LCompass compass;
WiFiUDP Udp;
WiFiConnectionHandler conMan("Belastingdienst", "42763460158"); // This is the credentials of your network. I use Belastingdienst, which means tax office in Dutch, in order to scare people.

//the Arduino board's IP (choose one which is on your network's pool and not (already) reserved). The router may not always respect this however. I'd reserve an IP on the router.
IPAddress ip(192, 168, 0, 102);
//destination IP (use the IP of the computer you have your OSC Server running). If you want to save yourself some headache, make sure to reserve an IP for this computer on your router settings.
IPAddress outIp(192,168,0,100);
const unsigned int outPort = 666;

// the MAC address is sort of arbitrary
// you can read more on MAC Physical Address online
byte mac[] = {
  0x1B, 0x5B, 0x57, 0x9D, 0x22, 0x81
};

bool wifiIsConnected = false;



void setup() {
  Serial.begin(9600);
  pinMode(9, INPUT_PULLUP);
  compass.init();
  
  // This is IMPORTANT. Always calibrate the QMC5883L. You can do so with the provided calibration.ino in the QMC5883LCompass Library. Paste those numbers here.
  compass.setCalibration(-2301, 0, -1908, 770, -1107, 1646); 



  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");//Uh oh. Busted!
    while (1);
  }


  Udp.begin(8888);
  conMan.addCallback(NetworkConnectionEvent::CONNECTED, onNetworkConnect);
  conMan.addCallback(NetworkConnectionEvent::DISCONNECTED, onNetworkDisconnect);

}


void loop() {
  unsigned long msNow = millis();
  
  /* define values for the readings. 
  The foil is a boolean for detecting if the tip of the weapon is de-pressed or not. On a foil the tip is shorted to the lame and the connection breaks when de-pressed.
  On the arduino the internal pull-up is engaged to read this easily and without the need for an extra resistor. The lame is grounded and the tip is connected to Pin 9.

  The following values annotated down below.
  */
  
  bool foil;
  float x, y, z; //Acceleremoter
  float xG, yG, zG; //Gyroscope
  int xM, yM, zM; //Magnetometer(Compass)

  //Check connection
  conMan.check();

  
  // Read compass values
  compass.read();

  // Return XYZ readings
  xM = compass.getX();
  yM = compass.getY();
  zM = compass.getZ();
  
  
  // Check for a touche
  foil = digitalRead(9);
  
  //Check IMU and return accelerometer xyz reading
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
  }

   //Check IMU and return gyroscope xyz reading, easy as pie!
   if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(xG, yG, zG);

  }

  
  //the message wants an OSC address as first argument
  if (wifiIsConnected) {
   
    Serial.println("running");
    OSCBundle bndl;    

    /*Create first OSC Message with the accelerometer. You can freely change the /glove/acc adress to whatever you prefer.
     In my case this made the most sense because I intend on expanding the project to have ankle mounted sensors which then would be named /shoe/acc etc.
    */
    bndl.add("/glove/acc").add((float)x).add((float)y).add((float)z);

    Udp.beginPacket(outIp, outPort);
    bndl.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    bndl.empty(); // empty the bundle to free room for a new one
    
    //Create the second message with the gyroscope. Just like the previous one feel free to change the adress to whatever you prefer
    bndl.add("/glove/gyro").add((int32_t)xG).add((int32_t)yG).add((int32_t)zG);
    Udp.beginPacket(outIp, outPort);
    bndl.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    bndl.empty(); // empty the bundle to free room for a new one

    //Create the another message with the magnetometer info. Ditto
    bndl.add("/glove/mag").add((int32_t)xM).add((int32_t)yM).add((int32_t)zM);
    
    Udp.beginPacket(outIp, outPort);
    bndl.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    bndl.empty(); // empty the bundle to free room for a new one

    //Last message with the foil tip. Or technically, any push button.
    bndl.add("/glove/tip").add((bool)foil);
   

    Udp.beginPacket(outIp, outPort);
    bndl.send(Udp); // send the bytes to the SLIP stream
    Udp.endPacket(); // mark the end of the OSC Packet
    bndl.empty(); // empty the bundle to free room for a new one
    delay(20);

   
  }
}

void onNetworkConnect() {
  Serial.println("Connection open");
  wifiIsConnected = true;
}
void onNetworkDisconnect() {
  Serial.println("Connection closed");
  wifiIsConnected = false;
}




//This was here so I didnt remove it. Its not elegant (Sorry but should give you some info.



/* Max/MSP compressed patch
 * Copy the following block from <pre> to </pre> included
 * If you don't know Max/MSP but can run your own OSC Server feel free to do so
 
<pre><code>
----------begin_max5_patcher----------
3018.3oc6bz1ihab9y69qvBkOjHw4adl2mppJE09kH0pdpoU8CQQm7B9104.
ajwr81Fk+68YFa.CdLXVauPZBmNyxvXOyy6uN7y2e2jGx9R75IA+gfeH3t69
46u6N2P1Atq5y2MYYzWlsHZsaZSlksbYbZwjokeWQ7WJbi+OiWWD7gnhYOE7
0qiSmuNnHKXQ1rnEOkgey6d2e5a1dOoaVlsoXQbg6IRpFck8dSRe7i4wyJJ2
QflDRruXRifqUFkbZ.kPC4ZCSBDtRw4JAfCJDgZAPjLhhJ0.mpryLjD7iUO9
j4tsY1C+z6DxZ6jjzsaDvN1ub+81KS6H1HM9+fOwFHiMoqhl8Y+vKsc3Uxog
ZJQ.RMkqnTpYZ.iIBMTCQinACWR3bbPtJjfvGsF7UtBEurJt7YMIwRjJeyGR
fyFLjv5EIyiy293hRSyJhJRxRceoej.TM55hWV31vSdNdwyw6XqxxSPlrcOk
SfyDFSSblVFZzJofSYHmhgpESCLDKNC3xSgz1ipVm7ecCBTKttI5axtsTdzx
3h37OFmF8PIvPFY1qhf0wKPLPvhSyh8orzhTb24to+ZTQ1jSHqwEMk0XLV4f
fRqAlgwYHymNjU9RwHRbdriD0ZfUK2sVtw83W6d6SQyN.eUC+xzSpMwcDCR3
fgaaSQ1W+GKUaE7ce3LZrtDzKSoCEMTko3MQuRRSzKnqgd8g4r69YYKxxKWM
7QPDZMkInFN.10hDJv24pciaGRx.lhJ3FBXjDMtPsnwDFapgE.dXSQAJuW8H
exo4plFjU4wq2pTnNlUakqkR6Ug65AXqCIcrWAoihZSZP5X.oIoSY7HY.GO1
Iz9Ls7+GnyNOaS57XGs.3gjiYW2YhcRarBFoVZMG56uZgdCptndqNu.B4NTj
W9AODvgVp8eDOKN448VgZgYw3XV.A0wyXbVQYsxs7ZDz0FjHIYZF0flgT..T
bAIRObKdDzOXyTWPGdaDz49Ez4sHn+FPX+dzWxyRVAByQJcl3E5Qfr52UToG
yiByMHYkbqQV+awqWG8XbmzsqcWEryna+UQVQqsR7khvPmHonihHRjSEMIqF
ZHf+EPYDzgRtPBmvr7aDYkYtHyxuAj0+cdRQbPT5KEVDcvSw41OMOvtlXDgO
EGf2UbdvmieY68tHIMdFZiqnt84C3GN+LNliAHt.jTtOvoCJKij5wOYT6Pi.
N74MvA6kqfibL00hiokPYVjEMeok2X89f+7Gp3kPiPJQHnHT.MDCHVQyDVOw
fxbDHXFEUREp1LBS5VHhmKDF4X6zbK3TziQTHz8wwM7PN2ihRTO8khQun.CE
WBVkNBbpJNgLbrpni3MQrVKdcJ3hAhSkcsB1dUdx9Qa04tRc5Zv9FiNBtAXX
xxbJxUbFkQUtXizdhNmwt8hNmQuwTp+XTQ7vIgnE13uUDiP.BjxXLmf3nGKQ
D3Jo1wIgD7u9KeHuLB2gKkTdwqTpIzlacEHEDsFsUZUGoNipmyg7Hid5ixd7
wEmKZBfIpoHATau1hhD3D5KvPmjkunRCiPYswRBdPcMG6zYw2mLO0b4Ifd7E
7wG8JLd8g0YNDA6I0o9blSxGMSjzQOezKOLh383zr7hfuBFYON.F0Sl9YCK9
rgcPxzZWZw5F8hbidqusOlGM2VBo5K+COdvhqktTgV+s5agGd7SIKV3tiOtE
B2+X8LI2U3U7zcWoauQoM1wCe6j2316i5bSXpy6BilvK+KbHn8GPT5ikJLnp
83yClvFTyZcTYnx3YZqxyrroUJTv00Lf0Rc9pJSeuARABLtQT4tA.Di.b.GS
AaSs3PnFQckhIrH3ggrZgRimRh3qXgWtKYOfLkWTDgTwUBmNRV67Xry3oXSi
owN9UBiVOaP1rHFbjQw9id8WlaekmAXrwCAy9+FuIX1pn0HwPXH6MxG5sg2D
vu6Mwuk8lXjxhmWo.Fm9llFOJ4VPssqgov+Mf5Xn9p4BECimKEDJHnDsfQbJ
sAwngdAy0RosEs9UnfWoqQCFVU6qRVBT1WyLBhVJnRshfpZjpa.M2f920b+a
YM2q27v5hjhsk1JndOO0+.Z7phgS70GohQs9Vf7JknYLNaaeauW+cfAeMboa
1OFVyCMfRZXJT+sTJ0bqRbYiPtOGVSbspfUUirDTkih4daqA5YaqA5YZqASs
LWK3tBgQFzRfIvmsQYjBlBjDshwssUOZN0vroWUJ.lvHrsGivi..ibc6DFfe
sJA1qqoU0kMppprCDgsWu7JQ36HC3kpwHMXZRdLMCeRKRl845qh+Lx3Eqytp
EgvxSEOOonUj+yQ46X+mGUDsmnjUjYAvce62lmDsvLoKBgzxdMjIKkBksR03
ulFWAIcdR+iIDohJtTxjLCQpsCx7zF9BcqUV5grbaeTZQ06OOCcjAXaOIO8X
aY06M48xZ9j+qy0PuzlKldoxwUi+43WVlMOdnU4+AzEmyUpQQ8RMVJ6Mv8rf
h4KA1.3IGrvEzyBuUJrIWsdVXyxG12PwcUgsq3tSKaCXaiO0vAkAHm5dqHgu
yY.+bo9sHOY8rnJApPwojqOT494byBtTQW3UJ5hSMY4lktG6qfHuAEb2zI6B
Ioq1T7t4wOmLKdcOMPTctBJ6WYk0HLRH4RkB8lhoDmjioKmLkh7cpyNQpnPB
tjvQOBJ4hbmDRgLjwQ26DBAAH36VVKzPFUQ0ZQIaj1dRENcjLdMBjTDub8QE
uYJdApdmFxp9Kz2dDIfK8judQx5huY6LmDk9h8sc4+nZ32+2+9+76qF78UGU
vSwohjkOk7kOtEIQax.adSNWK9yy.cZsKmMCCk25YSu.2pp08P4Rph6xU.2p
Nl007L7ZxWvkCWmMCCCRpCZwYb6YHIX1l7bjJssjRAQOFkj1I2zKu1GuzQOG
UbtVJDRI2PIZnkxkBZulp6b6B0pq55aJO024JkmZ70ltUnz.LT5KELFde6u3
fZVyj.A.0SPvpK6n94Ua5Yr.KeqL.OR9N+ceHXUGbeV6belK1d9MFbum4fWR
sraks7Z68r3V6b+fj0uc9b2g7IK2kZVGgnKwSWRjKOl8CMU1egiDdhQBn5aO
xL6WkwHUdTrGkPj7WlUTYQSo1qUHRzekDgTqljaT+11NJ0x8Da84N287gR3k
YKXvwDae8mNXFeiwvU1Xrae5pzvQ+D93vl1wOjAXc1l7YaA5pvgBfcP6730E
Io6V7eXeyfErmttLY9pLDYUshZ6whUpM7pyOKyJmgCJUbsQKkFfxjkGFROUv
+34E7id4j6J3nOXm5GbjmBZ3lPZC+zkpP.Cn1fJwIDE2ErDH3dZ.xlyrm.jr
C.j8.wYmTuVH6uFPmkS3jLBRMZ863eGdTPnQnQ7BPQrgwU8aIwzkI1S.h0ad
gVIwFCxqqI.FisgA7ocdlCAHA8BjzUciqhH0BgRwLWYPpKL315EzNLoTbTIx
wG0WITt8OnyqnBe+DNzXl8ElDcAlDWLqGGATN5Eo8G8LIUvHV5DUAcZl8ElX
cAlfAPQDC5hJOxASZeksF+k1dtq7szjdszG8TaApGB7KU2eINBqSxQuYRb1C
UwYgIN6jVuLmntqBpTH4Bi1FplTPaX9xyDaxgPGeXzwg.WDL5ZAAWsk439WI
QXvV.CRnlS3TM9BmWKSbvkB5BLdDCbmfQCxvRw3ZzXHE.y5lwT+bwdl4Pqko
S.IvdE.IJqIXHD.Bv19l1CtqOlUOSrILx5GL1Ey5aQD8akD80MOeAlbMcIxA
Q8xKOyMlSdGE.YKbCCQTLzt3lx1HC52JA81IOu+hPZ+EUwzMm7ZNy9BSjtPm
FBjWWVmgvYGnKtUMHKjt6KjetApwCMloTgnPnlPEBJPXFirEtAOyrufjpC5g
fSZLlQkgvw+3pw31rjqkbjikYygm8.F3G5aNy9BSxdK01hrHuyRs7AVpE5TT
KxgfIuKHulKTYRJiVs54370Uy1sFSVF8SkUNQO89xDZW9QWVOmjG+bx14Ctr
lNIJe1SIEwyJ1jWlA4uHK662I1rilmtIox+LD7v0zkraa1lWupJGttbhe+ub
++CvZPFEZ
-----------end_max5_patcher-----------
</code></pre>

*/
