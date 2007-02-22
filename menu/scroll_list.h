#ifndef BTANKS_MENU_SCROLL_LIST_H__
#define BTANKS_MENU_SCROLL_LIST_H__

#include "control.h"
#include "box.h"
#include <deque>
#include "sdlx/font.h"
#include "sdlx/rect.h"

class ScrollList : public Control {
public: 
	ScrollList(const int w, const int h);
	void add(const std::string &item);
	const size_t getPosition() const { return _current_item; }
	void setPosition(const size_t idx) { _current_item = idx; }
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private:
	Box _background;
	const sdlx::Surface *_scrollers;
	sdlx::Rect _up_area, _down_area, _items_area;
	const sdlx::Font *_font;
	int _item_h, _client_w, _client_h;

	typedef std::deque<std::string> List;
	List _list;

	float _pos, _vel;
	size_t _current_item;
};

#endif
