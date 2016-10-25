// stylus2osc4wekinator
// Author: Marc Lagies, 2016
//
// This program is based on Sylvain Hanneton's tablet2osc
// https://sourceforge.net/p/tablet2osc/wiki/Home/
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef STYLUS2OSC4WEKINATOR_H
#define STYLUS2OSC4WEKINATOR_H

#include "lo/lo.h"
#include <QThread>
#include <string>
#include <string.h>
#include <QStringList>
#include <QCoreApplication>
#include <X11/extensions/XInput.h>

#define MAXDEVICENAME 256
#define MAXMESSAGE  512

class XTablet
{
public :
    // X Properties
    bool    Enabled;
    Display *TabletDisplay;
    XDevice *Tablet;
    XDeviceInfoPtr TabletInfoPtr;
    int nEventListCnt;
    XEventClass eventList[32];

    // XTablet Object Methods
    XTablet();
    ~XTablet(void);
    bool ConnectDevice(std::string name);
    bool OpenDevice(void);
    void PrepareEvents(void);
    void DispatchEvents(void);
    void (*EventCallback)(char *m);

    // Methods from XIDUMP.C
    Display* InitXInput(void);
    XDeviceInfoPtr GetDevice(Display* pDisp, const char* pszDeviceName);
};

class MainWindow : public QObject
{
    Q_OBJECT

public:
    QString IP;
    QString OscAddress;
    QString TabletName;
    QString OscPort;
    lo_address address;
    MainWindow();
    ~MainWindow();
    void Init();
    void InitTablet(const char *devicename);
    void InitOsc(void);
    bool SendOscSixFloat(float f1,float f2, float f3, float f4, float f5, float f6);
    void OscError(int num, const char *message);

private:
    MainWindow *ui;
    XTablet *T;
};

class AcquisitionThread : public QThread
{
public :
    XTablet *Tablet;
    AcquisitionThread(XTablet *Tab);
    void run();
};

#endif // STYLUS2OSC4WEKINATOR_H
