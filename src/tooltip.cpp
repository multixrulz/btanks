#include "tooltip.h"
#include "mrt/logger.h"
#include <ctype.h>
#include "menu/box.h"
#include "resource_manager.h"
#include "sdlx/font.h"
#include <math.h>
#include <assert.h>
#include <deque>

Tooltip::Tooltip(const std::string &_text, const bool use_background, const int w)  : _use_background(use_background) {
	std::string text;
	bool space = true;
	size_t i;
	for(i = 0; i < _text.size(); ++i) {
		const int c = _text[i];
		const bool c_space = c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
		//LOG_DEBUG(("%d '%c': %s %s", c, c, space?"true":"false", c_space?"true":"false"));
		if (space) {
			if (c_space)
				continue;
			space = false;
			text += c;
		} else {
			if (c_space) {
				space = true;
				text += ' ';
			} else 
				text += c;
		}
	}
	
	//LOG_DEBUG(("trimmed string : '%s'", text.c_str()));
		
	std::vector<int> lens;
	std::vector<std::string> words;
	mrt::split(words, text, " ");
	lens.resize(words.size());

	std::string lens_dump;
	size_t sum = 0;
	for(i = 0; i < words.size(); ++i) {
		unsigned int l = words[i].size();
		lens[i] = l;
		sum += l;
		lens_dump += mrt::formatString("%s%u", (i == 0)?"":", ", l);
	}
//	LOG_DEBUG(("sum: %u, words: %s", sum, lens_dump.c_str()));

	int cell = (int)(sqrt(sum / 2.0) + 0.5);
	int xsize = cell * 2;
	//LOG_DEBUG(("approx size : %dx%d", xsize, cell * 3));

	const sdlx::Font *font = ResourceManager->loadFont("small", false);
	assert(font != NULL);

	int mx = 0, my = 0;
	if (_use_background) {
		_background.init("menu/background_box.png", 200, 60);
		_background.getMargins(mx, my);
	}
	
	int line_h = font->getHeight() + 2;

	int width = 0;
	std::deque<size_t> lines;

	for(i = 0; i < words.size(); ) {
		int l, line_size = 0;
		for(l = 0; (w != 0?line_size < w: l < xsize) && i < words.size(); l += lens[i], ++i) {
			int lw = font->render(NULL, 0, 0, words[i] + " ");
			if (w != 0 && lw + line_size > w) 
				break;
			line_size += lw;
		}
		if (line_size > width)
			width = line_size;
		lines.push_back(i);
	}


	//LOG_DEBUG(("line width: %d, lines: %u", width, lines.size()));
	if (_use_background) {
		_background.init("menu/background_box.png", width +  mx, line_h * lines.size() +  my);
		_surface.createRGB(_background.w, _background.h, SDL_SRCALPHA);
	} else {
		_surface.createRGB(w, line_h * (lines.size() + 2/*magic! */), SDL_SRCALPHA);
	}
	_surface.convertAlpha();
	
	int yp = my;
	i = 0;
	while(!lines.empty()) {
		int xp = mx;
		size_t n = lines.front();
		while(i < n) {
			xp += font->render(_surface, xp, yp, words[i] + " ");
			++i;
		} 
		yp += line_h;
		lines.pop_front();
	}
}

void Tooltip::render(sdlx::Surface &surface, const int x, const int y) {
	if (_use_background)
		_background.render(surface, x, y);
	surface.copyFrom(_surface, x, y);
}

void Tooltip::getSize(int &w, int &h) {
	w = _surface.getWidth();
	h = _surface.getHeight();
}
