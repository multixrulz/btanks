#ifndef BTANKS_CAMPAIGN_H__
#define BTANKS_CAMPAIGN_H__

#include "mrt/xml.h"
#include "math/v2.h"

namespace sdlx {
	class Surface;
}

class Campaign : protected mrt::XMLParser {
public: 
	Campaign();
	std::string base, name, title;
	int minimal_score;
	
	const sdlx::Surface *map;
	
	struct Map {
		std::string id;
		std::string visible_if;
		const sdlx::Surface *map_frame;
		v2<int> position;
		
		bool no_medals;
		int time; 
		Map() : id(), visible_if(), map_frame(NULL), position(), no_medals(false), time(0) {}
	};
	
	struct ShopItem {
		std::string type, name, object, animation, pose;
		int amount, price, max_amount;
		float dir_speed;
		
		void validate();
		ShopItem() : amount(0), price(0), max_amount(0), dir_speed(0) {}
	};
	
	struct Medal {
		std::string id, tile;
	};
	
	std::vector<Map> maps;
	std::vector<ShopItem> wares;
	std::vector<Medal> medals;
	
	void init(const std::string &base, const std::string &file);
	const bool visible(const Map &map_id) const;
	const int getCash() const;

	const bool buy(ShopItem &item) const;
	const bool sell(ShopItem &item) const;
	
	const ShopItem * find(const std::string &name) const;
	void clearBonuses();
	
protected: 
	void getStatus(const std::string &map_id, bool &played, bool &won) const;

	void start(const std::string &name, Attrs &attr);
	void end(const std::string &name);
private: 
	bool _wares_section;
};

#endif
