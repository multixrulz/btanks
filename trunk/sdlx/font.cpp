#include "font.h"
#include "surface.h"

using namespace sdlx;

Font::Font() : _type(), _surface(NULL){}

Font::~Font() {
	clear();
}

void Font::clear() {
	if (_surface == NULL) 
		return;
	delete _surface;
	_surface = NULL;
}
	
void Font::load(const std::string &file, const Type type) {
	clear();
	_type = type;
	_surface = new sdlx::Surface;
	_surface->loadImage(file);
}

const int Font::render(sdlx::Surface &window, const int x, const int y, const std::string &str) const {
	int fw, fh;
	fw = fh = _surface->getHeight();

	for(unsigned i = 0; i < str.size(); ++i) {
		int c = str[i];
		
		switch(_type) {
		case AZ09:
			c -= '0';
			if (c > 9)
				c -= 7;
			if (c < 0 || c > 36) 	
				continue;
		break;
		}
		
		sdlx::Rect src(c * fw, 0, fw, fh);
		
		window.copyFrom(*_surface, src, x + i * fw, y);
	}
	return str.size() * fw;
}

