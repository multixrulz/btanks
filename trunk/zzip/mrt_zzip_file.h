#ifndef ZZIP_MRT_FILE_H__
#define ZZIP_MRT_FILE_H__

#include "mrt/base_file.h"
#include <zzip/zzip.h>

namespace zzip {
	class ZZIPAPI File : public mrt::BaseFile {
		const bool readLine(std::string &str, const size_t bufsize = 1024) const;

		File();

		virtual void open(const std::string &fname, const std::string &mode);
		virtual const bool opened() const;
	
		virtual int seek(long offset, int whence) const;
		virtual long tell() const;
		virtual void write(const mrt::Chunk &ch) const;

		virtual const off_t getSize() const;
		virtual const size_t read(void *buf, const size_t size) const;
		virtual void close();
	
		virtual const bool eof() const;

		~File();
	private: 
		ZZIP_FILE *_f;
	};
}

#endif
