#pragma once

#ifdef WIN32
	#ifndef BUILDING_PLUGIN
		#define PLUGINEXPORT __declspec(dllexport)
	#else
		#define PLUGINEXPORT __declspec(dllimport)
	#endif
	
	#define PLUGINCLASS class PLUGINEXPORT
	
	#define DLLEXPORT __declspec(dllexport)
#else
	#define PLUGINEXPORT
	#define DLLEXPORT
#endif