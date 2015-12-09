#pragma once
#include <string>
#include <map>

class Plugin;

class PluginHost
{
public:
	Plugin* load_plugin(std::string filename);
	bool unload_plugin(std::string name);
protected:
	std::map<std::string, Plugin *> active_plugins;
};

class Plugin
{
public:
	virtual void init(PluginHost *h) = 0;
	virtual void deinit(PluginHost *h) = 0;
	virtual std::string name() = 0;
};
