#include "medals.h"
#include "box.h"
#include "game_monitor.h"
#include "resource_manager.h"
#include "campaign.h"
#include "label.h"
#include "tooltip.h"
#include <assert.h>

Medals::Medals(int w, int h) : campaign(NULL), active(0) {
	_modal = true;
	add(0, 0, new Box("menu/background_box_dark.png", w, h));
	title = new Label("big", std::string());
	add(0, 0, title);
	hint = NULL;
}

void Medals::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	const sdlx::Surface * s = tiles[active];
	assert(s != NULL);
	int w, h;
	get_size(w, h);
	surface.blit(*s, (w - s->get_width()) / 2, (h - s->get_height()) / 2);
}

void Medals::hide(const bool hide) {
	Container::hide(hide);
	if (hide) {
		if (campaign != NULL) {
			LOG_DEBUG(("unloading resources"));
			for(size_t i = 0; i < campaign->medals.size(); ++i) {
				ResourceManager->unload_surface(campaign->medals[i].tile);
			}
			tiles.clear();
		}
		return;
	}
	if (campaign == NULL) 
		throw_ex(("campaign == NULL"));

	tiles.resize(campaign->medals.size());
	for(size_t i = 0; i < tiles.size(); ++i) {
		tiles[i] = ResourceManager->load_surface(campaign->medals[i].tile);
	}
	update();
}

void Medals::update() {
	assert(campaign != NULL);
	
	int idx = active;
	if (idx >= (int)tiles.size()) {
		idx = tiles.size() - 1;
	}
	
	const Campaign::Medal &medal = campaign->medals[idx];
	title->set("campaign/medals", medal.id);
	
	invalidate();
}

bool Medals::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
	case SDLK_RETURN: 
		hide();
		return true;

	case SDLK_LEFT: 
		active -= 2;
	case SDLK_RIGHT: 
		++active;
		if (active < 0)
			active += tiles.size();
		if (active >= (int)tiles.size())
			active -= tiles.size();
		update();
		return true;
	default: 
		return true;
	}
}
