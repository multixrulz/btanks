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

const bool MapPicker::MapDesc::operator<(const MapPicker::MapDesc & other) const {
	if (base != other.base)
		return base < other.base;
	return name < other.name;	
} 

struct MapScanner : mrt::XMLParser {
	MapScanner() : slots(0) {}
	std::string object;
	int slots;
	std::string object_restriction;

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
		_maps.push_back(MapList::value_type(path, map, comments, m.object, m.slots));
	}	
	dir.close();

}

void MapPicker::tick(const float dt) {
	if (_index != _list->getPosition()) {
		_index = _list->getPosition();
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc );
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

	sdlx::Rect list_pos(0, 0, (w - 64)/3, h - 128);
	_list = NULL;
	TRY {
		_list = new ScrollList(list_pos.w, list_pos.h);
		for(MapList::const_iterator i = _maps.begin(); i != _maps.end(); ++i) {
			_list->add(i->name);
		}
		add(list_pos, _list);
		_list->setPosition(_index);
	} CATCH("MapPicker::ctor", {delete _list; throw; });

	sdlx::Rect map_pos(list_pos.w + 16, 0, (w - 64) / 3, h - 128);

	_details = NULL;	
	TRY {
		_details = new MapDetails(map_pos.w, map_pos.h);
		_details->set(_maps[_index].base, _maps[_index].name, _maps[_index].desc);
		add(map_pos, _details);
	} CATCH("MapPicker::ctor", {delete _details; throw; });


	sdlx::Rect pp_pos(map_pos.x + map_pos.w + 16, 0, w - map_pos.x - map_pos.w - 16, h - 128);
	_picker = NULL;
	TRY {
		_picker = new PlayerPicker(pp_pos.w, pp_pos.h, 2);
		add(pp_pos, _picker);
	} CATCH("PlayerPicker::ctor", {delete _picker; throw; });

}
