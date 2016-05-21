#pragma once
#include <string>
#include <map>
#include <functional>
#include "export.h"

class Plugin;

typedef std::function<Plugin*(std::string)> PluginCallback;

PLUGINCLASS PluginHost
{
public:
	Plugin* load_plugin(std::string filename);
	bool unload_plugin(std::string name);
	void add_extension_handler(std::string ext, PluginCallback p);
protected:
	std::map<std::string, Plugin *> active_plugins;
	std::map<std::string, PluginCallback> extensions;
};

PLUGINCLASS Plugin
{
public:
	virtual ~Plugin() {};
	virtual void init(PluginHost *h) = 0;
	virtual void deinit(PluginHost *h) = 0;
	virtual std::string name() = 0;
};
