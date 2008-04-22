#include "video_control.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"
#include "mrt/chunk.h"
#include "config.h"

#define SMPEG_CHECK(f) do { const char * err; if ((err = SMPEG_error(mpeg)) != NULL) throw_ex(("%s: %s", f, err)); } while(0)

VideoControl::VideoControl(const std::string &base, const std::string &name) : 
base(base), name(name), mpeg(0), lock(SDL_CreateMutex()), active(false), started(false) //, updated(false)  
{
	if (lock == NULL)
		throw_sdl(("SDL_CreateMutex"));
	
	std::string fname = "maps/" + name + ".jpg";
	if (Finder->exists(base, fname)) {
		screenshot = ResourceManager->loadSurface("../" + fname);
	} else 
		screenshot = ResourceManager->loadSurface("../maps/null_video.png");

	GET_CONFIG_VALUE("engine.disable-video", bool, edv, false);
	if (edv)
		return;
	
	fname = "maps/" + name + ".mpg";
	if (Finder->exists(base, fname)) {
		{
			mrt::Chunk video_data;
			Finder->load(video_data, fname);
			LOG_DEBUG(("video file loaded (%u bytes)", (unsigned)video_data.getSize()));

			mpeg = SMPEG_new_data(video_data.getPtr(), video_data.getSize(), &mpeg_info, 0);
			if (mpeg == NULL)
				throw_sdl(("SMPEG_new_data: %s", SDL_GetError()));
		}

		shadow.createRGB(screenshot->getWidth(), screenshot->getHeight(), 24, SDL_SWSURFACE);
		shadow.fill(shadow.mapRGBA(0, 0, 255, 0));
		shadow.setAlpha(255, 0);

		frame.createRGB(screenshot->getWidth(), screenshot->getHeight(), 24, SDL_SWSURFACE);
		frame.fill(frame.mapRGBA(255, 255, 255, 255));
		frame.convertAlpha();

		LOG_DEBUG(("video file info: %dx%d, %.02g seconds", mpeg_info.width, mpeg_info.height, mpeg_info.total_time));

		SMPEG_enableaudio(mpeg, 0);
		SMPEG_CHECK("SMPEG_enableaudio");
		SMPEG_enablevideo(mpeg, 1);
		SMPEG_CHECK("SMPEG_enablevideo");
		
		SMPEG_setdisplay(mpeg, shadow.getSDLSurface(), lock, NULL); //update);
		SMPEG_CHECK("SMPEG_setdisplay");
		
		SMPEG_scaleXY(mpeg, screenshot->getWidth(), screenshot->getHeight());
		SMPEG_CHECK("SMPEG_scaleXY");

		checkStatus();
		//SMPEG_play(mpeg);
		//SMPEG_CHECK("SMPEG_play");
	}
}

void VideoControl::activate(const bool a) {
	active = a;
	checkStatus();
}

void VideoControl::checkStatus() {
	if (mpeg == NULL)
		return;
	
	switch(SMPEG_status(mpeg)) {
	case SMPEG_PLAYING: 
		if (!active) {
			assert(started);
			LOG_DEBUG(("calling SMPEG_pause"));
			SMPEG_pause(mpeg);
		}
		break;
	case SMPEG_STOPPED: 
		if (active) {
			if (!started) {
				LOG_DEBUG(("starting stream..."));
				SMPEG_play(mpeg);
				SMPEG_loop(mpeg, 1);
				started = true;
			} else {
				LOG_DEBUG(("calling SMPEG_pause"));
				SMPEG_pause(mpeg);
			}
		}
		break;
	case SMPEG_ERROR: 
		LOG_DEBUG(("SMPEG error: %s", SMPEG_error(mpeg)));
		SMPEG_delete(mpeg);
		mpeg = NULL;
		break;
	}
}

void VideoControl::tick(const float dt) {
	Control::tick(dt);
	if (mpeg == NULL || !active) 
		return;
		
	checkStatus();

	//if (updated) 
	{
		//LOG_DEBUG(("syncing frame with shadow"));
		SDL_mutexP(lock);
		try {
			frame.lock();
			shadow.lock();
			for(int y = 0; y  < frame.getHeight(); ++y) {
				for(int x = 0; x < frame.getWidth(); ++x) {
					Uint8 r, g, b, a;
					shadow.getRGBA(shadow.getPixel(x, y), r, g, b, a);
					if (a == 0) 
					frame.putPixel(x, y, frame.mapRGBA(r, g, b, 255));
				}
			}
			shadow.unlock();
			frame.unlock();
		} catch(...) {
			SDL_mutexV(lock);
			throw;
		}
		//updated = false;
		SDL_mutexV(lock);
	}
}

void VideoControl::render(sdlx::Surface &surface, const int x, const int y) const {
	if (mpeg == NULL || !active) {
		surface.copyFrom(*screenshot, x, y);
		return;
	}
	//int dx = (screenshot->getWidth() - mpeg_info.width) / 2;
	//int dy = (screenshot->getHeight() - mpeg_info.height) / 2;
	int dx = 0, dy = 0;
	//LOG_DEBUG(("render %d %d", dx, dy));
	
	surface.copyFrom(frame, x + dx, y + dy);
}

void VideoControl::getSize(int &w, int &h) const {
	w = screenshot->getWidth();
	h = screenshot->getHeight();
}

VideoControl::~VideoControl() {
	if (mpeg != NULL) {
		SMPEG_stop(mpeg);
		SMPEG_delete(mpeg);
	}
	SDL_DestroyMutex(lock);
}
