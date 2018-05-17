/*

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/* Created by w.grinfeld 9/05/2018  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

static char * frontend_device_name = "/dev/dvb/adapter0/frontend0";

static int fd_frontend=0;

static int sendDiseqcCommand(int *data, int length) {
    struct dvb_diseqc_master_cmd command;
    struct dvb_diseqc_master_cmd *cmd = &command;

    if(ioctl(fd_frontend, FE_SET_TONE, SEC_TONE_OFF)) {
        fprintf(stderr, "sendDiseqcCommand(): set tone failed.\n");
        perror("vboxctrl ");
        return 0;
    }

    if(ioctl(fd_frontend, FE_SET_VOLTAGE, SEC_VOLTAGE_13)) {
        fprintf(stderr, "sendDiseqcCommand(): set voltage failed.\n");
        perror("vboxctrl ");
        return 0;
    }

    usleep(15000);

    for(int i = 0; i < length; i++){
        cmd -> msg[i] = (int8_t) data[i];
     }

    cmd -> msg_len = length;

    if( ioctl(fd_frontend, FE_DISEQC_SEND_MASTER_CMD, cmd)){
        fprintf(stderr, "sendDiseqcCommand(): send master command failed.\n");
        perror("vboxctrl ");
        return 0;
    }

    usleep(100000);

    return 1;
}

static void print_usage()
{
   fprintf(stderr, "\
      Usage:\n\
         vboxctrl [-g position-number | -s position-number] \n\
        \n\
           -g position-number        goto satellite number on positioner.\n\
           -s position-number        store satellite number on positioner.\n");
}

static void powerOn()
{
	int pwrOn [] = {0xe0, 0x31, 0x03};
	sendDiseqcCommand(pwrOn, 3);
}
static void powerOff()
{
	int pwrOff [] = {0xe0, 0x31, 0x02};
	sendDiseqcCommand(pwrOff, 3);
}
static void storePosition(int positionNumber){
	int seq [] = {0xe0, 0x31, 0x6a, positionNumber};

    if(! sendDiseqcCommand(seq, 4)){
		fprintf(stderr, "vboxctrl: storePosition(): could not command postioner to store position: %d", positionNumber);
    }
}
static void gotoPosition(int positionNumber){
	int seq [] = {0xe0, 0x31, 0x6b, positionNumber};

	if(! sendDiseqcCommand(seq, 4)){
		fprintf(stderr, "vboxctrl: gotoPosition(): could not command postioner to goto position: %d", positionNumber);
    }
}

int main(int argc, char **argv)
{

    if( (fd_frontend = open(frontend_device_name, O_RDWR | O_NONBLOCK)) < 0){
	    fprintf(stderr, "vboxctrl: main(): open frontend device %s failed.", frontend_device_name);
	    perror("vboxctrl ");
	    exit(1);
	}


    int opt;
    while( (opt = getopt(argc, argv, "g:s:h")) != -1 )
    {
        switch(opt)
        {
            case 's': // store satellite position
                powerOn();
                storePosition(atoi(optarg));
                powerOff();
                return 0;
            case 'g': // goto satellite position
                powerOn();
                gotoPosition(atoi(optarg));
                return 0;
        }
    }

    print_usage();
    exit(1);

}