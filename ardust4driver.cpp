/*******************************************************************************
  Copyright(c) 2012 Jasem Mutlaq. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/
#include "ardust4driver.h"
#include <indidevapi.h>

ARDUST4Driver::ARDUST4Driver(std::string device)
{
    //ctor
    guideCMD[0]=0;
    debug=false;
    this->device = device;
}

ARDUST4Driver::~ARDUST4Driver()
{
    //dtor
   // usb_close(usb_handle);
}



bool ARDUST4Driver::Connect()
{
  this->fd = open(device.c_str(), O_RDWR | O_NOCTTY);

  struct termios tty;
  struct termios tty_old;
  memset (&tty, 0, sizeof tty);

  /* Error Handling */
  if ( tcgetattr ( fd, &tty ) != 0 ) {
    std::cout << "cerr " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
  }

  /* Save old tty parameters */
  tty_old = tty;

  /* Set Baud Rate */
  cfsetospeed (&tty, (speed_t)B57600);
  cfsetispeed (&tty, (speed_t)B57600);

  /* Setting other Port Stuff */
  tty.c_cflag     &=  ~PARENB;            // Make 8n1
  tty.c_cflag     &=  ~CSTOPB;
  tty.c_cflag     &=  ~CSIZE;
  tty.c_cflag     |=  CS8;

  tty.c_cflag     &=  ~CRTSCTS;           // no flow control
  tty.c_cc[VMIN]   =  1;                  // read doesn't block
  tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
  tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

  /* Make raw */
  cfmakeraw(&tty);

  /* Flush Port, then applies attributes */
  tcflush( fd, TCIFLUSH );
  if ( tcsetattr ( fd, TCSANOW, &tty ) != 0) {
    std::cerr << "Error " << errno << " from tcsetattr" << std::endl;
  }

  write_blocking("CONNECT#");
  return this->fd != -1;

}

bool ARDUST4Driver::Disconnect()
{
  write_blocking("DISCONNECT#");

    close(this->fd);
    return true;
}

bool ARDUST4Driver::startPulse(int direction)
{
    int rc=0;
    std::string command;
    switch (direction)
    {
        case ARDUST4_NORTH:
        if (debug) IDLog("Start North\n");
        command = "RA+#";
        break;

        case ARDUST4_WEST:
        if (debug) IDLog("Start West\n");
        command = "DEC+#";
        break;

        case ARDUST4_SOUTH:
        if (debug) IDLog("Start South\n");
        command = "RA-#";
        break;

        case ARDUST4_EAST:
        if (debug) IDLog("Start East\n");
        command = "DEC-#";
        break;
    }

   IDLog("start command value is %s\n", command.c_str());

   rc = write_blocking(command);

  IDLog("startPulse returns %d\n",rc);
    if(rc==1)
        return true;

    return false;
}

bool ARDUST4Driver::stopPulse(int direction)
{
   int rc=0;
   std::string command;
    switch (direction)
    {
        case ARDUST4_NORTH:
        if (debug) IDLog("Stop North\n");
        command = "RA0#";
        break;

        case ARDUST4_WEST:
        if (debug) IDLog("Stop West\n");
        command = "DEC0#";
        break;

        case ARDUST4_SOUTH:
        if (debug) IDLog("Stop South\n");
        command = "RA0#";
        break;

        case ARDUST4_EAST:
        if (debug) IDLog("Stop East\n");
        command = "DEC0#";
        break;
    }


    if (debug)
        IDLog("stop command value is %s\n", command.c_str());

    rc = write_blocking(command);


    if (debug)
        IDLog("stopPulse returns %d\n",rc);
    if(rc==1)
        return true;

    return false;

}


bool ARDUST4Driver::write_blocking(std::string s){
  this->write(s);
  std::string res = this->wait_read();
  return res == "INITIALIZED#" || res == "OK#";
}

void ARDUST4Driver::write(std::string s){
  const char *cmd = s.c_str();
  int n_written = 0,
  spot = 0;

  do {
    n_written = ::write( fd, &cmd[spot], 1 );
    spot += n_written;
  } while (spot != s.length() && n_written > 0);
}

std::string ARDUST4Driver::wait_read(){
  int n = 0;
  int spot = 0;
  char buf = '\0';

  /* Whole response*/
  char response[1024];
  memset(response, '\0', sizeof response);

  do {
    n = read( fd, &buf, 1 );
    sprintf( &response[spot], "%c", buf );
    spot += n;
  } while( buf != '#' && n > 0);
  return std::string(response);
}
