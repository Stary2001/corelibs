#pragma once
#include <string>
#include <map>
#include "export.h"

class Plugin;

PLUGINCLASS PluginHost
{
public:
	Plugin* load_plugin(std::string filename);
	bool unload_plugin(std::string name);
protected:
	std::map<std::string, Plugin *> active_plugins;
};

PLUGINCLASS Plugin
{
public:
	virtual void init(PluginHost *h) = 0;
	virtual void deinit(PluginHost *h) = 0;
	virtual std::string name() = 0;
};
