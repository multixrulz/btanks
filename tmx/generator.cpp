#include "generator.h"
#include "layer.h"
#include "tileset.h"

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/fs_node.h"
#include "mrt/xml.h"

MapGenerator::MapGenerator() {}

void MapGenerator::fill(Layer *layer, const std::vector<std::string> &args) {
	if (args.size() < 2) 
		throw_ex(("fill command takes 2 arguments."));
	LOG_DEBUG(("type: %s, name: %s",args[0].c_str(), args[1].c_str()));
}

void MapGenerator::tileset(const std::string &fname, const int gid) {
	std::string name = getName(fname);
	std::string xml_name = getDescName(fname);
	LOG_DEBUG(("tileset: %s, gid: %d, description file: %s", name.c_str(), gid, xml_name.c_str()));
	first_gid[name] = gid;
	
	if (_tilesets.find(name) != _tilesets.end())
		return;
	
	if (!mrt::FSNode::exists(xml_name))
		return;
	
	Tileset *t = NULL;
	TRY {
		t = new Tileset;
		t->parseFile(xml_name);
		_tilesets.insert(Tilesets::value_type(name, t));
	} CATCH("parsing tileset descriptor", {delete t; throw;} );
}


void MapGenerator::exec(Layer *layer, const std::string &command, const std::string &value) {
	LOG_DEBUG(("executing command '%s'...", command.c_str()));
	std::vector<std::string> args;
	mrt::split(args, value, ":");
	
	if (command == "fill") 
		fill(layer, args);
	else throw_ex(("unknown command '%s'", command.c_str()));
}

void MapGenerator::clear() {
	first_gid.clear();
}

const std::string MapGenerator::getName(const std::string &fname) {
	size_t end = fname.rfind(".");
	if (end == fname.npos) 
		end = fname.size();
	
	size_t start = fname.rfind("/");
	start = (start == fname.npos) ? 0: start + 1;
	return fname.substr(start, end - start);
}

const std::string MapGenerator::getDescName(const std::string &fname) {
	size_t end = fname.rfind(".");
	if (end == fname.npos) 
		throw_ex(("invalid filename '%s' for tileset", fname.c_str()));
	
	return fname.substr(0, end) + ".xml";
}
