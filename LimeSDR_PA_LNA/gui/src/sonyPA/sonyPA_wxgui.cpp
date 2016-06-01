/**
@file 	SonyPA_wxgui.cpp
@author Lime Microsystems
@brief 	panel for interacting with HPM7 board
*/

#include "ErrorReporting.h"
#include "sonyPA_wxgui.h"
#include "lms7suiteEvents.h"

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <vector>
#include "LMS64CProtocol.h"
#include "cpp-feather-ini-parser/INI.h"

using namespace lime;
using namespace std;

BEGIN_EVENT_TABLE(SonyPA_wxgui, wxPanel)

END_EVENT_TABLE()

SonyPA_wxgui::SonyPA_wxgui(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long styles)
{
    m_serPort = nullptr;
    Create(parent, id, wxDefaultPosition, wxDefaultSize, styles, _T("id"));
#ifdef WIN32
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif
    wxFlexGridSizer* cacheSizer = new wxFlexGridSizer(0, 3, 5, 5);
    btnImportCache = new wxButton(this, wxNewId(), _("Open"));
    cacheSizer->Add(btnImportCache, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(btnImportCache->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&SonyPA_wxgui::ImportCache);

    btnExportCache = new wxButton(this, wxNewId(), _("Save As"));
    cacheSizer->Add(btnExportCache, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(btnExportCache->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&SonyPA_wxgui::ExportCache);

    btnClearCache = new wxButton(this, wxNewId(), _("Clear"));
    cacheSizer->Add(btnClearCache, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(btnClearCache->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&SonyPA_wxgui::ClearCache);

    cmbCacheEntry = new wxComboBox(this, wxNewId(), wxEmptyString);
    cacheSizer->Add(cmbCacheEntry, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(cmbCacheEntry->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::ChangeState);

    btnAddEntry = new wxButton(this, wxNewId(), _("Add/Update"));
    cacheSizer->Add(btnAddEntry, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(btnAddEntry->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&SonyPA_wxgui::AddEntry);

    btnRemoveEntry = new wxButton(this, wxNewId(), _("Remove"));
    cacheSizer->Add(btnRemoveEntry, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    Connect(btnRemoveEntry->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&SonyPA_wxgui::RemoveEntry);



    wxFlexGridSizer* panelSizer = new wxFlexGridSizer(0, 1, 5, 5);
    wxFlexGridSizer* mainSizer = new wxFlexGridSizer(0, 2, 5, 5);
    wxFlexGridSizer* tunersSizer = new wxFlexGridSizer(0, 3, 5, 5);
    wxStaticBoxSizer* tunerGroup;
    wxStaticText* stext;
    wxString tunerNames[] = {
        _("TUNER_A_IN"),
        _("TUNER_A_MID"),
        _("TUNER_A_OUT"),
        _("TUNER_B_IN"),
        _("TUNER_B_MID"),
        _("TUNER_B_OUT")
    };

    wxArrayString ssc1_choices;
    for (int i = 0; i < pow(2.0, 5); ++i)
        ssc1_choices.push_back(wxString::Format("%i", i));
    wxArrayString ssc2_choices;
    for (int i = 0; i < pow(2.0, 4); ++i)
        ssc2_choices.push_back(wxString::Format("%i", i));

    for (int i = 0; i < 6; ++i)
    {
        tunerIds.push_back(wxNewId());
        cmbSSC1ids.push_back(wxNewId());
        tunerGroup = new wxStaticBoxSizer(wxVERTICAL, this, tunerNames[i]);
        chkEB.push_back(new wxCheckBox(this, tunerIds[i], _("Ext. branch (F11)")));
        Connect(tunerIds[i], wxEVT_CHECKBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
        tunerGroup->Add(chkEB[i], 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
        chkTP.push_back(new wxCheckBox(this, tunerIds[i], _("Through path (F10)")));
        Connect(tunerIds[i], wxEVT_CHECKBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
        tunerGroup->Add(chkTP[i], 1, wxALIGN_LEFT | wxALIGN_TOP, 0);

        stext = new wxStaticText(this, wxNewId(), _("SSC2 (F9-F6)"));
        tunerGroup->Add(stext);
        cmbSSC2.push_back(new wxChoice(this, tunerIds[i], wxDefaultPosition, wxDefaultSize, ssc2_choices));
        Connect(tunerIds[i], wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
        cmbSSC2[i]->SetSelection(0);
        tunerGroup->Add(cmbSSC2[i], 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);

        stext = new wxStaticText(this, wxNewId(), _("SSC1 (F5-F1)"));
        tunerGroup->Add(stext);
        cmbSSC1.push_back(new wxChoice(this, cmbSSC1ids[i], wxDefaultPosition, wxDefaultSize, ssc1_choices));
        Connect(cmbSSC1ids[i], wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
        cmbSSC1[i]->SetSelection(0);
        tunerGroup->Add(cmbSSC1[i], 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
        tunersSizer->Add(tunerGroup);
    }

    wxFlexGridSizer* leftCollumn = new wxFlexGridSizer(0, 1, 5, 5);
    btnUpdateAll = new wxButton(this, wxNewId(), _("Read All"));
    Connect(btnUpdateAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SonyPA_wxgui::DownloadAll);
    leftCollumn->Add(btnUpdateAll, 1, wxALIGN_LEFT | wxALIGN_TOP, 0);
    wxFlexGridSizer* gpioControls = new wxFlexGridSizer(0, 1, 5, 5);
    const wxString activePathChoices[] = { _("No path active"), _("LNAH"), _("LNAL"), _("LNAW") };

    gpioControls->Add(new wxStaticText(this, wxID_ANY, _("PAs Vd Driver:")), 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);
    const wxString paChoices[] = { _("5V"), _("2V") };
    //GPIO0
    //  0  5V
    //  1  2V
    cmbPAdriver = new wxChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, 2, paChoices);
    cmbPAdriver->SetSelection(0);
    Connect(cmbPAdriver->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
    gpioControls->Add(cmbPAdriver, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 5);
    leftCollumn->Add(gpioControls, 1, wxALIGN_LEFT | wxALIGN_TOP | wxEXPAND, 0);

    //DACs
    wxFlexGridSizer* dacSizer = new wxFlexGridSizer(0, 2, 5, 5);
    wxArrayString dac_choices;
    for (int i = 0; i < 256; ++i)
        dac_choices.push_back(wxString::Format("%.2f V", i*3.3/256));
    cmbDAC_A = new wxChoice(this, wxNewId(), wxDefaultPosition, wxSize(64, -1), dac_choices);
    cmbDAC_A->SetSelection(0);
    dacSizer->Add(new wxStaticText(this, wxNewId(), "DAC_A: "), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    dacSizer->Add(cmbDAC_A, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);

    cmbDAC_B = new wxChoice(this, wxNewId(), wxDefaultPosition, wxSize(64, -1), dac_choices);
    cmbDAC_B->SetSelection(0);
    dacSizer->Add(new wxStaticText(this, wxNewId(), "DAC_B: "), 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    dacSizer->Add(cmbDAC_B, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 0);
    Connect(cmbDAC_A->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
    Connect(cmbDAC_B->GetId(), wxEVT_COMBOBOX, (wxObjectEventFunction)&SonyPA_wxgui::UploadAll);
    wxStaticBoxSizer* dacGroup = new wxStaticBoxSizer(wxVERTICAL, this, _("DAC"));
    dacGroup->Add(dacSizer);

    leftCollumn->Add(dacGroup);
    mainSizer->Add(leftCollumn);
    mainSizer->Add(tunersSizer);

    panelSizer->Add(cacheSizer);
    panelSizer->Add(mainSizer);

    SetSizer(panelSizer);
    panelSizer->Fit(this);
    panelSizer->SetSizeHints(this);
    Layout();
}

void SonyPA_wxgui::Initialize(IConnection* serPort)
{
	m_serPort = dynamic_cast<LMS64CProtocol *>(serPort);
}

SonyPA_wxgui::~SonyPA_wxgui()
{

}

void SonyPA_wxgui::DownloadAll(wxCommandEvent& event)
{
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    uint16_t value;
    m_serPort->ReadRegister(0x0010, value);
    cmbPAdriver->SetSelection(value & 1);

    for (int i = 0; i < chkEB.size(); ++i)
    {
        m_serPort->ReadRegister(0x0020+i, value);
        chkEB[i]->SetValue((value >> 10) & 1);
        chkTP[i]->SetValue((value >> 9) & 1);
        cmbSSC2[i]->SetSelection((value >> 5) & 0xF);
        cmbSSC1[i]->SetSelection((value >> 0) & 0x1F);
    }

    m_serPort->ReadRegister(0x0030, value);
    cmbDAC_A->SetSelection(value & 0xFF);
    m_serPort->ReadRegister(0x0031, value);
    cmbDAC_B->SetSelection(value & 0xFF);
}

void SonyPA_wxgui::UploadAll(wxCommandEvent& event)
{
    if (m_serPort == nullptr || m_serPort->IsOpen() == false)
    {
        wxMessageBox(_("Board not connected"), _("Warning"));
        return;
    }

    vector<uint32_t> addrs;
    vector<uint32_t> values;

    uint16_t value;
    addrs.push_back(0x0010); values.push_back(cmbPAdriver->GetSelection() & 1);

    for (int i = 0; i < chkEB.size(); ++i)
    {
        uint16_t value = 0;
        value |= (chkEB[i]->GetValue() & 1) << 10;
        value |= (chkTP[i]->GetValue() & 1) << 9;
        value |= (cmbSSC2[i]->GetSelection() & 0xF) << 5;
        value |= (cmbSSC1[i]->GetSelection() & 0x1F);
        //m_serPort->WriteRegister(0x0020+i, value);
        addrs.push_back(0x0020+i);
        values.push_back(value);
    }
    addrs.push_back(0x0030);
    values.push_back(cmbDAC_A->GetSelection() & 0xFF);
    addrs.push_back(0x0031);
    values.push_back(cmbDAC_B->GetSelection() & 0xFF);
//    m_serPort->WriteRegister(0x0030, cmbDAC_A->GetSelection() & 0xFF);
//    m_serPort->WriteRegister(0x0031, cmbDAC_B->GetSelection() & 0xFF);
    if(m_serPort->WriteRegisters(addrs.data(), values.data(), addrs.size()) != 0)
        wxMessageBox(GetLastErrorMessage(), _("ERROR"));
}

SonyPA_wxgui::ControlsState SonyPA_wxgui::GetGUIState()
{
    ControlsState temp;
    temp.name = cmbCacheEntry->GetValue();
    temp.paDriver = cmbPAdriver->GetSelection();
    temp.A.dac = cmbDAC_A->GetSelection();
    temp.B.dac = cmbDAC_B->GetSelection();
    for(int i=0; i<3; ++i)
    {
        temp.A.extBranch[i] = chkEB[i]->GetValue();
        temp.A.throughPath[i] = chkTP[i]->GetValue();
        temp.A.ssc1[i] = cmbSSC1[i]->GetSelection();
        temp.A.ssc2[i] = cmbSSC2[i]->GetSelection();
    }
    for(int i=0; i<3; ++i)
    {
        temp.B.extBranch[i] = chkEB[i+3]->GetValue();
        temp.B.throughPath[i] = chkTP[i+3]->GetValue();
        temp.B.ssc1[i] = cmbSSC1[i+3]->GetSelection();
        temp.B.ssc2[i] = cmbSSC2[i+3]->GetSelection();
    }
    return temp;
}

void SonyPA_wxgui::SetGUIState(const ControlsState& state)
{
    cmbPAdriver->SetSelection(state.paDriver);
    cmbDAC_A->SetSelection(state.A.dac);
    cmbDAC_B->SetSelection(state.B.dac);
    for(int i=0; i<3; ++i)
    {
        chkEB[i]->SetValue(state.A.extBranch[i]);
        chkTP[i]->SetValue(state.A.throughPath[i]);
		cmbSSC1[i]->SetSelection(state.A.ssc1[i]);
		cmbSSC2[i]->SetSelection(state.A.ssc2[i]);
    }
    for(int i=0; i<3; ++i)
    {
        chkEB[i+3]->SetValue(state.B.extBranch[i]);
        chkTP[i+3]->SetValue(state.B.throughPath[i]);
		cmbSSC1[i+3]->SetSelection(state.B.ssc1[i]);
		cmbSSC2[i+3]->SetSelection(state.B.ssc2[i]);
    }
}

void SonyPA_wxgui::ExportCache(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Save config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    typedef INI<string, string, string> ini_t;
    ini_t parser(dlg.GetPath().ToStdString().c_str(), false);

    fstream fout;
    fout.open((dlg.GetPath()+".csv").ToStdString().c_str(), ios::out);
	fout << "name, pa_vd, dac_a, eb_a_in, tp_a_in, ssc2_a_in, ssc1_a_in, eb_a_mid, tp_a_mid, ssc2_a_mid, ssc1_a_mid, eb_a_out, tp_a_out, ssc2_a_out, ssc1_a_out,\
dac_b, eb_b_in, tp_b_in, ssc2_b_in, ssc1_b_in, eb_b_mid, tp_b_mid, ssc2_b_mid, ssc1_b_mid, eb_b_out, tp_b_out, ssc2_b_out, ssc1_b_out," << endl;
    for(auto iter : cache)
    {
		parser.create(iter.first);
        ControlsState state = iter.second;
        parser.set("PA", state.paDriver);
        parser.set("DAC_A", state.A.dac);
        parser.set( "EB_A_IN", state.A.extBranch[0]);
        parser.set( "EB_A_MID", state.A.extBranch[1]);
        parser.set( "EB_A_OUT", state.A.extBranch[2]);
        parser.set( "TP_A_IN", state.A.throughPath[0]);
        parser.set( "TP_A_MID", state.A.throughPath[1]);
        parser.set( "TP_A_OUT", state.A.throughPath[2]);
        parser.set( "SSC1_A_IN", state.A.ssc1[0]);
        parser.set( "SSC1_A_MID", state.A.ssc1[1]);
        parser.set( "SSC1_A_OUT", state.A.ssc1[2]);
        parser.set( "SSC2_A_IN", state.A.ssc2[0]);
        parser.set( "SSC2_A_MID", state.A.ssc2[1]);
        parser.set( "SSC2_A_OUT", state.A.ssc2[2]);
        parser.set( "DAC_B", state.B.dac);
        parser.set( "EB_B_IN", state.B.extBranch[0]);
        parser.set( "EB_B_MID", state.B.extBranch[1]);
        parser.set( "EB_B_OUT", state.B.extBranch[2] );
        parser.set( "TP_B_IN", state.B.throughPath[0]);
        parser.set( "TP_B_MID", state.B.throughPath[1]);
        parser.set( "TP_B_OUT", state.B.throughPath[2]);
        parser.set( "SSC1_B_IN", state.B.ssc1[0]);
        parser.set( "SSC1_B_MID", state.B.ssc1[1]);
        parser.set( "SSC1_B_OUT", state.B.ssc1[2]);
        parser.set( "SSC2_B_IN", state.B.ssc2[0]);
        parser.set( "SSC2_B_MID", state.B.ssc2[1]);
        parser.set( "SSC2_B_OUT", state.B.ssc2[2]);

        fout << state.name << ",";
        fout << cmbPAdriver->GetString(state.paDriver) << ",";
        fout << cmbDAC_A->GetString(state.A.dac) << ",";
        fout << state.A.extBranch[0] << ",";
		fout << state.A.throughPath[0] << ",";
		fout << state.A.ssc2[0] << ",";
		fout << state.A.ssc1[0] << ",";

        fout << state.A.extBranch[1] << ",";
		fout << state.A.throughPath[1] << ",";
		fout << state.A.ssc2[1] << ",";
		fout << state.A.ssc1[1] << ",";

        fout << state.A.extBranch[2] << ",";
        fout << state.A.throughPath[2] << ",";
		fout << state.A.ssc2[2] << ",";
        fout << state.A.ssc1[2] << ",";

		fout << cmbDAC_B->GetString(state.B.dac) << ",";
		fout << state.B.extBranch[0] << ",";
		fout << state.B.throughPath[0] << ",";
		fout << state.B.ssc2[0] << ",";
		fout << state.B.ssc1[0] << ",";

		fout << state.B.extBranch[1] << ",";
		fout << state.B.throughPath[1] << ",";
		fout << state.B.ssc2[1] << ",";
		fout << state.B.ssc1[1] << ",";

		fout << state.B.extBranch[2] << ",";
		fout << state.B.throughPath[2] << ",";
		fout << state.B.ssc2[2] << ",";
		fout << state.B.ssc1[2] << ",";
        fout << endl;
        
    }
    fout.close();
    parser.save();
}

void SonyPA_wxgui::ImportCache(wxCommandEvent& event)
{
    wxFileDialog dlg(this, _("Open config file"), "", "", "Project-File (*.ini)|*.ini", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_CANCEL)
        return;
    string filename = dlg.GetPath().ToStdString();

    ifstream f(filename);
    if (f.good() == false) //file not found
    {
        f.close();
        ReportError(ENOENT, "Import PA data(%s) - file not found", filename.c_str());
        return;
    }
    f.close();
    typedef INI<string, string, string> ini_t;
    ini_t parser(filename, true);
    //Loop through sections
    cache.clear();
	cmbCacheEntry->Clear();
    for(ini_t::sectionsit_t i = parser.sections.begin(); i != parser.sections.end(); i++)
    {
        ControlsState state;
        state.name = i->first;
        state.paDriver = parser.get(i->first, "PA", 0);
        state.A.dac = parser.get(i->first, "DAC_A", 0);
        state.A.extBranch[0] = parser.get(i->first, "EB_A_IN", 0);
        state.A.extBranch[1] = parser.get(i->first, "EB_A_MID", 0);
        state.A.extBranch[2] = parser.get(i->first, "EB_A_OUT", 0);
        state.A.throughPath[0] = parser.get(i->first, "TP_A_IN", 0);
        state.A.throughPath[1] = parser.get(i->first, "TP_A_MID", 0);
        state.A.throughPath[2] = parser.get(i->first, "TP_A_OUT", 0);
        state.A.ssc1[0] = parser.get(i->first, "SSC1_A_IN", 0);
        state.A.ssc1[1] = parser.get(i->first, "SSC1_A_MID", 0);
        state.A.ssc1[2] = parser.get(i->first, "SSC1_A_OUT", 0);
        state.A.ssc2[0] = parser.get(i->first, "SSC2_A_IN", 0);
        state.A.ssc2[1] = parser.get(i->first, "SSC2_A_MID", 0);
        state.A.ssc2[2] = parser.get(i->first, "SSC2_A_OUT", 0);
        state.B.dac = parser.get(i->first, "DAC_B", 0);
        state.B.extBranch[0] = parser.get(i->first, "EB_B_IN", 0);
        state.B.extBranch[1] = parser.get(i->first, "EB_B_MID", 0);
        state.B.extBranch[2] = parser.get(i->first, "EB_B_OUT", 0);
        state.B.throughPath[0] = parser.get(i->first, "TP_B_IN", 0);
        state.B.throughPath[1] = parser.get(i->first, "TP_B_MID", 0);
        state.B.throughPath[2] = parser.get(i->first, "TP_B_OUT", 0);
        state.B.ssc1[0] = parser.get(i->first, "SSC1_B_IN", 0);
        state.B.ssc1[1] = parser.get(i->first, "SSC1_B_MID", 0);
        state.B.ssc1[2] = parser.get(i->first, "SSC1_B_OUT", 0);
        state.B.ssc2[0] = parser.get(i->first, "SSC2_B_IN", 0);
        state.B.ssc2[1] = parser.get(i->first, "SSC2_B_MID", 0);
        state.B.ssc2[2] = parser.get(i->first, "SSC2_B_OUT", 0);
        cache[state.name] = state;
		cmbCacheEntry->Append(state.name);
    }
    if(cache.size() > 0)
    {
        SetGUIState(cache.begin()->second);
    }
}

void SonyPA_wxgui::RemoveEntry(wxCommandEvent& event)
{
    string name = cmbCacheEntry->GetValue().ToStdString();
    auto iter = cache.find(name);
    if(iter != cache.end())
    {
        cache.erase(iter);
        int res = cmbCacheEntry->FindString(name);
        cmbCacheEntry->Delete(res);
    }
}

void SonyPA_wxgui::AddEntry(wxCommandEvent& event)
{
    string name = cmbCacheEntry->GetValue().ToStdString();
    auto iter = cache.find(name);
    if(iter == cache.end())
        cmbCacheEntry->Append(name);

    cache[name] = GetGUIState();
}

void SonyPA_wxgui::ChangeState(wxCommandEvent& event)
{
    string name = cmbCacheEntry->GetValue().ToStdString();
    SetGUIState(cache[name]);
    UploadAll(event);
}

void SonyPA_wxgui::ClearCache(wxCommandEvent& event)
{
    cache.clear();
    cmbCacheEntry->SetValue(_(""));
	cmbCacheEntry->Clear();
}
