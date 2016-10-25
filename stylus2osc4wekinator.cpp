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

#include "stylus2osc4wekinator.h"

enum
{
    INPUTEVENT_MOTION_NOTIFY,
    INPUTEVENT_MAX
};

// Tablet Vars
int gnDevListCnt = 0;
XDeviceInfoPtr gpDevList = NULL;
int gnLastXError = 0;
int gnInputEvent[INPUTEVENT_MAX] = { 0 };

// MainWindow Vars
MainWindow *MW;
AcquisitionThread *tablet_thread;
void ProcessTabletEvents(char *m);


XTablet::XTablet(void)
{
    fprintf(stdout,"Connecting to X Server...\n");
    this->Enabled = true;
    this->TabletDisplay = this->InitXInput();
    this->nEventListCnt = 0;
}

XTablet::~XTablet(void)
{
    // Release device list.
    XUngrabDevice(this->TabletDisplay,this->Tablet,CurrentTime);
    XFree(this->Tablet);
    XCloseDisplay(this->TabletDisplay);
    //delete this->TabletDisplay;
}

bool XTablet::ConnectDevice(std::string name)
{
    fprintf(stdout,"Trying to connect %s...\n",name.c_str());
    this->TabletInfoPtr = this->GetDevice(this->TabletDisplay,name.c_str());
    if (this->TabletInfoPtr==NULL)
        return false;
    else return true;
}

bool XTablet::OpenDevice(void)
{
    this->Tablet = XOpenDevice(this->TabletDisplay,this->TabletInfoPtr->id);
    if (!this->Tablet)
    {
        fprintf(stderr,"Unable to open input device.\n");
        return false;
    }
    else return true;
}

void XTablet::PrepareEvents(void)
{
    XEventClass cls;

    DeviceMotionNotify(this->Tablet,gnInputEvent[INPUTEVENT_MOTION_NOTIFY],cls);
    if (cls) eventList[nEventListCnt++] = cls;
    fprintf(stdout,"Device ID = %lu\n",this->TabletInfoPtr->id);
    XGrabDevice(this->TabletDisplay,this->Tablet,DefaultRootWindow(this->TabletDisplay),
                0,nEventListCnt,eventList,GrabModeAsync,GrabModeAsync,CurrentTime);
}

void XTablet::DispatchEvents(void)
{
    XEvent event;
    XAnyEvent* pAny;
    char message[MAXMESSAGE];

    while (this->Enabled)
    {
        XNextEvent(this->TabletDisplay,&event);
        pAny = reinterpret_cast<XAnyEvent*>(&event);
        sprintf(message,"x=%d y=%d p=%d tx=%d ty=%d w=%d",
                (reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[0],
                (reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[1],
                (reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[2],
                static_cast<short>((reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[3]&0xffff),
                static_cast<short>((reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[4]&0xffff),
                static_cast<short>((reinterpret_cast<XDeviceMotionEvent*>(pAny))->axis_data[5]&0xffff));
        this->EventCallback(message);
        // Flush data to terminal.
        fflush(stdout);
    }
}

int ErrorHandler(Display* pDisp, XErrorEvent* pEvent)
{
    char chBuf[64];
    XGetErrorText(pDisp,pEvent->error_code,chBuf,sizeof(chBuf));
    fprintf(stderr,"X Error: %d %s\n", pEvent->error_code, chBuf);
    gnLastXError  = pEvent->error_code;
    return 0;
}

Display* XTablet::InitXInput(void)
{
    Display* pDisp;
    int nMajor, nFEV, nFER;

    pDisp = XOpenDisplay(NULL);

    if (!pDisp)
    {
        fprintf(stderr,"Failed to connect to X server.\n");
        return NULL;
    }

    XSetErrorHandler(ErrorHandler);
    XSynchronize(pDisp,1);

    if (!XQueryExtension(pDisp,INAME,&nMajor,&nFEV,&nFER))
    {
        fprintf(stderr,"Server does not support XInput extension.\n");
        XCloseDisplay(pDisp);
        return NULL;
    }

    return pDisp;
}

XDeviceInfoPtr XTablet::GetDevice(Display* pDisp, const char* pszDeviceName)
{
    // Get list of Devices.
    if (!gpDevList)
    {
        gpDevList = reinterpret_cast<XDeviceInfoPtr>(XListInputDevices(pDisp, &gnDevListCnt));
        if (!gpDevList)
        {
            fprintf(stderr,"Failed to get input device list.\n");
            return NULL;
        }
    }

    // Find device by name.
    for (int i=0; i<gnDevListCnt; ++i)
    {
        if (!strcasecmp(gpDevList[i].name,pszDeviceName) &&
                gpDevList[i].num_classes)
            return gpDevList + i;
    }

    return NULL;
}


//============================================================
//============================================================


AcquisitionThread::AcquisitionThread(XTablet *Tab)
{
    this->Tablet = Tab;
}

void AcquisitionThread::run(void)
{
    Tablet->DispatchEvents();
}

MainWindow::MainWindow()
{
    MW = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Init()
{
    // Device initialisation
    T = new XTablet();
    T->EventCallback = ProcessTabletEvents;
    // Create the acquisition Thread
    tablet_thread = new AcquisitionThread(T);
    // Connection
    InitTablet(this->TabletName.toStdString().c_str());
}

void MainWindow::InitTablet(const char *devicename)
{
        if (!T->ConnectDevice(devicename))
    {
        fprintf(stderr,"Unable to connect Device %s",devicename);
    }
    else
    {
        fprintf(stdout,"Connected...\n");

        if (!T->OpenDevice())
        {
            fprintf(stderr,"Unable to open the device %s",devicename);
        }
        else
        {
            fprintf(stdout,"Connection opened...\n");
            T->PrepareEvents();
            fprintf(stdout,"Events can be received and send...\n");
            T->Enabled = true;
        }
    }
}

void MainWindow::InitOsc(void)
{
    address = lo_address_new(IP.toStdString().c_str(),OscPort.toStdString().c_str());
}

bool MainWindow::SendOscSixFloat(float f1,float f2, float f3, float f4, float f5, float f6)
{
    if (lo_send(address,OscAddress.toStdString().c_str(), "ffffff", f1,f2,f3,f4,f5,f6) == -1)
    {
        OscError(lo_address_errno(address), lo_address_errstr(address));
        return(false);
    }
    else return(true);
}

void MainWindow::OscError(int num, const char *message)
{
    char qmessage[512];
    sprintf(qmessage,"OSC error #%d : %s",num,message);
}

void ProcessTabletEvents(char *m)
{
    QString M(m);
    QString I;
    QStringList Part = M.split(" ");
    float x,y,p,tx,ty,w;

    sscanf(M.toStdString().c_str(),"x=%f y=%f p=%f tx=%f ty=%f w=%f",&x,&y,&p,&tx,&ty,&w);
    MW->SendOscSixFloat(x,y,p,tx,ty,w);

    // Display message in the console
    fprintf(stdout,"%s\n",m);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainWindow w;

    // Get arguments
//    w.TabletName = "Wacom Intuos5 touch L Pen stylus";
//    w.IP = "127.0.0.1";
//    w.OscAddress = "/wek/inputs";
//    w.OscPort = "6448";

    w.TabletName = QString(argv[1]);
    w.IP = QString(argv[2]);
    w.OscAddress = QString(argv[3]);
    w.OscPort = QString(argv[4]);

    // Show window
    w.Init();
    w.InitOsc();
    tablet_thread->start();

    //w.Stop();

    return a.exec();
}
