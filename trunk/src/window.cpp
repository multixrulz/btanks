
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "window.h"
#include "config.h"
#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "version.h"
#include "finder.h"
#include <stdlib.h>

#ifdef WIN32
#	define putenv _putenv
#endif

IMPLEMENT_SINGLETON(Window, IWindow);

#include <SDL/SDL_opengl.h>
#ifndef SDL_OPENGLBLIT
#define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#endif

IWindow::IWindow() : _fr(10.0f) {}

void IWindow::initSDL() {
	//putenv(strdup("SDL_VIDEO_WINDOW_POS"));
	putenv(strdup("SDL_VIDEO_CENTERED=1"));

	LOG_DEBUG(("gl: %s, vsync: %s, dx: %s", _opengl?"yes":"no", _vsync?"yes":"no", _dx?"yes":"no"));
#ifdef WIN32
	putenv(strdup("SDL_VIDEO_RENDERER=gdi"));

	if (_dx) 
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
		_putenv(strdup("SDL_VIDEO_RENDERER=d3d"));
#else
		_putenv(strdup("SDL_VIDEODRIVER=directx"));
#endif

#endif

//opengl renderer
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
	if (_opengl)
		_putenv(strdup("SDL_VIDEO_RENDERER=opengl"));
#endif

	LOG_DEBUG(("initializing SDL..."));
	Uint32 subsystems = SDL_INIT_VIDEO | (_init_timer?SDL_INIT_TIMER:0) | (_init_joystick?SDL_INIT_JOYSTICK:0);
#ifdef DEBUG
	sdlx::System::init(subsystems | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(subsystems);
#endif
	
	LOG_DEBUG(("enabling unicode..."));

	SDL_EnableUNICODE(1);

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

	}
	
	int default_flags = sdlx::Surface::Hardware | sdlx::Surface::Alpha | (_opengl? SDL_OPENGL: 0) ;
	if (_opengl) {
		default_flags &= ~SDL_OPENGL;
#ifdef USE_GLSDL
		default_flags |= SDL_GLSDL;
#endif
	}

	sdlx::Surface::setDefaultFlags(default_flags);

	//LOG_DEBUG(("initializing SDL_ttf..."));
	//sdlx::TTF::init();
}

void IWindow::init(const int argc, char *argv[]) {
#ifdef __linux__
//	putenv(strdup("SDL_VIDEODRIVER=dga"));
#endif
	_init_timer = true;
	_init_joystick = true;
	_opengl = true;
	
	_fullscreen = false;
	_vsync = false;
	_fsaa = 0;

	_dx = false;
	_force_soft = false;
	Config->get("engine.window.width", _w, 800);
	Config->get("engine.window.height", _h, 600);
	Config->get("engine.window.fullscreen", _fullscreen, false);
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--no-gl") == 0) _opengl = false;
		else if (strcmp(argv[i], "--fs") == 0) _fullscreen = true;
		else if (strcmp(argv[i], "--vsync") == 0) _vsync = true;
#ifdef WIN32
		else if (strcmp(argv[i], "--dx") == 0) { _dx = true; _opengl = false; }
#endif
		//else if (strcmp(argv[i], "--320x200") == 0) { _w = 320; _h = 200; }
		//else if (strcmp(argv[i], "--320x240") == 0) { _w = 320; _h = 240; }
		else if (strcmp(argv[i], "-0") == 0) { _w = 640; _h = 480; }
		else if (strcmp(argv[i], "-1") == 0) { _w = 800; _h = 600; }
		else if (strcmp(argv[i], "-2") == 0) { _w = 1024; _h = 768; }
		else if (strcmp(argv[i], "-3") == 0) { _w = 1152; _h = 864; }
		else if (strcmp(argv[i], "-4") == 0) { _w = 1280; _h = 1024; }
		else if (strcmp(argv[i], "--fsaa") == 0) { _fsaa = (_fsaa)?(_fsaa<< 1) : 1; }
		else if (strcmp(argv[i], "--force-soft-gl") == 0) { _force_soft = true; }
		else if (strcmp(argv[i], "--no-joystick") == 0) { _init_joystick = false; }
		else if (strcmp(argv[i], "--no-timer") == 0) { _init_timer = false; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(
					"\t--no-gl\t\t\tdisable GL renderer\n"
					"\t--dx\t\t\tenable directX(tm) renderer (win32 only)\n"
					"\t-2 -3\t\t\tenlarge video mode to 1024x768 or 1280x1024\n"
				  );
			exit(0);
		}
	}
	
	initSDL();

#if 0
#ifdef WIN32
	LOG_DEBUG(("loading icon..."));
	TRY {
		HANDLE h = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		if (h == NULL)
			throw_ex(("LoadImage failed"));
		ULONG r = SetClassLongPtr(NULL, GCLP_HICON, (LONG_PTR)h);
		LOG_DEBUG(("SetClassLongPtr returned %08lx", (unsigned long)r));

	} CATCH("icon setup", );
#endif
#endif

#ifndef WIN32
	std::string icon_file = Finder->find("tiles/icon.png", false);
	if (!icon_file.empty()) {
		TRY {
			sdlx::Surface icon;
			icon.loadImage(icon_file);
			SDL_WM_SetIcon(icon.getSDLSurface(), NULL);
		} CATCH("setting icon", {});
	}
#endif

	LOG_DEBUG(("setting caption..."));		
	SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");

	createMainWindow();
}

void IWindow::createMainWindow() {
	//Config->get("engine.window.width", _w, 800);
	//Config->get("engine.window.height", _h, 600);

	int flags = SDL_HWSURFACE | SDL_ANYFORMAT;
	flags |= SDL_DOUBLEBUF;
	
	if (_fullscreen) flags |= SDL_FULLSCREEN;

	if (_opengl) {
#if SDL_VERSION_ATLEAST(1,2,10)
		LOG_DEBUG(("setting GL swap control to %d...", _vsync?1:0));
		int r = SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, _vsync?1:0);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_SWAP_CONTROL."));
#ifdef WIN32
		if (!_vsync) {
			typedef void (APIENTRY * WGLSWAPINTERVALEXT) (int);
			WGLSWAPINTERVALEXT wglSwapIntervalEXT = (WGLSWAPINTERVALEXT) 
			wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT) {
				LOG_DEBUG(("disabling vsync with SwapIntervalEXT(0)..."));
			    wglSwapIntervalEXT(0); // disable vertical synchronisation
			}
		}
#endif

		LOG_DEBUG(("setting GL accelerated visual..."));

#ifdef WIN32
		//SIGSEGV in SDL under linux if no GLX visual present. (debian sid, fc6)
		r = SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_ACCELERATED_VISUAL."));
#endif

#endif
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		
		if (_fsaa > 0) {
			LOG_DEBUG(("fsaa mode: %d", _fsaa));
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, _fsaa);
		}
	
		//_window.setVideoMode(w, h, 0,  SDL_OPENGL | SDL_OPENGLBLIT | flags );
#ifdef USE_GLSDL
		flags |= SDL_GLSDL;
#endif

		_window.setVideoMode(_w, _h, 0, flags );

#if SDL_VERSION_ATLEAST(1,2,10)

		int accel = 0;
		if ((r = SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &accel)) == 0) {
			LOG_DEBUG(("SDL_GL_ACCELERATED_VISUAL = %d", accel));

		
			if (!_force_soft && accel != 1) {
				throw_ex(("Looks like you don't have a graphics card that is good enough.\n"
				"Please ensure that your graphics card supports OpenGL and the latest drivers are installed.\n" 
				"Try --force-soft-gl switch to enable sofware GL renderer."
				"Or use --no-gl to switch disable GL renderer completely."
				));
			}
		} else LOG_WARN(("SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL) failed: %s (%d)", SDL_GetError(), r));
#endif

	} else {
		_window.setVideoMode(_w, _h, 0, flags);
	}

	LOG_DEBUG(("created main surface. (%dx%dx%d, %s)", _w, _h, _window.getBPP(), ((_window.getFlags() & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));

	sdlx::System::probeVideoMode();	
#if 0
	{
		SDL_Rect **modes;
		int i;

		/* Get available fullscreen/hardware modes */
		modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

		/* Check is there are any modes available */
		if(modes == (SDL_Rect **)0) 
			throw_ex(("No video modes available"));
    
	    /* Check if our resolution is restricted */
    	if(modes == (SDL_Rect **)-1){
			LOG_DEBUG(("all resolutions available."));
		} else {
			/* Print valid modes */
			LOG_DEBUG(("available modes:"));
			for(i=0;modes[i];++i)
				LOG_DEBUG(("\t%dx%d", modes[i]->w, modes[i]->h));
		}
	}
#endif

	_running = true;
}

void IWindow::run() {

	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 120);
	
	_fr = fps_limit;
	int max_delay = 1000000 / fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	SDL_Event event;
	while (_running) {
		_timer.reset();
		
		while (SDL_PollEvent(&event)) {
			event_signal.emit(event);
		
			switch(event.type) {
			case SDL_JOYBUTTONDOWN:
				joy_button_signal.emit(event.jbutton.which, event.jbutton.button, event.jbutton.type == SDL_JOYBUTTONDOWN);
			break;
			
			case SDL_KEYUP:			
			case SDL_KEYDOWN:
				key_signal.emit(event.key.keysym, event.type == SDL_KEYDOWN);
			break;
			
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_signal.emit(event.button.button, event.button.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				break;
			
			case SDL_MOUSEMOTION:
				mouse_motion_signal.emit(event.motion.state, event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
				break;
			
		    //case SDL_QUIT:
			//	_running = false;
			//break;
    		}
		}
		
		const float dt = 1.0/_fr;
		
		tick_signal.emit(dt);
		
		IWindow::flip();

		int t_delta = _timer.microdelta();

		if (t_delta < max_delay) {
			//LOG_DEBUG(("tdelta: %d, delay: %d", t_delta, max_delay - t_delta));
			sdlx::Timer::microsleep(max_delay - t_delta);
		}

		t_delta = _timer.microdelta();
		_fr = (t_delta != 0)? (1000000.0 / t_delta): 1000000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));

}

void IWindow::deinit() {
	_running = false;
	LOG_DEBUG(("shutting down, freeing surface"));
	_window.free();
}

IWindow::~IWindow() {
	//_window.free();
}

void IWindow::flip() {
	_window.flip();
	if (_opengl) {
		//glFlush_ptr.call();
	}
}

void IWindow::resetTimer() {
	_timer.reset();
}
