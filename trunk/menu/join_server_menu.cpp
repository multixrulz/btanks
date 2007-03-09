#include "join_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"
#include "host_list.h"
#include "map_details.h"
#include "prompt.h"
#include "text_control.h"
#include "player_manager.h"
#include "i18n.h"
#include "game.h"

JoinServerMenu::JoinServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	_back = new Button("big", "BACK");
	_add = new Button("big", "ADD");
	_scan = new Button("big", "SCAN");
	_join = new Button("big", "JOIN");
	_upper_box = new UpperBox(500, 80, false);
	_add_dialog = new Prompt(w / 2, 96, new HostTextControl("medium"));

	int bw, bh, xp;

	_back->getSize(bw, bh);
	add(xp = 32, h - 96, _back);
	xp += 16 + bw;
	
	_add->getSize(bw, bh);
	add(xp, h - 96, _add);
	xp += 16 + bw;

#ifndef RELEASE
	_scan->getSize(bw, bh);
	add(xp, h - 96, _scan);
#endif
	
	_join->getSize(bw, bh);
	add(w - 64 - bw, h - 96, _join);
	
	add((w - _upper_box->w) / 2, 32, _upper_box);

	sdlx::Rect list_pos(0, 128, (w - 64)/3 + 80, h - 256);

	_hosts = new HostList("multiplayer.recent-hosts", list_pos.w, list_pos.h);
	add(list_pos.x, list_pos.y, _hosts);
	
	sdlx::Rect map_pos(list_pos.x + list_pos.w + 16, 128, (w - 64) / 3, h - 256);

	_details = new MapDetails(map_pos.w, map_pos.h);
	add(map_pos.x, map_pos.y, _details);	
	
	_add_dialog->getSize(bw, bh);
	add(w / 3, (h - bh) / 2, _add_dialog);
	_add_dialog->hide();
}

void JoinServerMenu::join() {
	LOG_DEBUG(("join requested"));
	std::string host = _hosts->getValue();
	
	bool ok = true;
	TRY {
		PlayerManager->startClient(host);
	} CATCH("join", { 
		std::string msg = I18n->get("menu", "connection-failed");
		Game->displayMessage(msg, 1.5); 
		ok = false; 
	});
		
	if (_parent)	
		_parent->setActive(!ok);
}

void JoinServerMenu::tick(const float dt) {
	Container::tick(dt);
	if (_back->changed()) {
		LOG_DEBUG(("[back] clicked"));
		MenuConfig->save();
		_back->reset();
		_parent->back();
	}
	if (_add->changed()) {
		_add->reset();
		_add_dialog->hide(false);
	}
	if (_add_dialog->changed()) {
		_add_dialog->reset();
		_add_dialog->hide();
		if (!_add_dialog->get().empty())
			_hosts->add(_add_dialog->get());
		
		_add_dialog->set(std::string());
	}


	if (_scan->changed()) {
		_scan->reset();
		LOG_DEBUG(("scan"));
	}
	
	if (_join->changed()) {
		_join->reset();
		join();
	}

}

bool JoinServerMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch(sym.sym) {

	case SDLK_RETURN:
		join();
		return true;
	
	case SDLK_a: 
		_add_dialog->hide(false);
		return true;
	
	case SDLK_ESCAPE: 
		MenuConfig->save();
		_parent->back();
		return true;
	default: ;
	}
	return false;
}

JoinServerMenu::~JoinServerMenu() {}
