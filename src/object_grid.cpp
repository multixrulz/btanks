
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "object_grid.h"
#include "mrt/fmt.h"
#include "mrt/logger.h"
#include <algorithm>
#include "math/binary.h"
#include "config.h"

Grid::Grid() {}

void Grid::clear() {
	_grid.clear();
	_grid4.clear();
	_index.clear();
}

//#define NEW_COLLIDE
#define NEW_OLD_COLLIDE

void Grid::collide(std::set<int> &objects, const GridMatrix &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> start = area_pos / grid_size;
	v2<int> end = (area_pos + area_size - 1) / grid_size;

	for(int y = math::max(0, start.y); y <= math::min((int)grid.size() - 1, end.y); ++y) 
		for(int x = math::max(0, start.x); x <= math::min((int)grid[y].size() - 1, end.x); ++x) {
			const IDSet &set = grid[y][x];
			//std::set_union(objects.begin(), objects.end(), set.begin(), set.end(), 
			//	std::insert_iterator<std::set<int> > (objects, objects.begin()));
			//can cause infinite recursion under win32 :(
			for(IDSet::const_iterator i = set.begin(); i != set.end(); ++i) {
				objects.insert(*i);
			}
		}	
}

void Grid::collide(std::set<int> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> size = area_size / _grid_size;
	int n = size.x * size.y;
	GET_CONFIG_VALUE("engine.grid-1x-limit", int, limit, 16);
	if (n >= limit) {
		collide(objects, _grid4, _grid4_size, area_pos, area_size);	
	} else {
		collide(objects, _grid, _grid_size, area_pos, area_size);		
	}
}


void Grid::resize(GridMatrix &grid, const v2<int> &grid_size, const v2<int> &map_size) {
	v2<int> dim = (map_size - 1) / grid_size + 1;
	grid.resize(dim.y);
	for(int y = 0; y < dim.y; ++y) 
		grid[y].resize(dim.x);
}

void Grid::setSize(const v2<int> &size, const int step) {
	clear();
	_grid_size = v2<int>(step, step);
	resize(_grid, _grid_size, size);
	_grid4_size = v2<int>(step * 4, step * 4);
	resize(_grid4, _grid4_size, size);
}

void Grid::removeFromGrid(GridMatrix &grid, const v2<int> &grid_size, const int id, const Object &o) {
	v2<int> start = o.pos / grid_size;
	v2<int> end = (o.pos + o.size - 1) / grid_size;
	for(int y = math::max(0, start.y); y <= math::min((int)grid.size() - 1, end.y); ++y) 
		for(int x = math::max(0, start.x); x <= math::min((int)grid[y].size() - 1, end.x); ++x) {
			grid[y][x].erase(id);
		}
}

void Grid::update(GridMatrix &grid, const v2<int> &grid_size, const int id, const v2<int> &pos, const v2<int> &size) {
	//insert
	v2<int> start = pos / grid_size;
	v2<int> end = (pos + size - 1) / grid_size;
	//LOG_DEBUG(("updating %d (%d, %d) -> (%d, %d) (%d %d)", id, start.x, start.y, end.x, end.y, pos.x, pos.y));
	for(int y = math::max(0, start.y); y <= math::min((int)grid.size() - 1, end.y); ++y) 
		for(int x = math::max(0, start.x); x <= math::min((int)grid[y].size() - 1, end.x); ++x) {
				grid[y][x].insert(id);
		}

}


void Grid::update(const int id, const v2<int> &pos, const v2<int> &size) {
	Index::iterator i = _index.find(id);
	if (i != _index.end()) {
	//skip modification if grid coordinates
		if (pos / _grid_size == i->second.pos / _grid_size &&
			(pos + size - 1) / _grid_size == (i->second.pos + size - 1) / _grid_size) {
			return;	
		}
		

		removeFromGrid(_grid, _grid_size, id, i->second);
		removeFromGrid(_grid4, _grid4_size, id, i->second);
		i->second.pos = pos;
		i->second.size = size;
	} else 
		_index.insert(Index::value_type(id, Object(pos, size)));

	update(_grid, _grid_size, id, pos, size);
	update(_grid4, _grid4_size, id, pos, size);
}

void Grid::remove(const int id) {
	Index::iterator i = _index.find(id);
	if (i != _index.end()) {
		removeFromGrid(_grid, _grid_size, id, i->second);		
		removeFromGrid(_grid4, _grid4_size, id, i->second);		
		_index.erase(i);
	} 
}
