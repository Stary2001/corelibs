#include "plugin.h"

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef void *(* plugin_callback)(void*); // plugin_callback;

Plugin* PluginHost::load_plugin(std::string filename)
{
	char* error = NULL;

	HMODULE handle = LoadLibrary(filename.c_str());
	if(handle == NULL)
	{
		return NULL;
	}

	plugin_callback fn;
	fn = (plugin_callback) GetProcAddress(handle, "plugin_init");

	if (fn == NULL)
	{
		//CloseHandle(handle);
		return NULL;
	}

	Plugin *p = (Plugin*) fn(this);
	active_plugins[p->name()] = p;
	// CloseHandle(handle);

	return p;
}

#else

#include <dlfcn.h>

typedef void *(* plugin_callback)(void*); // plugin_callback;

Plugin* PluginHost::load_plugin(std::string filename)
{
	for(auto ext : extensions)
	{
		if(filename.substr(filename.length() - ext.first.length()) == ext.first)
		{
			return ext.second(filename);
		}
	}
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

#endif

bool PluginHost::unload_plugin(std::string name)
{
	if(active_plugins.find(name) != active_plugins.end())
	{
		active_plugins[name]->deinit(this);
		active_plugins.erase(name);
		return true;
	}
	else
	{
		return false;
	}
}

void PluginHost::add_extension_handler(std::string ext, PluginCallback p)
{
	extensions[ext] = p;
}
