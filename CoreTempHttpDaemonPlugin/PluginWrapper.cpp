#include "CoreTemperatureHttpDaemon.h"

CoreTemperatureHttpDaemon *coreTemperatureHttpDaemon = nullptr;

int Start() {
	return coreTemperatureHttpDaemon->Start();
}

void Update(const LPCoreTempSharedData data) {
    coreTemperatureHttpDaemon->Update(data);
}

void Stop() {
    coreTemperatureHttpDaemon->Stop();
}

int Configure() {
	return coreTemperatureHttpDaemon->Configure();
}

void Remove(const wchar_t *path) {
    coreTemperatureHttpDaemon->Remove(path);
}

LPCoreTempPlugin WINAPI GetPlugin(HMODULE hModule) {
    coreTemperatureHttpDaemon = new CoreTemperatureHttpDaemon();
    LPCoreTempPlugin plugin = coreTemperatureHttpDaemon->GetPluginInstance(hModule);
    plugin->Start = Start;
    plugin->Update = Update;
    plugin->Stop = Stop;
    plugin->Configure = Configure;
    plugin->Remove = Remove;
    return plugin;
}


void WINAPI ReleasePlugin() {
    if (coreTemperatureHttpDaemon) {
        delete coreTemperatureHttpDaemon;
        coreTemperatureHttpDaemon = nullptr;
    }
}
