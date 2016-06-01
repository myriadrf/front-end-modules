#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif //WX_PRECOMP

#include "lms7suiteAppFrame.h"
#include "IConnection.h"
#include "dlgConnectionSettings.h"
#include "lms7suiteEvents.h"
#include "pnlMiniLog.h"
#include "sonyPA_wxgui.h"
#include "SPI_wxgui.h"
#include <wx/string.h>
#include "dlgDeviceInfo.h"
#include <functional>
#include <ConnectionRegistry.h>
#include <LMSBoards.h>
#include <sstream>

using namespace std;
using namespace lime;

///////////////////////////////////////////////////////////////////////////

const wxString LMS7SuiteAppFrame::cWindowTitle = _("LMS7Suite");

void LMS7SuiteAppFrame::HandleLMSevent(wxCommandEvent& event)
{

}

LMS7SuiteAppFrame::LMS7SuiteAppFrame( wxWindow* parent ) :
    AppFrame_view( parent ), lms7controlPort(nullptr), streamBoardPort(nullptr)
{
#ifndef __unix__
    SetIcon(wxIcon(_("aaaaAPPicon")));
#endif
    sonyPA = new SonyPA_wxgui(this, wxNewId(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE);
    sonyPA->Initialize(lms7controlPort);
    spi = nullptr;
    deviceInfo = nullptr;

    //mContent->Initialize(lms7controlPort);
    mMiniLog = new pnlMiniLog(this, wxNewId());
    Connect(LOG_MESSAGE, wxCommandEventHandler(LMS7SuiteAppFrame::OnLogMessage), 0, this);

    contentSizer->Add(sonyPA, 1, wxEXPAND, 5);
    contentSizer->Add(mMiniLog, 1, wxEXPAND, 5);

	Layout();
	Fit();

    SetMinSize(GetSize());
    UpdateConnections(lms7controlPort, lms7controlPort);
}

LMS7SuiteAppFrame::~LMS7SuiteAppFrame()
{
    ConnectionRegistry::freeConnection(lms7controlPort);
    ConnectionRegistry::freeConnection(streamBoardPort);
}

void LMS7SuiteAppFrame::OnClose( wxCloseEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnQuit( wxCommandEvent& event )
{
    Destroy();
}

void LMS7SuiteAppFrame::OnShowConnectionSettings( wxCommandEvent& event )
{
	dlgConnectionSettings dlg(this);

    dlg.SetConnectionManagers(&lms7controlPort, &streamBoardPort);
    Bind(CONTROL_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), this);
    Bind(DATA_PORT_CONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), this);
    Bind(CONTROL_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnControlBoardConnect), this);
    Bind(DATA_PORT_DISCONNECTED, wxCommandEventHandler(LMS7SuiteAppFrame::OnDataBoardConnect), this);
	dlg.ShowModal();
}


void LMS7SuiteAppFrame::OnControlBoardConnect(wxCommandEvent& event)
{
    UpdateConnections(lms7controlPort, streamBoardPort);
    const int controlCollumn = 1;
    if (lms7controlPort && lms7controlPort->IsOpen())
    {
        //bind callback for spi data logging
        lms7controlPort->SetDataLogCallback(bind(&LMS7SuiteAppFrame::OnLogDataTransfer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        DeviceInfo info = lms7controlPort->GetDeviceInfo();
        wxString controlDev = _("Control port: ");
        controlDev.Append(info.deviceName);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s"), info.firmwareVersion, info.hardwareVersion, info.protocolVersion));
        statusBar->SetStatusText(controlDev, controlCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
    }
    else
    {
        statusBar->SetStatusText(_("Control port: Not Connected"), controlCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Disconnected control port"));
        wxPostEvent(this, evt);
    }
}

void LMS7SuiteAppFrame::OnDataBoardConnect(wxCommandEvent& event)
{
    UpdateConnections(lms7controlPort, streamBoardPort);
    const int dataCollumn = 2;
    if (streamBoardPort && streamBoardPort->IsOpen())
    {
        //bind callback for spi data logging
        streamBoardPort->SetDataLogCallback(bind(&LMS7SuiteAppFrame::OnLogDataTransfer, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        DeviceInfo info = streamBoardPort->GetDeviceInfo();
        wxString controlDev = _("Data port: ");
        controlDev.Append(info.deviceName);
        controlDev.Append(wxString::Format(_(" FW:%s HW:%s Protocol:%s"), info.firmwareVersion, info.hardwareVersion, info.protocolVersion));
        statusBar->SetStatusText(controlDev, dataCollumn);

        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Connected ") + controlDev);
        wxPostEvent(this, evt);
    }
    else
    {
        statusBar->SetStatusText(_("Data port: Not Connected"), dataCollumn);
        wxCommandEvent evt;
        evt.SetEventType(LOG_MESSAGE);
        evt.SetString(_("Disconnected data port"));
        wxPostEvent(this, evt);
    }
}

void LMS7SuiteAppFrame::OnLogMessage(wxCommandEvent &event)
{
    if (mMiniLog)
        mMiniLog->HandleMessage(event);
}

void LMS7SuiteAppFrame::OnDeviceInfoClose(wxCloseEvent& event)
{
    deviceInfo->Destroy();
    deviceInfo = nullptr;
}

void LMS7SuiteAppFrame::OnShowDeviceInfo(wxCommandEvent& event)
{
    if (deviceInfo) //it's already opened
        deviceInfo->Show();
    else
    {
        deviceInfo = new dlgDeviceInfo(this, wxNewId(), _("Device Info"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
        deviceInfo->Initialize(lms7controlPort, streamBoardPort);
        deviceInfo->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnDeviceInfoClose), NULL, this);
        deviceInfo->Show();
    }
}

void LMS7SuiteAppFrame::OnSPIClose(wxCloseEvent& event)
{
    spi->Destroy();
    spi = nullptr;
}

void LMS7SuiteAppFrame::OnShowSPI(wxCommandEvent& event)
{
    if (spi) //it's already opened
        spi->Show();
    else
    {
        spi = new SPI_wxgui(this, wxNewId(), _("Device Info"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE);
		spi->Initialize(lms7controlPort, lms7controlPort);
        spi->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LMS7SuiteAppFrame::OnSPIClose), NULL, this);
        spi->Show();
    }
}

#include <iomanip>
void LMS7SuiteAppFrame::OnLogDataTransfer(bool Tx, const unsigned char* data, const unsigned int length)
{
    if (mMiniLog == nullptr || mMiniLog->chkLogData->IsChecked() == false)
        return;
    stringstream ss;
    ss << (Tx ? "Wr(" : "Rd(");
    ss << length << "): ";
    ss << std::hex << std::setfill('0');
    int repeatedZeros = 0;
    for (int i = length - 1; i >= 0; --i)
        if (data[i] == 0)
            ++repeatedZeros;
        else
            break;
    if (repeatedZeros == 2)
        repeatedZeros = 0;
    repeatedZeros = repeatedZeros - (repeatedZeros & 0x1);
    for (int i = 0; i<length - repeatedZeros; ++i)
        //casting to short to print as numbers
        ss << " " << std::setw(2) << (unsigned short)data[i];
    if (repeatedZeros > 2)
        ss << " (00 x " << std::dec << repeatedZeros << " times)";
    cout << ss.str() << endl;
    wxCommandEvent *evt = new wxCommandEvent();
    evt->SetString(ss.str());
    evt->SetEventObject(this);
    evt->SetEventType(LOG_MESSAGE);
    wxQueueEvent(this, evt);
}


void LMS7SuiteAppFrame::UpdateConnections(IConnection* lms7controlPort, IConnection* streamBoardPort)
{
    if(sonyPA)
        sonyPA->Initialize(lms7controlPort);
    if(deviceInfo)
        deviceInfo->Initialize(lms7controlPort, streamBoardPort);
    if(spi)
		spi->Initialize(lms7controlPort, lms7controlPort);

}
