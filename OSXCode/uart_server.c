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


#define RETURNCODE int
#define OK 0


/****************************************************************************
globals
*****************************************************************************/
unsigned int iSerialDeviceCtr = 0;
unsigned int iActiveDevice = -1;
char serialDeviceNames[256][256];
char buf[256];
unsigned char delayms;
int tty_fd;
long chksum;
unsigned int byteCtr;




/****************************************************************************
stuff to put in a utility library - START
*****************************************************************************/
int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
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

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
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
    		String_Upper(buf);
    		if(startsWith("TTY.",buf)) {
    			if(strstr(buf,"BLUE") == NULL) {
	    			strcat(serialDeviceNames[iSerialDeviceCtr], ep->d_name);
                    if(strncmp(desiredDevice, buf, strlen(desiredDevice)) == 0)
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


void lsResponse() {
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
    dp = opendir(".");
    time(&now);

    if(dp != NULL) {
    	while((ep = readdir(dp)) != NULL) {
    		strcpy(buf,ep->d_name);
    		String_Upper(buf);
    		if(*(buf) != '.') {
                sendString(buf);
	            sendByte(':');
        	}
        }
    }


    sendByte(13);

    time(&later);
    double seconds = difftime(later, now);

    printf("Checksum: %ld\n",chksum);
    printf("Bytes sent: %d\n", byteCtr);
    printf("Duration: %d\n", (int)seconds);
    printf("Bps: %d\n", byteCtr/(int)seconds);
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


int main(int argc,char** argv)
{
	char desiredDevice[256];
	RETURNCODE r;
	unsigned char c;
	struct termios tio;
	char workbuff[512];

    setbuf(stdout, NULL);
    delayms = 4;
    printf("Starting...\n");

	memset(desiredDevice,0x0,sizeof(desiredDevice));
	if(argc > 1) {
		strcpy(desiredDevice,argv[1]);
		String_Upper(desiredDevice);
	}

    printf("Getting devices...\n");
    r = getDevicesAndHandleErrors(desiredDevice);
    
    if(r != OK)
    	exit(r);

    printf("Opening...");
    strcpy(buf,"/dev/");

    strcat(buf,serialDeviceNames[iActiveDevice]);

    tty_fd = open(buf, O_RDWR | O_NOCTTY | O_NDELAY);

    if(tty_fd < 0) {
    	printf("Unable to open device: %s, rc=%d", buf, tty_fd);
        exit(-5);
    }

    set_interface_attribs(tty_fd, B57600, 0);
    set_blocking(tty_fd, 0);

    bool bRunning = true;
    while(bRunning == true) {
	    workbuff[0]=0x0;
   		int iCtr = 0;
	    printf("Waiting for command.\n");

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

        String_Upper(workbuff);
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
        else if(strncmp(workbuff, "KILL", strlen(workbuff)) == 0) {
        	printf("Received KILL command.\n");
            sendResponse("OK");
        	bRunning = false;
            printf("Goodbye cruel world.");
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
