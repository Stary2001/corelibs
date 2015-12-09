#include "plugin.h"
#include <dlfcn.h>

typedef void *(* plugin_callback)(void*); // plugin_callback;

bool PluginHost::load_plugin(std::string filename)
{
	char* error = NULL;

	void *handle = dlopen(filename.c_str(), RTLD_LAZY);
	if(handle == NULL)
	{
		return false;
	}

	plugin_callback fn;
	fn = (plugin_callback) dlsym(handle, "plugin_init");
	if ((error = dlerror()) != NULL)
	{
		dlclose(handle);
		return false;
	}
	Plugin *p = (Plugin*) fn(this);
	active_plugins.push_back(p);
	dlclose(handle);

	return true;
}