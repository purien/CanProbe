/* can.ino */
/* Copyright (C) 2018 Pascal Urien (pascal.urien@gmail.com)
 * All rights reserved.
 *
 * CanProbe software is free for non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution.
 * 
 * Copyright remains Pascal Urien's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Pascal Urien should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes CanTool software written by
 *     Pascal Urien (pascal.urien@gmail.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY PASCAL URIEN ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */



#include <mcp_can.h>
// ===========================
// MCP_CAN Library for Arduino
// ===========================
// This library is compatible with any shield or board 
// that uses the MCP2515 or MCP25625 CAN protocol controller

#define CAN0_INT 2    // Set INT to pin 2 (NOT USED)
MCP_CAN CAN0(10);     // Set CS to pin 10


char sbuf[256]; // Serail Buffer

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128]   ;     // Array to store serial string

#define MAXFILTER 64
 long unsigned int Filter_Id[MAXFILTER];
 unsigned char Filter_Mask[MAXFILTER];
 int Filter_Nb=0;
 int mode=0; 
 bool MyFirst=true;


unsigned long iId;
int iLen;
unsigned char iData[8];
unsigned char iMask;
bool iCheck;



int readBuffer(char* buf, int count, char delim)
{
  static int pos = 0;
  char c;
  byte nb;
  bool found=false;

  while (pos < (count-1) )
  { 
      if (Serial.available() <= 0)
      return 0;
      
      nb =  Serial.readBytes(&c,1) ;

      if (nb == 0)   
      return 0;

      buf[pos++] = c ;

      if (c == delim )
      { found = true;
        break;
      }
  }

  buf[pos] = '\0';
  nb = pos ;
  pos=0;
  if (found) return nb;
  
  return 0;
}


bool sendcan(long unsigned int txId, unsigned char len, unsigned char * txBuf)
{  byte sndStat=0;
   int i, tlen;
  
  sprintf(msgString, "%.3lX %1d", txId, len);
  tlen = strlen(msgString);
  for(i = 0; i<(int)len; i++)
  sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & txBuf[i]));
  Serial.println(msgString);

  sndStat = CAN0.sendMsgBuf(txId, 0, len, txBuf);
   
    if(sndStat == CAN_OK)
    {
    Serial.println("Message Sent Successfully!");
    } 
    else 
    {
    Serial.println("Error Sending Message...");
    return false ;
    }
  
  return true ;
  
}

bool recvcan(long unsigned int aId,unsigned long atimeout )
{  int i,tlen;
   byte sndStat;
   unsigned long t1=0;
   unsigned long t2=0;
  
  t1= millis();
  
  while (1)
  { t2= millis();

    if ((t2-t1) > atimeout)  
    { Serial.println("Rx Timeout");
      return false;
    }
   
  if( !digitalRead(CAN0_INT) )                        // If CAN0_INT pin is low, read receive buffer
  { 
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000) ;    // Determine if ID is standard (11 bits) or extended (29 bits)
    // sprintf(msgString, "%.8lX %1d ", (rxId & 0x1FFFFFFF), len);
    else if (rxId == aId)
    sprintf(msgString, "%.3lX %1d", rxId, len);
  
    if((rxId & 0x40000000) == 0x40000000);
    else if (rxId == aId)
    { 
      tlen = strlen(msgString);
      for(byte i = 0; i<len; i++)
      sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
      Serial.println(msgString);
      break;
    }
 }
 }

 return true;
  
}



bool recviso(long unsigned int aId,unsigned char SID, unsigned long atimeout )
{  int i,tlen;
   byte sndStat;
   unsigned long t1=0;
   unsigned long t2=0;
  
  t1= millis();
  
  while (1)
  { t2= millis();

    if ((t2-t1) > atimeout)  
    { Serial.println("Rx Timeout");
      return false;
    }
   
  if( !digitalRead(CAN0_INT) )                        // If CAN0_INT pin is low, read receive buffer
  { 
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000) ;    // Determine if ID is standard (11 bits) or extended (29 bits)
    // sprintf(msgString, "%.8lX %1d ", (rxId & 0x1FFFFFFF), len);
    else if (rxId == aId)
    sprintf(msgString, "%.3lX %1d", rxId, len);
  
    if((rxId & 0x40000000) == 0x40000000);
    else if (rxId == aId)
    { 
      if (len >= 2)
      {
        if ( (rxBuf[0] >= 2) && (rxBuf[0] <= (len-1)) )
        { 
          if ( (rxBuf[1] == (unsigned char)(SID+(unsigned char)0x40)) ||  ( (rxBuf[1] == (unsigned char)0x7F) && (rxBuf[2] == (unsigned char)(SID+(unsigned char)0x40))) )
          {
          tlen = strlen(msgString);
          for(byte i = 0; i<len; i++)
          sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
          Serial.println(msgString);
          break;  
          }
          
        }
      }
   }
 }
 }

 return true;
  
}


bool sendiso(long unsigned int dId,long unsigned int sId, int len,  char *data)
{ unsigned char SID;

  SID= data[1];
  
    if (!sendcan(dId,len,data))
    return false;

    if (!recviso(sId,SID, 1000L))
    return false;

    if (rxBuf[1] != SID)
    return false;

    Serial.println("Success");
    
return true;
       
}


bool auth(long unsigned int aId, char *secret)
{
   byte data1[8] =  {0x02,0x27,0x01,0x00,0x00,0x00,0x00,0x00 };
   byte data2[8] =  {0x06,0x27,0x02,0x00,0x00,0x00,0x00,0x00 };
   int i;

    if (!sendcan(aId-8L,8,data1))
    return false;

    if (!recvcan(aId,1000L))
    return false;

    if  ( (rxBuf[0] != (char)0x06 ) || (rxBuf[1] != (char)0x67 ) || (rxBuf[2] != (char)0x01 ) )
    return false;

    for(i=3;i<=6;i++)
    data2[i]= 0xFF & (rxBuf[i] ^ secret[i-3]);

    if (!sendcan(aId-8L,8,data2))
    return false;

    if (!recvcan(aId,1000L))
    return false;

    if (rxBuf[1] != (char)0x67 )
    return false;

return true;
       
}



void setup()
{ int i;
  
   Serial.begin(115200);
   
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  
   if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
   Serial.println("MCP2515 ready...");
   else
   Serial.println("Error Initializing MCP2515...");
 
   CAN0.setMode(MCP_NORMAL);   // Set operation mode to normal so the MCP2515 sends acks to received data.
   pinMode(CAN0_INT, INPUT);   // Configuring pin for /INT input

  for(i=0;i<MAXFILTER;i++)
  Filter_Mask[i]= 0xFF;

  Serial.println("Scanning...");
  mode=1;
  MyFirst=true;
  Serial.print(">");
  
    
}

// CanProbe Mode
// iddle =0 
// scan = 1
// send=  4  send CanId Len Data Mask ComputeCrc 
// diff=  5
// Main commands
// filter CanId1...CanIdn
// mask   mask1...maskn
// can CanId Len Data
// iso CanIdReq CanIdResp Len Data  (Idle mode only)



void loop()
{ int nb,i,k;
  char *token=NULL, *opt=NULL;
  unsigned long MyId=0,MyResp=0L ;
  char MySecret[4];
  int MyLen=0;
  unsigned char MyData[8];
 

  nb= readBuffer(sbuf,sizeof(sbuf)-1,(char)'\n') ;
  if (nb != 0)
  { 
    token = strtok(sbuf, " \r\n");
    if (token == NULL) 
    { Serial.println();
      Serial.print(">"); 
      mode=0;
      return;
    }

    opt = strtok(NULL, " \r\n")  ;

    if (strcmp(token,"off")==0)
    { Serial.println();
      Serial.print(">"); mode=0;
    }

    else if (strcmp(token,"scan")==0)
    { Serial.println("Scanning...");
      MyFirst=true;
      mode=1;
    }

    else if (strcmp(token,"diff")==0)
    { Serial.println("Diff Scanning...");
      mode=5;
      MyFirst=true;
    }

    else if (strcmp(token,"filter")==0)
    { i=0;

      if (strcmp(opt,"off") == 0)
      Filter_Nb=0;

      else
      {
      while ( (opt != NULL) && (i<MAXFILTER) )
      { 
        if (strlen(opt) & 0x1)
        { strcpy(msgString,"0");
          strcat(msgString,opt);
        }
        else
        strcpy(msgString,opt);
     
        nb=  Ascii2bin(msgString);
        if (nb <= 0)
        return ;
        Filter_Id[i]=0 ;
        for (k=0;k<nb;k++)
        *(((char*)&Filter_Id[i])+k )= msgString[nb-1-k];
        i++;
        Filter_Nb = i; 
        opt = strtok(NULL, " \r\n")  ;
        }
      }

      sprintf(msgString,"%d Filters: ", Filter_Nb);
      for(i=0;i<Filter_Nb;i++)
      sprintf(msgString+strlen(msgString),"%.3lX ", Filter_Id[i]);
      Serial.println(msgString);
      Serial.print(">");
         
     
    }

    else if (strcmp(token,"mask")==0)
    { i=0;

      if (strcmp(opt,"off") == 0)
      for(k=0;k<MAXFILTER;k++) Filter_Mask[k]= 0xFF ;

      else
      {
      while ( (opt != NULL) && (i<MAXFILTER) )
      { 
        if (strlen(opt) & 0x1)
        { strcpy(msgString,"0");
          strcat(msgString,opt);
        }
        else
        strcpy(msgString,opt);
     
        nb=  Ascii2bin(msgString);
        if (nb <= 0)
        return ;
        Filter_Mask[i]=   msgString[0];
        opt = strtok(NULL, " \r\n")  ;
       }
      }

      sprintf(msgString,"%d Masks: ", Filter_Nb);
      for(i=0;i<Filter_Nb;i++)
      sprintf(msgString+strlen(msgString),"%.2X ", 0xFF & Filter_Mask[i]);
      Serial.println(msgString);
      Serial.print(">");
   }
    

   else if (strcmp(token,"send")==0)
    { 
      
      if (opt == NULL)  return;
     
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
     
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
      iId=0L;
      nb=2;
      for (k=0;k<nb;k++)
      *(((char*)&iId)+k )= msgString[nb-1-k];

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      iLen = atoi(opt);
      if ((iLen<0) || (iLen>8))
      return;

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
     
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
   
      for(k=0;k<iLen;k++)
      iData[k] = msgString[k];

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;

      iMask= msgString[0];

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;

      iCheck= atoi(opt);

      sprintf(msgString,"Send: Id=%.3lX Len=%d Data=", iId,iLen);
      for(i=0;i<iLen;i++)
      sprintf(msgString+strlen(msgString),"%.2X", 0xFF & iData[i]);
      sprintf(msgString+strlen(msgString)," Mask=%.2X",0xFF & iMask);
      sprintf(msgString+strlen(msgString)," Check=%d ...",iCheck);
      Serial.println(msgString);

      mode=4;
      MyFirst=true;      
    }

   else if (strcmp(token,"can")==0)
    { 
      if (opt == NULL) return;
     
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)  return;
      MyId=0L;
      nb=2;
      for (k=0;k<nb;k++)
      *(((char*)&MyId)+k )= msgString[nb-1-k];

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      MyLen = atoi(opt);
      if ((MyLen<0) || (MyLen>8))
      return;

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
     
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
   
      for(k=0;k<MyLen;k++)
      MyData[k] = msgString[k];

      sprintf(msgString,"Can: Id=%.3lX Len=%d Data=", MyId,MyLen);
      for(i=0;i<MyLen;i++)
      sprintf(msgString+strlen(msgString),"%.2X", 0xFF & MyData[i]);
      Serial.println(msgString);
      
      if (sendcan(MyId, (unsigned char)MyLen,MyData));
      else ;
      
      Serial.println(">");
    }

   else if (strcmp(token,"iso")==0)
    { 
       if (mode !=0)
       { Serial.println("not allowed in working mode, enter to stop");
         return;
       }
      
      if (opt == NULL) return;
     
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)  return;
      MyId=0L;
      nb=2;
      for (k=0;k<nb;k++)
      *(((char*)&MyId)+k )= msgString[nb-1-k];

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL) return;
      
            
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)  return;
      MyResp=0L;
      nb=2;
      for (k=0;k<nb;k++)
      *(((char*)&MyResp)+k )= msgString[nb-1-k];
      
      

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      MyLen = atoi(opt);
      if ((MyLen<0) || (MyLen>8))
      return;

      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)  return;
      
      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
     
      else
      strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
   
      for(k=0;k<MyLen;k++)
      MyData[k] = msgString[k];

      sprintf(msgString,"ISOTP: Req=%.3lX Resp=%.3lX Len=%d Data=", MyId,MyResp,MyLen);
      for(i=0;i<MyLen;i++)
      sprintf(msgString+strlen(msgString),"%.2X", 0xFF & MyData[i]);
      Serial.println(msgString);
      
      sendiso(MyId,MyResp,MyLen,MyData);
      
      Serial.println(">");
    }


    

    else if (strcmp(token,"auth")==0)
    {  
       if (mode !=0)
       { Serial.println("not allowed in working mode, enter to stop");
         return;
       }
      
      if (opt == NULL)
      return;
     
     if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
     
     else
     strcpy(msgString,opt);
      
      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
      MyId=0L;
      for (k=0;k<nb;k++)
      *(((char*)&MyId)+k )= msgString[nb-1-k];

      for(k=0;k<4;k++) 
      MySecret[k]=0;
      opt = strtok(NULL, " \r\n")  ;
      if (opt == NULL)
      return;

      if (strlen(opt) & 0x1)
      { strcpy(msgString,"0");
        strcat(msgString,opt);
      }
      else
      strcpy(msgString,opt);

      nb=  Ascii2bin(msgString);
      if (nb <= 0)
      return;
      for(i=0;i<4;i++)
      MySecret[i]= msgString[i];

      sprintf(msgString,"%.3lX ", MyId);
      for(i=0;i<4;i++)
      sprintf(msgString+strlen(msgString),"%.2X ", 0xFF & MySecret[i]);
      Serial.println(msgString);
      
      if (auth(MyId, MySecret))
      Serial.println("Success");
      else
      Serial.println("Fail");
      Serial.print(">");
    }


          
   return; 
  }

  switch(mode)
  { case 1: 
     scan(&MyFirst);
     break;
     
     case 4: 
     Sinject(&MyFirst);
     break;

     case 5: 
     diff(&MyFirst);
     break;
 }
 
}




void scan(bool *first)
{ static unsigned long t1=0;
  unsigned long t2=0;
  bool test=false,match=false;//true;
  int tlen=0,i;

 if (*first)
 { *first= false;
   t1 = millis();  
 }
 
  if( (!digitalRead(CAN0_INT)) || test) // If CAN0_INT pin is low, read receive buffer
  { 
    if (test)
    { rxId= 0x1C3L;
      len=0;
      memset(rxBuf,0,8);
      len=8;
      delay(50);
    }
    
    t2= millis();

    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
    sprintf(msgString, "%.8lX %1d ", (rxId & 0x1FFFFFFF), len);
    else
    sprintf(msgString, "%.3lX %1d", rxId, len);
  

    if( (rxId & 0x40000000) == 0x40000000 ) 
    { // Determine if message is a remote request frame.
      if (Filter_Nb == 0) 
      {
      strcat(msgString, " REMOTE REQUEST FRAME");
      Serial.println(msgString);
      }
      t1=t2;
    } 
    else
      { 
      if (Filter_Nb > 0)
      { match=false;
        for(i=0;i<Filter_Nb;i++)
        { if (rxId == Filter_Id[i]) { match=true; i=Filter_Nb; }}
      }
      else
      match=true;
      
      if (match)
      {
      tlen = strlen(msgString);
      for(byte i = 0; i<len; i++)
      sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
     
      sprintf(msgString+tlen+3*len," %4lu %lu",t2-t1,t2);  
      Serial.println(msgString);
      t1=t2; 
      }

      else
      t1=t2;
      
    }
  }
}

void diff(bool *first)
{ static unsigned long t1=0,pt[MAXFILTER];
  unsigned long t2=0;
  bool test=false;
  bool match=false;
  int tlen=0,i,k,mask;
  static unsigned char pdata[MAXFILTER][8],plen[MAXFILTER];
  static bool pmem[MAXFILTER],ffind;
  static int ct=0;
  
 if (*first)
 { *first= false;
   t1 = millis();
   for (i=0;i<Filter_Nb;i++)
   pmem[i]= false;  
 }
 
  if( (!digitalRead(CAN0_INT)) || test) // If CAN0_INT pin is low, read receive buffer
  { 
    if (test)
    { rxId= 0x1C3L;
      len=0;
      memset(rxBuf,0,8);
      len=8;
      ct++;
      if (ct >  20)
      { ct=0;
        rxBuf[7]= 1;
      }
      
      delay(50);
    }
    
    t2= millis();

    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
    sprintf(msgString, "%.8lX %1d ", (rxId & 0x1FFFFFFF), len);
    else
    sprintf(msgString, "%.3lX %1d", rxId, len);
  

    if( (rxId & 0x40000000) == 0x40000000 ) 
    { // Determine if message is a remote request frame.
      if (Filter_Nb == 0) 
      {
      strcat(msgString, " REMOTE REQUEST FRAME");
      Serial.println(msgString);
      }
      t1=t2;
    } 
    else
      { 
      if (Filter_Nb > 0)
      { match=false;
        for(i=0;i<Filter_Nb;i++)
        { if (rxId == Filter_Id[i]) { match=true; break; }}
      }
      else
      match=true;
      
      if (match)
      {

      if (!pmem[i])
      {  
      for(k=0; k<len; k++)
      pdata[i][k]= rxBuf[k] ;
      plen[i] = len         ;
      pt[i]= t2;
      pmem[i]=true;
      
      tlen = strlen(msgString);
      for(byte i = 0; i<len; i++)
      sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
      sprintf(msgString+tlen+3*len," %4lu %lu",t2-pt[i],t2);  
      Serial.println(msgString);
      }

      else
      {
      ffind=false;
      mask = 0xFF & Filter_Mask[i];
      for(k=0;k<len;k++)
      { if ( (mask & 0x1) && (pdata[i][k] != rxBuf[k]) )
        {ffind = true; break;}
        mask = mask>>1;  
      }

      for(k=0; k<len; k++)
      pdata[i][k]= rxBuf[k] ;
      plen[i] = len         ;
 
      if (ffind)
      {
      tlen = strlen(msgString);
      for(byte i = 0; i<len; i++)
      sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
      sprintf(msgString+tlen+3*len," %4lu %lu",t2-pt[i],t2);  
      Serial.println(msgString);
      pt[i]= t2;
      t1=t2; 
      }

      
      }
      }
      

      else
      t1=t2;
      
    }
  }
}


void Sinject(bool *first)
{ bool f_ext=false,f_req=false,f_tx=false;
  int S,i,mask,tlen;

 if(!digitalRead(CAN0_INT))                  // If CAN0_INT pin is low, read receive buffer
 {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);     // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)  f_ext= true ; // ID is extended (29 bits)
    else                                   f_ext= false; // ID is standard (11 bits)
     
    if((rxId & 0x40000000) == 0x40000000)  f_req=true ;
    else                                   f_req=false;

    f_tx=false;
   
    if (!f_req)
    { 
      if ( (rxId == iId) && (len== iLen) )
      { 
       mask = 0xFF & (int)iMask;
       for (i=0;i<iLen;i++)
       { if (mask & 0x1) 
           rxBuf[i] = iData[i] ;
         mask = 0xFF & (mask >> 1);
       }
       f_tx=true;
      }
      
    }
      
    if (f_tx)
    {
    if (iCheck)
    {  S =  (rxId >> 8) & 0xFF ;
       S += (rxId & 0xFF);
       S += (int)len;
       for (i=0;i<(int)(len-1);i++)
       S += (0xFF & rxBuf[i]) ;
       rxBuf[i] = 0xFF & S; 
    }

    sprintf(msgString, "%.3lX %1d", rxId, len);
    tlen = strlen(msgString);
    for(byte i = 0; i<len; i++)
    sprintf(msgString+tlen+3*i, " %.2X", (int)(0xff & rxBuf[i]));
    sprintf(msgString+tlen+3*len," %4lu",millis());  
    Serial.println(msgString);
    
    byte sndStat = CAN0.sendMsgBuf(rxId,0,len,rxBuf);
    
    
    }
 
   
   }
   

   
}


int isDigit(char c)
{ if (((int)c >= (int)'0') && ((int)c<= (int)'9')) return(1);
  if (((int)c >= (int)'A') && ((int)c<= (int)'F')) return(1);
  if (((int)c >= (int)'a') && ((int)c<= (int)'f')) return(1);
  return(0);
}


int Ascii2bin(char *Data_In)
{   int deb=-1,fin=-1,i,j=0,nc,iCt=0,len;
    long decimal_value=0;
    char c; 
    char *data_in = NULL;
    char *data_out= NULL;

    data_out= data_in = Data_In;
    
    len =(int) strlen(Data_In);

  for(i=0;i<len;i++)
  {   if      ( (deb == -1) && (isDigit(data_in[i])) )             {iCt=1;deb=i;}
      else if ( (deb != -1) && (iCt==1) && (isDigit(data_in[i])) ) {iCt=2;fin=i;}

    if (iCt == 2)
    { c= data_in[fin+1];
      data_in[deb+1]= data_in[fin];
      data_in[deb+2]= 0;
     
      decimal_value = strtol(&data_in[deb], NULL, 16);
      data_in[fin+1]=c;

      c= (char)(0xFF & decimal_value);
      
      data_out[j++]= c ;
      deb=fin=-1;iCt=0 ;
     }
    }

 return(j);
}



