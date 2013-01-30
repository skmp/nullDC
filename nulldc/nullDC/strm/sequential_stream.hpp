#ifndef _sequential_stream_
#define _sequential_stream_
#include "../basic_types.h"
#include <vector>

//TODO buffered reads + 64bit strm + endianess test(Div22)
class sequential_stream_if {
	public:
	
	template <typename base_t>
	virtual base_t  get() {
		return (base_t)0;
	}

	virtual void get(void* p,const u32 l) {
	}
	
	template <typename base_t>
	virtual void put(const base_t v) {
	}
	
	virtual void put(const void* p,const u32 l) {
	}
	
	bool open(const std::string& fn) {
		return true;
	}
	
	bool flush(const std::string& fn) {
		return true; 
	}
	
	private:
};

class sequential_stream_writer_c: public sequential_stream_if {
	public:
	
	sequential_stream_writer_c() : m_strm(0){
	}
	
	~sequential_stream_writer_c() {
		flush();
	}
	
	template <typename base_t>
	base_t  get() {
		return (base_t)0;
	}
	
	void get(void* p,const u32 l) {
	}

	template <typename base_t>
	void put(const base_t v) {
		put((const void*)&v,sizeof(base_t));
	}
	
	void put(const void* p,const u32 l) {
		const u8* pp = (const u8*)p;
		const u32 k_min_buf_len = 32 * 1024;
		
		if (((u32)m_buf + l) >= k_min_buf_len) {
			fwrite((void*)&m_buf[0],1,m_buf.size(),m_strm);
			m_buf.clear();
		}
		
		for (u32 i = 0;i < l;++i) {
			m_buf.push_back(pp[i]);
		}
	}
	
	bool open(const std::string& fn) {
		if (m_strm != 0) {
			fclose(m_strm);
		}
		m_strm = fopen(fn.c_str(),"wb");
		return (0 != m_strm);
	}
	
	bool flush() {
		if (!m_strm) {
			return false;
		}
		if (!m_buf.empty()) {//Don't care;)
			fwrite((void*)&m_buf[0],1,m_buf.size(),m_strm);
		}
		
		fclose(m_strm);
		m_strm = 0;
		m_buf.clear();
		return true; 
	}
	private:
	FILE* m_fp;
	std::vector<u8> m_buf;
};


class sequential_stream_reader_c: public sequential_stream_if {
	public:
	
	sequential_stream_reader_c() m_strm(0) {
	}
	
	~sequential_stream_reader_c() {
		if (m_strm) {
			fclose(m_strm);
		}
	}
	
	template <typename base_t>
	base_t  get() {
		base_t tmp;
		get((void*)&tmp,sizeof(base_t));
		return tmp;
	}
	
	void get(void* p,const u32 l) {
		if (m_ptr + l > m_len) {
			return;
		}
		
		fread(p,1,l,m_strm); //TODO buffered reads`
	}

	template <typename base_t>
	void put(const base_t v) {
	}
	
	void put(const void* p,const u32 l) {
	}

	bool open(const std::string& fn) {
		if (m_strm != 0) {
			fclose(m_strm);
		}
		m_strm = fopen(fn.c_str(),"rb");
		m_ptr = m_len = 0;
		if (0 == m_strm) {
			return false;
		}
		fseek(m_strm,0,SEEK_END);
		m_len = ftell(m_strm);
		fseek(m_strm,0,SEEK_SET);
		
		if (0 == m_len) {
			fclose(m_strm);
			m_strm = 0;
		}
		return true;
	}
	
	bool flush(const std::string& fn) {
		return true; 
	}
	
	private:
	FILE* m_strm;
	u64 m_ptr,m_len;
};

#endif
