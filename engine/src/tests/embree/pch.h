/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfSource/Wolf.Engine/issues
	Website			 : https://WolfEngine.app
	Name			 : pch.h
	Description		 : The pre-compiled header
	Comment          :
*/

#ifndef __PCH_H__
#define __PCH_H__

#ifdef __WIN32

#include "w_target_ver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#endif

#include <wolf.h>
#include <w_window.h>

#endif
