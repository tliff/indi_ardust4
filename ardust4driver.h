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

#ifndef ARDUST4DRIVER_H
#define ARDUST4DRIVER_H


/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/time.h>
#include <time.h>

#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

#include <iostream>

enum
{
    ARDUST4_NORTH,
    ARDUST4_SOUTH,
    ARDUST4_EAST,
    ARDUST4_WEST,
};

class ARDUST4Driver
{
    public:
    ARDUST4Driver(std::string device);
    virtual ~ARDUST4Driver();

    //  Generic indi device entries
    bool Connect();
    bool Disconnect();

    bool startPulse(int direction);
    bool stopPulse(int direction);

    void setDebug(bool enable) { debug = enable; }

private:
    char guideCMD[1];
    bool debug;
    std::string device;
    int fd;

    bool write_blocking(std::string);
    void write(std::string s);
    std::string wait_read();


};

#endif // ARDUST4Driver_H
