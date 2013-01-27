#ifndef _algo_hpp_
#define _algo_hpp_

#include "../basic_types.h"

namespace algo {

	//Based on Varun Gupta's implementation | Optimizations+C++ impl : Div22
	//http://simplestcodings.blogspot.gr/2010/08/merge-sort-implementation-in-c.html
	
	//For non scalar/int/ptr
	template <typename base_t>
	class merge_sort2_c {
		public:
		merge_sort2_c() {
		}
		~merge_sort2_c() {
		}
		
		inline void sort(base_t* a,base_t* b,u32 len) {
			if (len < 2U) {
				return;
			}
			this->msrt(a,b,0U,len - 1U);
		}

		private:
		
		//TODO stackless impl w/ custom stack?
		void msrt(base_t* a,base_t* b,u32 lo,u32 hi) {
			if (lo >= hi) {
				return;
			}
			
			u32 avg = (lo + hi) >> 1;
			msrt(a,b,lo,avg);
			msrt(a,b,avg + 1,hi);
			merge(a,b,lo,avg,hi);
		}

		inline void merge(base_t* p_a,base_t* p_b,u32 lo,u32 piv,u32 hi) {
			register u32 h = lo,i = lo,j = piv + 1,k,t_hi = hi,t_piv = piv;
			register base_t* a = p_a;
			register base_t* b = p_b;
			
			while ((h<=t_piv) && (j<=t_hi)) {
				if (a[h] < a[j]) {
					b[i++] = a[h++];
					continue;
				}
	
				b[i++] = a[j++];
			}
			
			if (h > t_piv) {
				for(k=j; k <= t_hi;) {
					b[i++]=a[k++];
				}
			} else {
				for(k=h; k<=t_piv;) {
					b[i++]=a[k++];
				}
			}
	
			for (k=lo; k<=t_hi;) { 
				a[k]=b[k++];
			}
		}
	};

	template <typename base_t>
	class merge_sort_c {
		public:
		merge_sort_c() {
		}
		~merge_sort_c() {
		}
		
		inline void sort(base_t* a,base_t* b,u32 len) {
			if (len < 2U) {
				return;
			}
			this->msrt(a,b,0U,len - 1U);
		}

		private:
		
		//TODO stackless impl w/ custom stack?
		void msrt(base_t* a,base_t* b,u32 lo,u32 hi) {
			if (lo >= hi) {
				return;
			}
			
			u32 avg = (lo + hi) >> 1;
			msrt(a,b,lo,avg);
			msrt(aout,avg + 1,hi);
			merge(a,b,lo,avg,hi);
		}

		inline void merge(base_t* p_a,base_t* p_b,u32 lo,u32 piv,u32 hi) {
			register u32 h = lo,i = lo,j = piv + 1,k,t_hi = hi,t_piv = piv;
			register base_t* a = p_a;
			register base_t* b = p_b;
			
			while ((h<=t_piv) && (j<=t_hi)) {
				if (a[h] <= a[j]) {
					b[i++] = a[h++];
					continue;
				}
	
				b[i++] = a[j++];
			}
			
			if (h > t_piv) {
				for(k=j; k <= t_hi;) {
					b[i++]=a[k++];
				}
			} else {
				for(k=h; k<=t_piv;) {
					b[i++]=a[k++];
				}
			}
	
			for (k=lo; k<=t_hi;) { 
				a[k]=b[k++];
			}
		}
	};
	
};


#endif
