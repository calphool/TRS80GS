#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <time.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <glob.h>


#define RETURNCODE int
#define OK 0


/****************************************************************************
globals
*****************************************************************************/
unsigned int iSerialDeviceCtr = 0;
unsigned int iActiveDevice = -1;
 char serialDeviceNames[256][256];
 char buf[256];
 char workbuff[512];

unsigned char delayms;
int tty_fd;
long chksum;
unsigned int byteCtr;
    unsigned long szBuf;





/****************************************************************************
stuff to put in a utility library - START
*****************************************************************************/

char* stristr( const char* str1, const char* str2 )
{
    const char* p1 = str1 ;
    const char* p2 = str2 ;
    const char* r = *p2 == 0 ? str1 : 0 ;

    while( *p1 != 0 && *p2 != 0 ){
        if( tolower( (unsigned char)*p1 ) == tolower( (unsigned char)*p2 ) ){
            if( r == 0 ){
                r = p1 ;
            }
            p2++ ;
        }
        else{
            p2 = str2 ;
            if( r != 0 ){
                p1 = r + 1 ;
            }

            if( tolower( (unsigned char)*p1 ) == tolower( (unsigned char)*p2 ) ){
                r = p1 ;
                p2++ ;
            }
            else{
                r = 0 ;
            }
        }
        p1++ ;
    }
    return *p2 == 0 ? (char*)r : 0 ;
}

int stricmp(const char *a, const char *b, int x) {
  int ca, cb,c;
  c=0;
  do {
     ca = (unsigned char) *a++;
     cb = (unsigned char) *b++;
     ca = tolower(toupper(ca));
     cb = tolower(toupper(cb));
     c++;
   } while (c < x && ca == cb && ca != '\0');
   return ca - cb;
}

int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return stricmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void String_Upper(char string[]) 
{
	int i = 0;
 
	while (string[i] != '\0') 
	{
    	if (string[i] >= 'a' && string[i] <= 'z') {
        	string[i] = string[i] - 32;
    	}
      	i++;
	}
}
void String_Lower(char string[]) 
{
    int i = 0;
 
    while (string[i] != '\0') 
    {
        if (string[i] >= 'A' && string[i] <= 'Z') {
            string[i] = string[i] + 32;
        }
        i++;
    }
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : stricmp(pre, str, lenpre) == 0;
}

/****************************************************************************
stuff to put in a utility library - END
*****************************************************************************/



RETURNCODE getSerialDevices(char* desiredDevice) {
    DIR *dp;
    struct dirent *ep;
 
    printf("Searching for non-bluetooth tty devices...\n");
    memset(&serialDeviceNames,0x0,256*256);

    dp = opendir("/dev");
    if(dp != NULL) {
    	while((ep = readdir(dp)) != NULL) {
    		strcpy(buf,ep->d_name);
    		//String_Upper(buf);
    		if(startsWith("TTY.",buf)) {
    			if(stristr(buf,"BLUE") == NULL) {
	    			strcat(serialDeviceNames[iSerialDeviceCtr], ep->d_name);
            if(stricmp(desiredDevice, buf, strlen(desiredDevice)) == 0)
                iActiveDevice = iSerialDeviceCtr;
	    			iSerialDeviceCtr++;
	    			if(iSerialDeviceCtr > 255) {
	    				closedir(dp);
	    				perror("Too many serial devices!\n");
	    				return -2;
	    			}
    			}
    			else {
    				printf("...ignoring bluetooth device: %s ...\n",ep->d_name);
    			}
    		}
    	}
    	closedir(dp);
    }
    else {
    	perror("Couldn't open device directory /dev");
    	return -1;
    }	

    return OK;
}

void listDeviceChoices() {
	printf("Your choices: \n");
    for(int i=0;i<iSerialDeviceCtr;i++) {
    	printf("    %s\n",serialDeviceNames[i]);
    }
    printf("\n");
}

RETURNCODE getDevicesAndHandleErrors(char* desiredDevice) {
    if(getSerialDevices(desiredDevice) < OK) {
    	perror("Problem getting serial devices!  Exiting.");
    	return -1;
    }

    printf("\nFound %d valid serial device(s).\n", iSerialDeviceCtr);
    if(iSerialDeviceCtr < 1) {
    	printf("Unable to continue.  No usable devices.\n");
    	return -2;
    }

    if(iSerialDeviceCtr == 1) {
    	iActiveDevice = 0;
    	printf("Defaulting to serial device: %s\n", serialDeviceNames[iActiveDevice]);
    }
    else {
		if(strlen(desiredDevice) == 0) {
			printf("You must specify a desired device name, when there is more than one serial device.\n");
			listDeviceChoices();
			return -3;
		}

		if(iActiveDevice == -1) {
			printf("You attempted to specify a desired device, but it was not found.  Try again.\n");
			listDeviceChoices();
			return -4;
		}

		printf("Unexpected error.\n");
		return -5;
    }

    return 0;
}

void sleep_ms(int milliseconds) {
	struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


void sendByte(char b) {
    chksum += b;
    byteCtr++;
    write(tty_fd,&b,1);
    tcdrain(tty_fd);
    sleep_ms(delayms);    
}


void sendString(char* s) {
    char c;
    int x;

    x = strlen(s);
    for(int i=0;i<x;i++) {
        c=*(s+i);
        sendByte(c);
    }
}

void sendResponse(char* s) {
    sendString(s);
    sendByte(13);
}


void lsResponse(char* s) {
    DIR *dp;
    struct dirent *ep;
    char c;
    int status;
    time_t now;
    time_t later;
    struct dirent **namelist;
    int n;

    chksum = 0;
    byteCtr = 0;
    time(&now);

    char *token = strtok(s, " "); // command itself
    
    token = strtok(NULL, " "); // wildcard path sent
    if(token == NULL) {
        strcpy(buf,"*");
        token = buf;
    }

    sprintf(workbuff,"%s",token);
    String_Lower(workbuff);
  

    glob_t globbuf;
    int r1, r2;
    r1 = glob(token, 0, NULL, &globbuf);
    r2 = glob(workbuff, GLOB_APPEND, NULL, &globbuf);
    if(r1 == 0 || r2 == 0)
    {
      int i;
      printf("%zu matching file entries...", globbuf.gl_pathc);
      for(i = 0; i < globbuf.gl_pathc; i++) {
            strcpy(buf,globbuf.gl_pathv[i]);
            String_Upper(buf);
            sendString(buf);
            sendByte(':');
        }
        globfree(&globbuf);
    }

    sendByte(13);

    time(&later);
    double seconds = difftime(later, now);
    if(seconds == 0)
        seconds = 1;

    printf("  Checksum: %ld\n",chksum);
    printf("  Bytes sent: %d\n", byteCtr);
    printf("  Duration: %d\n", (int)seconds);
    printf("  Bps: %d\n", byteCtr/(int)seconds);
}



int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror ("error from tcgetattr");
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= CRTSCTS;
        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        //tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                perror ("error from tcsetattr");
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror ("error from tggetattr");
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                perror ("error setting term attributes");
}


void handleSetDelay(char* s) {
    char *token = strtok(s, " "); // command itself
    
    token = strtok(NULL, " "); // numeric value
    if(token == NULL) {
        sendResponse("NO_VALUE_SPECIFIED");
        return;
    }

    int val = atoi(token);
    

    if(val == 0) {
        sendResponse("BAD_NUMERIC_VALUE");
        return;
    }

    delayms = val;
    sendResponse("OK");
}

char* getCommand() {
    unsigned char c;
    int iCtr = 0;

    c = 0x0;
    while( c != 13) {
            if(read(tty_fd, &c, 1) > 0) {
                if(c != 13) {
                    workbuff[iCtr] = c;
                    workbuff[iCtr+1] = 0x0;
                    iCtr++;
                }
            }
            else
                sleep_ms(1);
    }

    return workbuff;
}

char* base64Encode( char* in, int iLen,  char* out) {
    size_t sz;
    typedef unsigned long UL;
    unsigned char c[4];
    UL u, len;
    const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "abcdefghijklmnopqrstuvwxyz"
                        "0123456789+/";
 
    //printf("a\n");
    FILE* fpIn = fmemopen(in, iLen, "rb");
    if(fpIn == NULL) {
        printf("  base64 encoding failure, fmemopen\n");
        return 0;
    }

    //printf("b\n");

    FILE* fpOut = open_memstream (&out, &sz);
    if(fpOut == NULL) {
        printf("  base64 encoding failure, open_memstream\n");
        return 0;
    }

    //printf("c\n");

    //printf("len: %d\n", iLen);
 
    do {
        //printf("x\n");
        c[1] = c[2] = 0;
 
        //printf("x\n");
        if (!(len = fread(&c, 1, 3, fpIn))) break;
        //printf("y, len=%zu\n",len);
        //printf("c[0]=%d, c[1]=%d, c[2]=%d, c[3]=%d\n", c[0], c[1], c[2], c[3]);

        u = (UL)c[0]<<16 | (UL)c[1]<<8 | (UL)c[2];
        //printf("z\n");

        //printf("u=%zu\n",(u>>18));
 
        fputc(alpha[u>>18], fpOut);
        //fflush(fpOut);
        //printf("size: %zu\n",sz); 

        fputc(alpha[u>>12 & 63], fpOut);
        //fflush(fpOut);
        //printf("size: %zu\n",sz); 

        fputc(len < 2 ? '=' : alpha[u>>6 & 63], fpOut);
        //fflush(fpOut);
        //printf("size: %zu\n",sz); 

        fputc(len < 3 ? '=' : alpha[u & 63], fpOut);

        //fflush(fpOut);
        //printf("size: %zu\n",sz); 
    } while (len == 3);

    fputc(0,fpOut);
    fflush(fpOut);

    //printf("d\n");
    fclose(fpIn);
    //printf("e\n");
    fclose(fpOut);
    //printf("f\n");

    //for(int i=0;i<sz;i++) {
    //    putchar(*(out+i));
    //}
    //printf("\n");
                 //printf("  base64 ptr in func: %p\n", out);
                 szBuf = sz;

    return out;
}


void handleLoadCommand(char* s) {
    FILE *fp;
    char *base64Buff;
    char tempbuff[512];
    char responseBuff[2048];
    char blocktype;
    unsigned char b;
    int len;
    unsigned long cksum;
    unsigned long bytesRead; 
    unsigned short address;
    unsigned long bytePercentage;
    double f;
    char *token = strtok(s, " "); // command itself
    
    token = strtok(NULL, " "); // file name
    if(token == NULL) {
        sendResponse("ERROR_NO_FILE_SPECIFIED");
        return;
    }

    if(EndsWith(token,".CMD") == 0) {
        sendResponse("ERROR_FILE_NOT_CMD_FORMAT");
        return;
    }

    fp = fopen(token, "rb");
    if(fp == NULL) {
        sendResponse("ERROR_UNABLE_TO_OPEN_FILE");
        return;
    }
    bytesRead = 0;

    fseek(fp, 0L, SEEK_END);
    long szFile = ftell(fp);
    rewind(fp);

    for(;;) {
        if(!fread(tempbuff,1,1,fp)) // EOF
            break;

        bytesRead++;

        blocktype = *(tempbuff);
        if(blocktype == 0x0) {
            // ignore blocktype 0x00
        }
        else
        if(blocktype == 0x01) {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             //printf("len read: %d\n",len);
             if(len<3) // compensate for special values 0,1, and 2.
                len+=256;
             //printf("adjusted len: %d\n",len);

             fread(&address,1,2,fp); // read 16-bit load-address
             bytesRead+=2;

             len=len-2;
             f = (double)bytesRead * (double)100.0 / (double)szFile; 
             printf("Reading Object block, addr 0x%x, length = %d (%d percent complete)\n",address,len, (int)f);
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;


             cksum = 0;
             for(int i=0;i<len;i++) {
                b = *(tempbuff+i);
                cksum+=b;
                //printf("  %ld ",cksum);
             }
             //printf("\n  cksum=%ld\n", cksum);

             base64Buff = base64Encode(tempbuff, len, base64Buff);

             f = f / 100;
             f = f * 64;
             bytePercentage = (int)f;
             if(bytePercentage == 0)
                 bytePercentage = 1;

             sprintf(responseBuff,"OBJ %04X %04lX %04lX %s", address, bytePercentage, cksum, base64Buff);
             //printf("  %s\n", responseBuff);
             free(base64Buff);
             int iResendCtr = 0;
             do {
                 sendResponse(responseBuff);
                 getCommand();
                 iResendCtr++;
             }
             while(stricmp(workbuff, "RESEND", strlen(workbuff)) == 0 && iResendCtr < 5);
             if(iResendCtr >= 5) {
                printf("  too many resends.  Forcing stop.\n");
                break;
             }

             if(stricmp(workbuff, "WRITE_FAILED_HIT_CLIENT_BUFFER", strlen(workbuff)) == 0) {
                printf("  WRITE_FAILED_HIT_CLIENT_BUFFER while sending object blocks, terminating GET request.");
                break;
             }

             if(stricmp(workbuff, "OK", strlen(workbuff)) != 0) {
                printf("  problem sending object block: %s, continuing loop.\n", workbuff);
             }
        }
        else if(blocktype == 0x02) {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             f = (double)bytesRead * (double)100.0 / (double)szFile; 
             printf("Reading Transfer Address, block length = %u, (%d percent complete).\n",len, (int)f);
             fread(&address,1,len,fp);
             bytesRead+=len;

             printf("   Entry point is 0x%x (%d)\n",address,address);
             sprintf(responseBuff,"ENTPT %X",address);
             sendResponse(responseBuff);
             getCommand();
             if(stricmp(workbuff, "OK", strlen(workbuff)) != 0) {
                printf("  problem sending transfer address: %s\n", workbuff);
             }
        }
        else if(blocktype == 0x03) {
             printf("Read End Of File Mark -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0) 
                len = 256; 
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x04) {
             printf("Read End of ISAM mark -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x05) {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             //printf("%X\n",len);

             if(len == 0 ) 
                len = 256;

             f = (double)bytesRead * (double)100.0 / (double)szFile; 
             printf("Reading Load Module Header, block length = %d (%d percent complete).\n",len,(int)f);
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;

             printf("    %s\n",tempbuff);
        }
        else if(blocktype == 0x06) {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;

             f = (double)bytesRead * (double)100.0 / (double)szFile; 
             printf("Reading PDS Header, block length = %d (%d percent complete).\n",len,(int)f);
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;

             //printf("    %s\n",tempbuff);
        }
        else if(blocktype == 0x07) {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;

             f = (double)bytesRead * (double)100.0 / (double)szFile; 
             printf("Reading Patch Name Header, block length = %u (%d percent complete).\n",len,(int)f);
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;

             //printf("    %s\n",tempbuff);
        }
        else if(blocktype == 0x08) {
             printf("Read ISAM directory entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;

             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x0a) {
             printf("Read End of ISAM directory entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x0c) {
             printf("Read PDS directory entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x0e) {
             printf("Read End of PDS directory entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x10) {
             printf("Read Yanked Load Block entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else if(blocktype == 0x1f) {
             printf("Read Copyright Block entry -- skipping.\n");
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             if(len == 0 ) 
                len = 256;
             memset(tempbuff, 0x0, sizeof(tempbuff));
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
        else {
             fread(tempbuff,1,1,fp);
             bytesRead++;

             len=*(tempbuff+0);
             len = len & 0x000000ff;
             f = (double)bytesRead * (double)100.0 / (double)szFile;  
             printf("Unknown block type 0x%x, length = %u (%d percent complete).\n",blocktype,len, (int)f);
             fread(tempbuff,1,len,fp);
             bytesRead+=len;
        }
    }
    
    fclose(fp);
    sendResponse("GET_DONE");
}



int main(int argc,char** argv)
{
	char desiredDevice[256];
	RETURNCODE r;
	struct termios tio;

    setbuf(stdout, NULL);
    delayms = 4;
    printf("Starting...\n");

	memset(desiredDevice,0x0,sizeof(desiredDevice));
	if(argc > 1) {
		strcpy(desiredDevice,argv[1]);
	}

    printf("Getting devices...\n");
    r = getDevicesAndHandleErrors(desiredDevice);
    
    if(r != OK)
    	exit(r);

    printf("Opening...\n");
    strcpy(buf,"/dev/");
    strcat(buf,serialDeviceNames[iActiveDevice]);

    tty_fd = open(buf, O_RDWR | O_NOCTTY | O_NDELAY);

    if(tty_fd < 0) {
    	printf("Unable to open device: %s, rc=%d\n", buf, tty_fd);
        exit(-5);
    }

    set_interface_attribs(tty_fd, B57600, 0);
    set_blocking(tty_fd, 0);

    bool bRunning = true;
    while(bRunning == true) {
	    workbuff[0]=0x0;
	    printf("Waiting for command.\n");

        
        getCommand();
        if(startsWith("LS",workbuff)) {
        	printf("Received LS command.\n");
        	lsResponse(workbuff);
        	printf("Response sent.\n\n");
        }
        else if(startsWith("SETDELAY",workbuff)) {
            printf("Transmit Delay change requested...");
            handleSetDelay(workbuff);
            printf("Delay changed to %d.\n\n",delayms);
        }
        else if(stricmp(workbuff, "KILL", strlen(workbuff)) == 0) {
        	printf("Received KILL command.\n");
            sendResponse("OK");
        	bRunning = false;
            printf("Goodbye cruel world.");
        }
        else if(startsWith("GET",workbuff)) {
            printf("Get Command requested...\n");
            printf("    %s\n",workbuff);
            handleLoadCommand(workbuff);
            printf("Get Command completed.\n\n");
        }
        else if(startsWith("OK",workbuff)) {
            printf("Received spurious OK command.  Ignoring.\n");
        } 
        else {
        	printf("Received unknown command: %s\n",workbuff);
        	sendResponse("UNKNOWN_COMMAND");
            printf("Sent UNKNOWN_COMMAND\n");
        } 
	}
    
    close(tty_fd);


    printf("Done!\n\n");
    return 0;
}
