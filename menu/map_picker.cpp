#include "map_picker.h"
#include "map_details.h"
#include "player_picker.h"
#include "scroll_list.h"
#include "mrt/exception.h"
#include "mrt/directory.h"
#include "mrt/xml.h"
#include "config.h"
#include <algorithm>

#include "i18n.h"
#include "player_manager.h"
#include "map_desc.h"
#include "upper_box.h"
#include "game.h"
#include <assert.h>

struct MapScanner : mrt::XMLParser {
	MapScanner() : slots(0) {}
	std::string object;
	int slots;
	std::string object_restriction;
	std::string game_type;

	void scan(const std::string &name) {
		parseFile(name);
		LOG_DEBUG(("parser: slots: %d, object_restriction: '%s'", slots, object_restriction.c_str()));
	}
private: 
	virtual void start(const std::string &name, Attrs &attr) {
		if (name == "property") {
			if (attr["name"].substr(0, 6) == "spawn:")
				++slots;
			else if (attr["name"] == "config:multiplayer.restrict-start-vehicle" && attr["value"].substr(0, 7) == "string:") {
				object_restriction = attr["value"].substr(7);
			} else if (attr["name"] == "config:multiplayer.game-type" && attr["value"].substr(0, 7) == "string:") {
				game_type = attr["value"].substr(7);
			}
		}
	}
	virtual void end(const std::string &name) {}
//	virtual void charData(const std::string &data);
};


void MapPicker::scan(const std::string &path) {
	if (!mrt::Directory::exists(path))
		return;
	
	mrt::Directory dir;
	dir.open(path);
	std::string fname;

	while(!(fname = dir.read()).empty()) {
		std::string map = fname;
		
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = map.substr(0, map.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		MapScanner m;
		TRY {
			m.scan(path + "/" + fname);
		} CATCH("scanning map", {});
		const std::string &comments = I18n->has("maps/descriptions", map)?I18n->get("maps/descriptions", map): 
			I18n->get("maps/descriptions", "(default)");
		_maps.push_back(MapList::value_type(path, map, comments, m.object, m.game_type, m.slots));
	}	
	dir.close();

}

void MapPicker::tick(const float dt) {
	const MapDesc &map = getCurrentMap();
	_upper_box->value = map.game_type;

	if (_upper_box->changed() || _index != _list->getPosition()) {
		_upper_box->reset();
		_index = _list->getPosition();
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc );
		_picker->set(_maps[_index]);
	}
	Container::tick(dt);
}


MapPicker::MapPicker(const int w, const int h) : _index(0) {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

	scan(data_dir + "/maps");
	scan("private/" + data_dir + "/maps");
	
	if (_maps.empty())
		throw_ex(("no maps found. sorry. install some maps/reinstall game."));
		
	std::sort(_maps.begin(), _maps.end());
	
	std::string map;

	Config->get("menu.default-mp-map", map, "lenin_square");
	for(_index = 0; _index < _maps.size(); ++_index) {
		if (_maps[_index].name == map)
			break;
	}
	if (_index >= _maps.size())
		_index = 0;
	LOG_DEBUG(("map index: %d", _index));
	
	TRY {
		_upper_box = new UpperBox(500, 80, true);
		sdlx::Rect r((w - _upper_box->w) / 2, 32, _upper_box->w, _upper_box->h);
		add(r, _upper_box);
	} CATCH("StartServerMenu", {delete _upper_box; throw; });

	sdlx::Rect list_pos(0, 128, (w - 64)/3, h - 256);
	_list = NULL;
	TRY {
		_list = new ScrollList(list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			_list->add(i->name);
		}
		add(list_pos, _list);
		_list->setPosition(_index);
	} CATCH("MapPicker::ctor", {delete _list; throw; });

	sdlx::Rect map_pos(list_pos.w + 16, 128, (w - 64) / 3, h - 256);

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc);
		add(map_pos, _details);
	} CATCH("MapPicker::ctor", {delete _details; throw; });


	sdlx::Rect pp_pos(map_pos.x + map_pos.w + 16, 128, w - map_pos.x - map_pos.w - 16, h - 256);
	_picker = NULL;
	TRY {
		_picker = new PlayerPicker(pp_pos.w, pp_pos.h);
		_picker->set(_maps[_index]);
		add(pp_pos, _picker);
	} CATCH("PlayerPicker::ctor", {delete _picker; throw; });

}

void MapPicker::fillSlots() const {
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	if (!split) {	
		GET_CONFIG_VALUE("player.control-method", std::string, cm, "keys");

		std::string vehicle, animation;
		PlayerManager->getDefaultVehicle(vehicle, animation);
		int idx = PlayerManager->spawnPlayer(vehicle, animation, cm);
		assert(idx == 0);

		PlayerManager->setViewport(idx, Game->getSize());
	} else 
		throw_ex(("split screen mode is unimplemented"));
}
