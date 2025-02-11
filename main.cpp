#include <wx/wx.h>
#include "serial.h"
#include <fstream>
#include <thread>
#include <chrono>
#include <sys/stat.h> // For retrieving file modification time

class MyApp : public wxApp {
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame {
public:
    MyFrame(const wxString& title);
    ~MyFrame();

private:
    void OnPortSelect(wxCommandEvent& event);
    void StartFileReading();
    void ClosePort();  // Added method to handle closing the port

    wxChoice* m_portChoice;
    SerialPort* m_serialPort;
    bool m_keepReading;
    std::thread m_readThread;
    std::string m_lastSelectedPort;  // Added variable to store the last selected port name
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("AI-based Defect Serial Communicator(7th Sem FYP, NIEIT)");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 200)), m_serialPort(nullptr), m_keepReading(false), m_lastSelectedPort("") {

    wxPanel* panel = new wxPanel(this, -1);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    wxArrayString portChoices;
    std::vector<std::string> ports = SerialPort::enumeratePorts();
    for (const auto& port : ports) {
        portChoices.Add(port);
    }

    if (portChoices.IsEmpty()) {
        portChoices.Add("No ports available");
    }

    m_portChoice = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, portChoices);
    vbox->Add(m_portChoice, 0, wxALL | wxCENTER, 10);

    panel->SetSizer(vbox);

    m_portChoice->Bind(wxEVT_CHOICE, &MyFrame::OnPortSelect, this);
}

MyFrame::~MyFrame() {
    m_keepReading = false;
    if (m_readThread.joinable()) {
        m_readThread.join();
    }
    ClosePort();  // Ensure the port is properly closed before deleting
}

void MyFrame::OnPortSelect(wxCommandEvent& event) {
    wxString selectedPort = m_portChoice->GetStringSelection().ToStdString();  // Get the selected port as std::string

    // If a port is already selected, check if it's the same as the last one
    if (selectedPort == m_lastSelectedPort) {
        wxMessageBox("Port already selected!", "Info", wxOK | wxICON_INFORMATION);
        return;  // Do nothing if the same port is selected again
    }

    // If we are changing the port, close the previous one if it exists
    ClosePort();

    // Recreate the SerialPort object with the new selected port
    m_serialPort = new SerialPort(selectedPort.c_str(), 9600);

    if (!m_serialPort->isConnected()) {
        wxMessageBox("Failed to connect to the serial port!", "Error", wxOK | wxICON_ERROR);
        delete m_serialPort;
        m_serialPort = nullptr;
        return;
    }

    m_lastSelectedPort = selectedPort;  // Store the current port as the last selected port
    m_keepReading = true;
    m_readThread = std::thread(&MyFrame::StartFileReading, this);
}

void MyFrame::ClosePort() {
    if (m_serialPort) {
        delete m_serialPort;  // The destructor should handle cleanup
        m_serialPort = nullptr;
    }
}

void MyFrame::StartFileReading() {
    std::string lastValue = "";                // To keep track of the last sent value
    time_t lastModificationTime = 0;          // To store the last known modification time

    while (m_keepReading) {
        struct stat fileStat;
        if (stat("c:/Chirag/data.txt", &fileStat) == 0) { // Get file metadata
            if (fileStat.st_mtime != lastModificationTime) { // Check if file modification time has changed
                lastModificationTime = fileStat.st_mtime;   // Update the known modification time

                std::ifstream inputFile("c:/Chirag/data.txt");
                if (inputFile.is_open()) {
                    std::string value;
                    std::getline(inputFile, value);

                    if (m_serialPort && m_serialPort->isConnected() && (value == "1" || value == "0")) {
                        m_serialPort->writeSerialPort(value.c_str(), 1); // Send the value regardless
                        lastValue = value; // Update the last sent value
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Delay between checks
    }
}
