#ifndef CORETEMPERATUREGTTPDAEMON_LIBRARY_H
#define CORETEMPERATUREGTTPDAEMON_LIBRARY_H

#include "CoreTempPlugin.h"
#include "microhttpd.h"
#include <cstdio>
#include "json/json.h"

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif


class CoreTemperatureHttpDaemon {
public:
    CoreTemperatureHttpDaemon(int port = 80);

    virtual ~CoreTemperatureHttpDaemon();
	
    int Start();

    int Configure();

    void Update(LPCoreTempSharedData data);

    void Stop();

    void Remove(const wchar_t *path);

    LPCoreTempPlugin GetPluginInstance(HMODULE hModule);


private:
    static uint16_t port;
    MHD_Daemon *httpDaemon;
    CoreTempPlugin plugin;
    CoreTempPluginInfo *pluginInfo;
    static LPCoreTempSharedData data;
	HWND dialog;
	HMODULE hModule;

	const char *configFile = "plugins\\CoreTempHttpDaemonPlugin\\port.cfg";

	static int httpHandler(void *cls,
		struct MHD_Connection *connection,
		const char *url,
		const char *method,
		const char *version,
		const char *upload_data,
		size_t *upload_data_size,
		void **con_cls);
	static INT_PTR CALLBACK dialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM pParam);
	void writePort();
	void readPort();
	void createDialog();

};

#endif