#ifndef _timer_hpp_
#define _timer_hpp_

#include "../basic_types.h"
#include <windows.h>

class timer_if {
	public:
		
	timer_if() {
	}
	
	~timer_if() {
	}
	
	virtual u64 ticks()  {
		return 0U;
	}
	
	virtual u64 ticks_qp() {
		return 0U;
	}

	virtual f64 seconds() {
		return 0.0;
	}
	
	private:
};

class basic_timer_c: public timer_if {
	public:
	
	basic_timer_c() {
	}
	
	~basic_timer_c() {
	}
	
	u64 ticks()  {
		return timeGetTime();
	}
	
	u64 ticks_qp() {
		return timeGetTime();
	}

	f64 seconds() {
		const u64 r = this->ticks();
		return (0U == r) ? 0.0 : r / 1000.0;
	}
	
	private:
};

class cpu_timer_c: public timer_if {
	public:
	
	cpu_timer_c() {
	}
	
	~cpu_timer_c() {
	}
	
	u64 ticks()  {
		return __rdtsc();
	}
	
	u64 ticks_qp() {
		return __rdtsc();
	}

	f64 seconds() {
		const u64 r = this->ticks();
		return (0U == r) ? 0.0 : r / 1000.0;
	}
	
	private:
};

class high_frequency_timer_c: public timer_if {
	public:
	
	high_frequency_timer_c() {
		this->init();
	}
	
	~high_frequency_timer_c() {
	}
	
	u64 ticks()  {
		u64 r;
		if (!m_b_high_frq) {
			return GetTickCount();
		}

        QueryPerformanceCounter(&m_time_now);
		r = (m_time_now.QuadPart * 1000ULL);
        return  (r != 0U) ? r / m_frq.QuadPart : 0U;
	}
	
	u64 ticks_qp()  {
		if (!m_b_high_frq) {
			return GetTickCount();
		}

        QueryPerformanceCounter(&m_time_now);
        return  m_time_now.QuadPart;
	}

	f64 seconds() {
		const u64 r = this->ticks();
		return (0U == r) ? 0.0 : r / 1000.0;
	}
	
	private:
	inline void init() {
		m_b_high_frq = QueryPerformanceFrequency(&m_frq);
	}
	
	BOOL m_b_high_frq;
	LARGE_INTEGER m_frq;
	LARGE_INTEGER m_time_now;
};

#endif
