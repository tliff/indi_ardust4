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
#include "ardust4.h"
#include "ardust4driver.h"
#include <memory>

#define POLLMS 250

// We declare an auto pointer to gpGuide.
std::unique_ptr<ARDUST4> gpGuide(new ARDUST4());

void ISGetProperties(const char *dev)
{
        gpGuide->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
        gpGuide->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
        gpGuide->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
        gpGuide->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
  INDI_UNUSED(dev);
  INDI_UNUSED(name);
  INDI_UNUSED(sizes);
  INDI_UNUSED(blobsizes);
  INDI_UNUSED(blobs);
  INDI_UNUSED(formats);
  INDI_UNUSED(names);
  INDI_UNUSED(n);
}
void ISSnoopDevice (XMLEle *root)
{
    INDI_UNUSED(root);
}

ARDUST4::ARDUST4()
{
    driver = new ARDUST4Driver("/dev/ttyUSB0");
    WEDir = NSDir = 0;
    InWEPulse = InNSPulse = false;
    WEPulseRequest = NSPulseRequest =0;
    WEtimerID = NStimerID = 0;
}

ARDUST4::~ARDUST4()
{
    //dtor

    delete (driver);

}


const char * ARDUST4::getDefaultName()
{
    return (char *)"ARDUST4";
}

bool ARDUST4::Connect()
{
    driver->setDebug(isDebug());

    bool rc = driver->Connect();

    if (rc)
        IDMessage(getDeviceName(), "ARDUST4 is online.");
    else
        IDMessage(getDeviceName(), "Error: cannot find ARDUST4 device.");

    return rc;

}

bool ARDUST4::Disconnect()
{
    IDMessage(getDeviceName(), "GPSUSB is offline.");

    return driver->Disconnect();
}

bool ARDUST4::initProperties()
{
    initGuiderProperties(getDeviceName(), MAIN_CONTROL_TAB);

    addDebugControl();

    return INDI::DefaultDevice::initProperties();
}

bool ARDUST4::updateProperties()
{

    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineNumber(&GuideNSNP);
        defineNumber(&GuideWENP);
    }
    else
    {
        deleteProperty(GuideNSNP.name);
        deleteProperty(GuideWENP.name);
    }

    return true;

}

void ARDUST4::ISGetProperties (const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);
}

bool ARDUST4::ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n)
{
    if(strcmp(dev,getDeviceName())==0)
    {
        if (!strcmp(name,GuideNSNP.name) || !strcmp(name,GuideWENP.name))
        {
            processGuiderProperties(name, values, names, n);
            return true;
        }
    }

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool ARDUST4::ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n)
{
    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool ARDUST4::ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n)
{
    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool ARDUST4::ISSnoopDevice (XMLEle *root)
{
    return INDI::DefaultDevice::ISSnoopDevice(root);
}

void ARDUST4::debugTriggered(bool enable)
{
    driver->setDebug(enable);
}

float ARDUST4::CalcWEPulseTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now,NULL);

    timesince=(double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(WEPulseStart.tv_sec * 1000.0 + WEPulseStart.tv_usec/1000);
    timesince=timesince/1000;


    timeleft=WEPulseRequest-timesince;
    return timeleft;
}

float ARDUST4::CalcNSPulseTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now,NULL);

    timesince=(double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(NSPulseStart.tv_sec * 1000.0 + NSPulseStart.tv_usec/1000);
    timesince=timesince/1000;


    timeleft=NSPulseRequest-timesince;
    return timeleft;
}


void ARDUST4::TimerHit()
{
    float timeleft;

    if(InWEPulse)
    {
        timeleft=CalcWEPulseTimeLeft();

        if(timeleft < 1.0)
        {
            if(timeleft > 0.25)
            {
                //  a quarter of a second or more
                //  just set a tighter timer
                WEtimerID = SetTimer(250);
            } else
            {
                if(timeleft >0.07)
                {
                    //  use an even tighter timer
                    WEtimerID = SetTimer(50);
                } else
                {
                    //  it's real close now, so spin on it
                    while(timeleft > 0)
                    {
                        int slv;
                        slv=100000*timeleft;
                        //IDLog("usleep %d\n",slv);
                        usleep(slv);
                        timeleft=CalcWEPulseTimeLeft();
                    }


                    driver->stopPulse(WEDir);
                    InWEPulse = false;

                    // If we have another pulse, keep going
                    if (!InNSPulse)
                        SetTimer(250);

                }
            }
        } else if (!InNSPulse)
        {
            WEtimerID = SetTimer(250);
        }
    }

    if(InNSPulse)
    {
        timeleft=CalcNSPulseTimeLeft();

        if(timeleft < 1.0)
        {
            if(timeleft > 0.25)
            {
                //  a quarter of a second or more
                //  just set a tighter timer
                NStimerID =  SetTimer(250);
            } else
            {
                if(timeleft >0.07)
                {
                    //  use an even tighter timer
                    NStimerID = SetTimer(50);
                } else
                {
                    //  it's real close now, so spin on it
                    while(timeleft > 0)
                    {
                        int slv;
                        slv=100000*timeleft;
                        //IDLog("usleep %d\n",slv);
                        usleep(slv);
                        timeleft=CalcNSPulseTimeLeft();
                    }

                    driver->stopPulse(NSDir);
                    InNSPulse = false;
                }
            }
        } else
        {
            NStimerID = SetTimer(250);
        }
    }

}

IPState ARDUST4::GuideNorth(float ms)
{

    RemoveTimer(NStimerID);


    driver->startPulse(ARDUST4_NORTH);

    NSDir = ARDUST4_NORTH;

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting NORTH guide");

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(ARDUST4_NORTH);
        return IPS_OK;
    }

    NSPulseRequest=ms/1000.0;
    gettimeofday(&NSPulseStart,NULL);
    InNSPulse=true;


    NStimerID = SetTimer(ms-50);

    return IPS_BUSY;
}

IPState ARDUST4::GuideSouth(float ms)
{
    RemoveTimer(NStimerID);

    driver->startPulse(ARDUST4_SOUTH);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting SOUTH guide");

    NSDir = ARDUST4_SOUTH;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(ARDUST4_SOUTH);
        return IPS_OK;
    }

    NSPulseRequest=ms/1000.0;
    gettimeofday(&NSPulseStart,NULL);
    InNSPulse=true;


    NStimerID = SetTimer(ms-50);

    return IPS_BUSY;
}

IPState ARDUST4::GuideEast(float ms)
{
    RemoveTimer(WEtimerID);

    driver->startPulse(ARDUST4_EAST);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting EAST guide");

    WEDir = ARDUST4_EAST;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(ARDUST4_EAST);
        return IPS_OK;
    }

    WEPulseRequest=ms/1000.0;
    gettimeofday(&WEPulseStart,NULL);
    InWEPulse=true;

    WEtimerID = SetTimer(ms-50);

    return IPS_BUSY;
}

IPState ARDUST4::GuideWest(float ms)
{

    RemoveTimer(WEtimerID);

    driver->startPulse(ARDUST4_WEST);

    DEBUG(INDI::Logger::DBG_DEBUG, "Starting WEST guide");

    WEDir = ARDUST4_WEST;

    if (ms <= POLLMS)
    {

        usleep(ms*1000);

        driver->stopPulse(ARDUST4_WEST);
        return IPS_OK;
    }

    WEPulseRequest=ms/1000.0;
    gettimeofday(&WEPulseStart,NULL);
    InWEPulse=true;


    WEtimerID = SetTimer(ms-50);

    return IPS_BUSY;

}
