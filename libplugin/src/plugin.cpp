#include "plugin.h"
#include <dlfcn.h>

typedef void *(* plugin_callback)(void*); // plugin_callback;

Plugin* PluginHost::load_plugin(std::string filename)
{
	char* error = NULL;

	void *handle = dlopen(filename.c_str(), RTLD_LAZY | RTLD_NODELETE);
	if(handle == NULL)
	{
		return NULL;
	}

	plugin_callback fn;
	fn = (plugin_callback) dlsym(handle, "plugin_init");
	if ((error = dlerror()) != NULL)
	{
		dlclose(handle);
		return NULL;
	}

	Plugin *p = (Plugin*) fn(this);
	active_plugins[p->name()] = p;
	dlclose(handle);

	return p;
}

bool PluginHost::unload_plugin(std::string name)
{
	if(active_plugins.find(name) != active_plugins.end())
	{
		active_plugins[name]->deinit(this);
		active_plugins.erase(name);
	}
}