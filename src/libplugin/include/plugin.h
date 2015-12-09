#pragma once
#include <string>
#include <vector>

class Plugin;

class PluginHost
{
protected:
	std::vector<Plugin *> active_plugins;
	bool load_plugin(std::string filename);
};

class Plugin
{
public:
	virtual void init(PluginHost *h) = 0;
	virtual void deinit(PluginHost *h) = 0;
};
