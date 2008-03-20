#include <SDL.h>
#include <SDL_audio.h>
#include "context.h"
#include <string.h>
#include "sdl_ex.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "source.h"
#include <assert.h>
#include <math.h>
#include <map>
#include "locker.h"
#include "stream.h"

using namespace clunk;

Context::Context() : period_size(0), max_sources(8) {
}

void Context::callback(void *userdata, Uint8 *bstream, int len) {
	Context *self = (Context *)userdata;
	assert(self != NULL);
	Sint16 *stream = (Sint16*)bstream;
	TRY {
		self->process(stream, len);
	} CATCH("callback", )
}

void Context::process(Sint16 *stream, int size) {
	typedef std::multimap<const float, std::pair<v3<float>, Source *> > sources_type;
	sources_type sources;
	
	for(objects_type::iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		v3<float> base = o->position;
		std::set<Source *> & sset = o->sources;
		for(std::set<Source *>::iterator j = sset.begin(); j != sset.end(); ++j) {
			Source *s = *j;
			v3<float> position = base + s->delta_position - listener;
			float dist = position.length();
			if (sources.size() < max_sources) {
				sources.insert(sources_type::value_type(dist, std::pair<v3<float>, Source *>(position, s)));
			} else {
				if (sources.rbegin()->first <= dist) 
					continue;
				//sources.erase(sources.rbegin());
				sources.insert(sources_type::value_type(dist, std::pair<v3<float>, Source *>(position, s)));
			}
		}
	}
	std::vector<std::pair<v3<float>, Source *> > lsources;
	sources_type::iterator j = sources.begin();
	for(unsigned i = 0; i < max_sources && j != sources.end(); ++i, ++j) {
		LOG_DEBUG(("%u: source in %g", i, j->first));
		lsources.push_back(j->second);
	}
	sources.clear();

	unsigned n = size / spec.channels / 2;
	LOG_DEBUG(("generating %u samples", n));
	Sint16 *dst = stream;
	for(int i = 0; i < size / 2; ++i) {
		*dst++ = 0;
	}

	for(streams_type::iterator i = streams.begin(); i != streams.end();) {
		LOG_DEBUG(("processing stream %d", i->first));
		stream_info &stream_info = i->second;
		while ((int)stream_info.buffer.getSize() < size) {
			mrt::Chunk data;
			bool eos = stream_info.stream->read(data);
			stream_info.buffer.append(data);
			if (eos) {
				if (stream_info.loop) {
					stream_info.stream->rewind();
				} else {
					break;
				}
			}
		}
		int buf_size = stream_info.buffer.getSize();
		LOG_DEBUG(("buffered %d bytes", buf_size));
		if (buf_size == 0) {
			//all data buffered. continue;
			LOG_DEBUG(("stream %d finished. dropping.", i->first));
			TRY {
				delete stream_info.stream;
			} CATCH("mixing stream", );
			streams.erase(i++);
			continue;
		}
		
		if (buf_size >= size)
			buf_size = size;

		int sdl_v = floor(SDL_MIX_MAXVOLUME * stream_info.gain + 0.5f);
		SDL_MixAudio((Uint8 *)stream, (Uint8 *)stream_info.buffer.getPtr(), buf_size, sdl_v);
		
		++i;
	}
	
	mrt::Chunk buf;
	buf.setSize(size);
	
	for(unsigned i = 0; i < lsources.size(); ++i ) {
		v3<float> & position = lsources[i].first;
		Source * source = lsources[i].second;
		float volume = source->process(buf, spec.channels, position);
		if (volume <= 0)
			continue;
		int sdl_v = floor(SDL_MIX_MAXVOLUME * volume + 0.5f);
		LOG_DEBUG(("mixing source with volume %g (%d)", volume, sdl_v));
		SDL_MixAudio((Uint8 *)stream, (Uint8 *)buf.getPtr(), size, sdl_v);
	}
}


Object *Context::create_object() {
	AudioLocker l;
	Object *o = new Object(this);
	objects.insert(o);
	return o;
}

Sample *Context::create_sample() {
	AudioLocker l;
	return new Sample(this);
}


void Context::init(const int sample_rate, const Uint8 channels, int period_size) {
	SDL_AudioSpec src;
	memset(&src, 0, sizeof(src));
	src.freq = sample_rate;
	src.channels = channels;
	src.format = AUDIO_S16LSB;
	src.samples = period_size;
	src.callback = &Context::callback;
	src.userdata = (void *) this;
	
	this->period_size = period_size;
	
	if ( SDL_OpenAudio(&src, &spec) < 0 )
		throw_sdl(("SDL_OpenAudio(%d, %u, %d)", sample_rate, channels, period_size));
	if (src.format != AUDIO_S16LSB)
		throw_ex(("SDL_OpenAudio(%d, %u, %d) returned format %d", sample_rate, channels, period_size, spec.format));
	LOG_DEBUG(("opened audio device, sample rate: %d, period: %d", spec.freq, spec.samples));
//	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_PauseAudio(0);
}

void Context::delete_object(Object *o) {
	AudioLocker l;
	objects.erase(o);
}

void Context::deinit() {
	SDL_PauseAudio(1);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
	
Context::~Context() {
	deinit();
}


//MUSIC MIXER: 

void Context::play(const int id, Stream *stream, bool loop) {
	LOG_DEBUG(("play(%d, %p, %s)", id, (const void *)stream, loop?"'loop'":"'once'"));
	AudioLocker l;
	stream_info & stream_info = streams[id];
	delete stream_info.stream;
	stream_info.stream = stream;
	stream_info.loop = loop;
	stream_info.paused = false;
}

bool Context::playing(const int id) const {
	AudioLocker l;
	return streams.find(id) != streams.end();
}

void Context::pause(const int id) {
	AudioLocker l;
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	
	i->second.paused = !i->second.paused;
}

void Context::stop(const int id) {
	AudioLocker l;
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	
	TRY {
		delete i->second.stream;
	} CATCH(mrt::formatString("stop(%d)", id).c_str(), {
		streams.erase(i);
		throw;
	})
	streams.erase(i);
}

void Context::set_volume(const int id, float volume) {
	if (volume < 0)
		volume = 0;
	if (volume > 1)
		volume = 1;
		
	streams_type::iterator i = streams.find(id);
	if (i == streams.end())
		return;
	i->second.gain = volume;
}
