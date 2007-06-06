#include "campaign_menu.h"
#include "box.h"
#include "finder.h"
#include "i18n.h"
#include "chooser.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "scroll_list.h"
#include "mrt/directory.h"
#include "math/binary.h"
#include "menu.h"
#include "game_monitor.h"
#include "game.h"
#include "player_manager.h"
#include "window.h"
#include "player_slot.h"
#include "config.h"

void CampaignMenu::start() {
	int ci = _active_campaign->get();
	const Campaign &campaign = _campaigns[ci];
	std::string map = _maps->getValue();
	LOG_DEBUG(("campaign: %s, map: %s", campaign.name.c_str(), map.c_str()));
	//ensure world is created 
	Game->clear();
	GameMonitor->loadMap(campaign.name, map);
	
	PlayerSlot &slot = PlayerManager->getSlot(0);
	std::string cm;
	Config->get("player.control-method", cm, "keys");
	slot.createControlMethod(cm);

	std::string object, vehicle;
	PlayerManager->getDefaultVehicle(object, vehicle);

	slot.spawnPlayer(object, vehicle);
	PlayerManager->setViewport(0, Window->getSize());
	
	PlayerManager->startServer();	
}

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _w(w), _h(h) {
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty())
		return;

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));
	std::vector<std::string> titles;
	for(size_t i = 0; i < files.size(); ++i) {
		LOG_DEBUG(("campaign[%u]: %s", (unsigned)i, files[i].first.c_str()));
		Campaign c;
		c.base = files[i].first;
		c.init();
		_campaigns.push_back(c);
		titles.push_back(c.title);
	}

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
	int mx, my;
	b->getMargins(mx, my);

	int cw, ch;
	_active_campaign = new Chooser("medium", titles);
	_active_campaign->getSize(cw, ch);
	add(w / 2 - cw / 2, my, _active_campaign);

	int map_w = _w / 2;
	map_view = sdlx::Rect(mx * 2, my * 2 + ch, map_w, 3 * map_w / 4);
	
	_maps = new ScrollList("menu/background_box.png", "medium", w - map_view.w - 6 * mx, map_view.h );
	int sw, sh;
	_maps->getSize(sw, sh);
	add(w - sw - 2 * mx, map_view.y, _maps);
	
	init();
}

void CampaignMenu::init() {
	int ci = _active_campaign->get();
	const Campaign &campaign = _campaigns[ci];
	_maps->clear();
	for(size_t i = 0; i < campaign.maps.size(); ++i) {
		_maps->append(campaign.maps[i]);
	}
}

void CampaignMenu::tick(const float dt) {
	if (_active_campaign->changed()) {
		_active_campaign->reset();
		init();
	}
	
	if (_maps->changed()) {
		_maps->reset();
		int ci = _active_campaign->get();
		const Campaign &campaign = _campaigns[ci];

		int mi = _maps->get();
		map_dst = campaign.maps_pos[mi].convert<float>();
	}
	
	v2<float> map_vel = map_dst - map_pos;
	if (map_vel.quick_length() < 1) {
		map_pos = map_dst;
	} else {
		map_vel.normalize();
		float dist = math::min(map_dst.distance(map_pos), dt * 200);
		map_pos += map_vel * dist;
	}
}
bool CampaignMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	switch(sym.sym) {
	case SDLK_RETURN: 
		start();
		return true;	
	case SDLK_ESCAPE:
		_parent->back();
		return true;	
	default: 
		return false;
	}
}

const bool CampaignMenu::empty() const {
	return _campaigns.empty();
}

void CampaignMenu::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	int ci = _active_campaign->get();
	//sdlx::Rect clip = surface.getClipRect();
	//surface.setClipRect(map_view);
	surface.copyFrom(*_campaigns[ci].map, sdlx::Rect((int)map_pos.x, (int)map_pos.y, map_view.w, map_view.h), map_view.x, map_view.y);
	//surface.setClipRect(clip);
}

Campaign::Campaign() : map(NULL) {}

void Campaign::init() {
	map = NULL;
	parseFile(base + "/campaign.xml");
	/*
	mrt::Directory dir;
	dir.open(base + "/maps");
	std::string fname;

	while(!(fname = dir.read()).empty()) {
		std::string map = fname;
		
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = fname.substr(0, fname.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		/ *
		MapScanner m;
		TRY {
			m.scan(path + "/" + fname);
		} CATCH("scanning map", {});
		const std::string &comments = I18n->has("maps/descriptions", map)?I18n->get("maps/descriptions", map): 
			I18n->get("maps/descriptions", "(default)");
		maps.push_back(MapList::value_type(path, map, comments, m.object_restriction, m.game_type, m.slots));
		* /
		maps.push_back(fname);
	}	
	dir.close();
	*/
}

void Campaign::start(const std::string &name, Attrs &attr) {
	if (name == "campaign") {
		if (attr["title"].empty())
			throw_ex(("campaign must have title attr"));
		this->name = attr["title"];
		title = I18n->get("campaign", this->name);
		if (attr["map"].empty())
			throw_ex(("campaign must have map attr"));
		map = ResourceManager->loadSurface(attr["map"]);
	} else if (name == "map") {
		if (attr["id"].empty())
			throw_ex(("map must have id attr"));
		if (attr["position"].empty())
			throw_ex(("map must have position attr"));
		LOG_DEBUG(("map: %s, if-won: '%s', if-lost: '%s'", name.c_str(), attr["if-won"].c_str(), attr["if-lost"].c_str()));

		v2<int> pos;
		pos.fromString(attr["position"]);

		maps.push_back(attr["id"]);
		maps_pos.push_back(pos);
	}
}

void Campaign::end(const std::string &name) {}
