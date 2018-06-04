#include "CoreTemperatureHttpDaemon.h"
#include"resource.h"

#include <iostream>
#include <cstring>
#include <sstream>


#include <fstream>

using namespace std;
using namespace Json;

LPCoreTempSharedData CoreTemperatureHttpDaemon::data = nullptr;
uint16_t CoreTemperatureHttpDaemon::port = 80;

int CoreTemperatureHttpDaemon::Start() {
	httpDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, 
		CoreTemperatureHttpDaemon::port,
		nullptr, nullptr, 
		&CoreTemperatureHttpDaemon::httpHandler, nullptr,
		MHD_OPTION_END);
    if (httpDaemon) {
        return 0;
    } else {
        return -1;
    }
}

int CoreTemperatureHttpDaemon::Configure() {
	readPort();
	createDialog();
	writePort();
    return 233;
}

void CoreTemperatureHttpDaemon::Update(const LPCoreTempSharedData data) {
    CoreTemperatureHttpDaemon::data = data;
}

void CoreTemperatureHttpDaemon::Stop() {
    MHD_stop_daemon(httpDaemon);
}

void CoreTemperatureHttpDaemon::Remove(const wchar_t *path) {

}

int CoreTemperatureHttpDaemon::httpHandler(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
    MHD_Response *rsp;
    StreamWriterBuilder writerBuilder;
    auto writer = writerBuilder.newStreamWriter();
    Value root;
    ostringstream strOut;

    if (strcmp(method, "GET") != 0)
        return MHD_NO;

    *con_cls = nullptr;
    std::string str;
	
    if (strcasecmp(url, "/cpuinfo") == 0) {
        root["physicCPUs"] = data->uiCPUCnt;
        root["CorePerCPU"] = data->uiCoreCnt;
        root["CPUName"] = data->sCPUName;
        root["CPUSpeed"] = data->fCPUSpeed;
        root["CriticalTemperature"] = data->uiTjMax[0];
        writer->write(root, &strOut);
        str = strOut.str();
        rsp = MHD_create_response_from_buffer(str.size(), (void *) str.c_str(), MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(rsp, "Content-Type", "application/json; charset=utf-8");
    } else if (strcasecmp(url, "/temperature") == 0) {
        root["physicCPUs"] = data->uiCPUCnt;
        root["CorePerCPU"] = data->uiCoreCnt;
        Value tempArray;
        for (auto i = 0; i < data->uiCPUCnt; i++) {
            for (auto j = 0; j < data->uiCoreCnt; j++) {
                tempArray[i][j] = data->fTemp[i * data->uiCoreCnt + j];
            }
        }
        root["temperature"] = tempArray;
        tempArray.clear();
        for (auto i = 0; i < data->uiCPUCnt; i++) {
            for (auto j = 0; j < data->uiCoreCnt; j++) {
                tempArray[i][j] = data->uiLoad[i * data->uiCoreCnt + j];
            }
        }
        root["load"] = tempArray;
        writer->write(root, &strOut);
        str = strOut.str();
        rsp = MHD_create_response_from_buffer(str.size(), (void *) str.c_str(), MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(rsp, "Content-Type", "application/json; charset=utf-8");
    } else {//403
        return MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, nullptr);
    }
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, rsp);
    MHD_destroy_response(rsp);
    return ret;
}

void CoreTemperatureHttpDaemon::writePort() {
	ofstream f(configFile);
	if (f.fail()) {
		return;
	}
	f << CoreTemperatureHttpDaemon::port;
	f.close();
}

void CoreTemperatureHttpDaemon::readPort() {
	ifstream f(configFile);
	if (f.fail()) {
		return;
	}
	auto port = 0;
	f >> port;
	if (port) {
		CoreTemperatureHttpDaemon::port = port;
	}
	f.close();
}

INT_PTR CoreTemperatureHttpDaemon::dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM pParam) {
	LPWSTR portText;
	int port;
	switch (msg) {
	case WM_SHOWWINDOW:
		portText = new TCHAR[10];
		wsprintf(portText, TEXT("%d"), CoreTemperatureHttpDaemon::port);
		SetDlgItemText(hWnd, IDC_EDIT_PORT, portText);
		delete portText;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_OK:
			portText = new TCHAR[10];
			GetDlgItemText(hWnd, IDC_EDIT_PORT, portText, 9);
			port = stoi(portText);
			delete portText;
			if (port < 0 || port > 65535) {
				MessageBox(hWnd, TEXT("Bad Port Number!"), TEXT("Error"), 0);
				return TRUE;
			} else {
				MessageBox(hWnd, TEXT("Restart Plugin To Change Port"), TEXT("Note"), 0);
				CoreTemperatureHttpDaemon::port = port;
				EndDialog(hWnd, 0);
				return TRUE;
			}
			break;
		case ID_CANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
			break;
		default:
			break;
		}
	default:
		break;
	}
	return FALSE;
}

void CoreTemperatureHttpDaemon::createDialog() {
	DialogBox(hModule, MAKEINTRESOURCE(IDD_DIALOG_PORT), NULL, (DLGPROC)&CoreTemperatureHttpDaemon::dialogProc);
}

CoreTemperatureHttpDaemon::CoreTemperatureHttpDaemon(int port) {
	CoreTemperatureHttpDaemon::port = port;
    pluginInfo = new CoreTempPluginInfo();
    plugin.pluginInfo = pluginInfo;
    pluginInfo->name = L"CoreTemperatureHttpDaemon";
    pluginInfo->description =L"Plugin to get temperature from http";
    pluginInfo->version = L"1.0";
    // Interface version should be 1 for current plugin API.
    plugin.interfaceVersion = 1;
    plugin.type = General_Type;
}

CoreTemperatureHttpDaemon::~CoreTemperatureHttpDaemon() {
    delete pluginInfo;
}

LPCoreTempPlugin CoreTemperatureHttpDaemon::GetPluginInstance(HMODULE hModule) {
	this->hModule = hModule;
    return &plugin;
}
