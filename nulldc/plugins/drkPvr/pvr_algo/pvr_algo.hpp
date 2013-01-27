

#ifndef _algo_hpp_
#define _algo_hpp_

#include "pvr_r.hpp"
#include "../../nulldc/basic_types.h"
#include "../../nulldc/algo/algo.hpp"
#include <vector>
#include <algorithm>

namespace pvr_algo {

	union poly_indice_pair_t {
		struct {
			u32 id;
			u32 gp;
			f32 z;
		};
		u8 _align[16];
	};
	
	class pvr_sort_c {
		public:
		pvr_sort_c() {
		}
		~pvr_sort_c() {
		}
		
		inline void sort(poly_indice_pair_t* a,poly_indice_pair_t* b,u32 len) {
			if (len < 2U) {
				return;
			}
			this->msrt(a,b,0U,len - 1U);
		}

		private:
		
		//TODO stackless impl w/ custom stack?
		void msrt(poly_indice_pair_t* p_a,poly_indice_pair_t* p_b,u32 lo,u32 hi) {
			if (lo >= hi) {
				return;
			}
			
			u32 avg = (lo + hi) >> 1;
			msrt(p_a,p_b,lo,avg);
			msrt(p_a,p_b,avg + 1,hi);
			merge(p_a,p_b,lo,avg,hi);
		}

		inline void merge(poly_indice_pair_t* p_a,poly_indice_pair_t* p_b,u32 lo,u32 piv,u32 hi) {
			register u32 h = lo,i = lo,j = piv + 1,k,t_hi = hi,t_piv = piv;
			register poly_indice_pair_t* a = p_a;
			register poly_indice_pair_t* b = p_b;
			
			while ((h<=t_piv) && (j<=t_hi)) {
				if (a[h].z <= a[j].z) {
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
	  
	
	struct pvr_state_node_t {
		std::vector<u64> children;
		
		pvr_state_node_t() {
			this->children.reserve(R_STATES);
			for (u32 i = 0;i < R_STATES;++i) {
				this->children.push_back( (u64)-1 );
			}
		}
		
		~pvr_state_node_t() {
		}
	};
	
	class pvr_state_manager_c {
		public:
		pvr_state_manager_c() {
			this->init();
		}
		~pvr_state_manager_c() {
		}
		
		inline bool cached(const u32 sampler,const u32 state,const u64 val) {
			//verify(sampler < k_max_pvr_samplers);
			//verify(state < R_STATES);
			if (this->m_nodes[sampler].children[state] != val) {
				this->m_nodes[sampler].children[state] = val;
				return false;
			}
			return true;
		}
		
		inline void translate(const u32 sampler,const u32 in_state,u64& out_state,u64& val) {
			out_state = k_r_states_tbl[in_state];
			val = this->m_nodes[sampler].children[in_state];
		}
		
		inline void translate(const u32 in_state,u64& out_state) {
			out_state = k_r_states_tbl[in_state];
		}
		
		inline void invalidate_sampler(const u32 sampler) {
			std::vector<u64>& v = m_nodes[sampler].children;
			for (u32 i = 0;i < R_STATES;++i) {
				v[i] = (u64)-1 ;
			}
		}
		private:
		std::vector<pvr_state_node_t> m_nodes;
		
		void init() {
			this->m_nodes.reserve(k_max_pvr_samplers);
	
			for (u32 i = 0;i < k_max_pvr_samplers;++i) {
				this->m_nodes.push_back(pvr_state_node_t());
			}
		}
	};
  
};


#endif
