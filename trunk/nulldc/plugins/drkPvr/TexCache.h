#pragma once
#include "drkPvr.h"
#include <unordered_map>
#include <list>

extern u8* vq_codebook;
extern u32 palette_index;
extern u32 palette_lut[1024];
extern bool pal_needs_update;
extern u32 pal_rev_256[4];
extern u32 pal_rev_16[64];
extern u32 _pal_rev_256[4];
extern u32 _pal_rev_16[64];

template <class tex_base_c>
union tex_cache_node_t {
	struct  {
		u32 addr;
		tex_base_c* data;
		tex_cache_node_t* l;
		tex_cache_node_t* r;
	};
	u8 _pad[sizeof(void*)*4];
};

//If highmem is set a lut of sizeof(ptr) * (1<<21)+1 is created to store temp pointers and turn find() into O(1) operation
//Idealy no allocation would be needed if a table of sizeof(tex cache entry) * (1<<21) was created instead (with the obvious drawbacks)
//DiV22

#define drkprv_high_mem
template <class tex_base_c>
class tex_cache_list_c {
	public:
	u32 textures; //lazy a$$;)
	

	tex_cache_list_c() : textures(0){
#ifndef drkprv_high_mem
		m_bitmap_range = (((1<<21) + 8) & (~7)) >> 3;
		m_bitmap = new u8[m_bitmap_range];
		verify(m_bitmap != 0);
		memset(m_bitmap,0,m_bitmap_range);
#else
		m_lut = new tex_cache_node_t<tex_base_c>*[(1 << 21) + 1];
		verify(m_lut != 0);

		for (u32 i = 0;i <= 1<<21;++i) {
			m_lut[i] = 0;
		}
#endif
	}

	~tex_cache_list_c() {
		this->cleanup();
#ifndef drkprv_high_mem
		delete[] m_bitmap;
#else
		delete[] m_lut;
#endif
	}

	inline void traverse(s32(*cb)(tex_base_c*)) {
		if (!cb) {
			return;
		} else if (0U == textures) {
			return;
		}
		//printf("%u\n",textures);

		for (register tex_cache_node_t<tex_base_c>* a = m_head;a != 0;) {
			tex_cache_node_t<tex_base_c>* t = a;
			a = a->r;
			s32 ret = cb(t->data);
			if (ret == 0) {
				this->remove(t);
			} 
		}
	}

	inline void traverse_inv(s32(*cb)(tex_base_c*)) {
		if (!cb) {
			return;
		} else if (0U == textures) {
			return;
		}
		//printf("%u\n",textures);
		for (register tex_cache_node_t<tex_base_c>* a = m_tail;a != 0;) {
			tex_cache_node_t<tex_base_c>* t = a;
			a = a->l;
			s32 ret = cb(t->data);
			if ( ret == 0) {
				this->remove(t);
			}
		}
	}

	inline tex_cache_node_t<tex_base_c>* find(u32 addr) {
#ifndef drkprv_high_mem
		register u32 t = addr;
		if (!bm_is_set(t)) {
			return 0;
		}

		for (register tex_cache_node_t<tex_base_c>* a = m_head;(a != 0);a = a->r) {
			if (a->addr == t) {
				if (m_head == a) {
					return a;
				}

				tex_base_c* tmp0 = m_head->data;
				u32 tmp1 = m_head->addr;
				m_head->data = a->data;
				m_head->addr = a->addr;
				a->data = tmp0;
				a->addr = tmp1;
				return m_head;
			}
		}
		return 0;
#else
		//printf("%u addr / %u R\n",addr,1 << 21);
		return (addr <= (1 << 21)) ? m_lut[addr] : 0;
#endif
	}

	inline void add(u32 addr,tex_base_c* texture) {
		u32 t = addr;
		tex_cache_node_t<tex_base_c>* node;

		node = new tex_cache_node_t<tex_base_c>;
		//verify(node);
		node->l = node->r = 0;
		node->data = texture;
		node->addr = t;
		++textures;

		if (0 == m_tail) {
			m_tail = m_head = node;
		} else {
			m_tail->r = node;
			node->l = m_tail;
			m_tail = node;
		}

#ifndef drkprv_high_mem
		bm_set(t);
#else
		if (t <= (1 << 21)) {
			m_lut[t] = node;
		}
#endif
	}

	inline void remove(tex_cache_node_t<tex_base_c>* node) {
		if ((0 == m_tail) || (!node)) {
			return;
		}

		register tex_cache_node_t<tex_base_c>* l = node->l;
		register tex_cache_node_t<tex_base_c>* r = node->r;

		if (l != 0) {
			if (r == 0) {
				l->r = 0;
				m_tail = l;
			} else {
				l->r = r;
				r->l = l;
			}
		} else if (r != 0) {
			r->l = 0;
			m_head = r;
		} else {
			m_tail = m_head = 0;
		}

#ifdef drkprv_high_mem
		if (node->addr <= (1 << 21)) {
			m_lut[node->addr] = 0;
		}
#endif
		delete node->data;
		delete node;

		--textures;
	}

	inline void cleanup() {
		if (0 == m_tail) {
			return;
		}

		for (register tex_cache_node_t<tex_base_c>* a = m_head;a != 0;) {
			tex_cache_node_t<tex_base_c>* t = a;
			a = a->r;
			this->remove(t);
		}
 
		textures = 0U;
		m_head = m_tail = 0;
#ifndef drkprv_high_mem
		memset(m_bitmap,0,m_bitmap_range);
#endif
	}

	private:
#ifndef drkprv_high_mem
	u32 m_bitmap_range;
	u8* m_bitmap;
#else
	tex_cache_node_t<tex_base_c>** m_lut;
#endif
	tex_cache_node_t<tex_base_c>* m_head;
	tex_cache_node_t<tex_base_c>* m_tail;

#ifndef drkprv_high_mem
	inline bool bm_is_set(u32 addr) {
		if ((addr >> 3) >= m_bitmap_range) {
			return false;
		}

		return !!(m_bitmap[addr >> 3] & (1 << (addr & 7)));
	}

	inline void bm_set(u32 addr) {
		if ((addr >> 3) >= m_bitmap_range) {
			return;
		} else if (bm_is_set(addr)) {
			return;
		}
		m_bitmap[addr >> 3] |= 1 << (addr & 7);
	}
#endif
};


//Pixel buffer class (realy helpfull ;) )
struct PixelBuffer
{
	u32* p_buffer_start;
	u32* p_current_line;
	u32* p_current_pixel;

	u32 pixels_per_line;

	void init(void* data,u32 ppl_bytes)
	{
		p_buffer_start=p_current_line=p_current_pixel=(u32*)data;
		pixels_per_line=ppl_bytes/sizeof(u32);
	}
	__forceinline void prel(u32 x,u32 value)
	{
		p_current_pixel[x]=value;
	}

	__forceinline void prel(u32 x,u32 y,u32 value)
	{
		p_current_pixel[y*pixels_per_line+x]=value;
	}

	__forceinline void rmovex(u32 value)
	{
		p_current_pixel+=value;
	}
	__forceinline void rmovey(u32 value)
	{
		p_current_line+=pixels_per_line*value;
		p_current_pixel=p_current_line;
	}
	__forceinline void amove(u32 x_m,u32 y_m)
	{
		//p_current_pixel=p_buffer_start;
		p_current_line=p_buffer_start+pixels_per_line*y_m;
		p_current_pixel=p_current_line + x_m;
	}
};

//Generic texture cache / texture format conevertion code / macros
extern u32 unpack_4_to_8_tw[16];
extern u32 unpack_5_to_8_tw[32];
extern u32 unpack_6_to_8_tw[64];

void palette_update();

const u32 unpack_1_to_8[2]={0,0xFF};

#define clamp(minv,maxv,x) min(maxv,max(minv,x))

#define ARGB8888(A,R,G,B) \
	PixelPacker::pack(A,R,G,B)

#define ARGB1555_PL( word )	\
	ARGB8888(unpack_1_to_8[(word>>15)&1],((word>>10) & 0x1F)<<3,	\
	((word>>5) & 0x1F)<<3,(word&0x1F)<<3)

#define ARGB565_PL( word )		\
	ARGB8888(0xFF,((word>>11) & 0x1F)<<3,	\
	((word>>5) & 0x3F)<<2,(word&0x1F)<<3)

#define ARGB4444_PL( word )	\
	ARGB8888( (word&0xF000)>>(12-4),(word&0xF00)>>(8-4),(word&0xF0)>>(4-4),(word&0xF)<<4 )


#define ARGB1555_TW( word )	\
	ARGB8888(unpack_1_to_8[(word>>15)&1],unpack_5_to_8_tw[(word>>10) & 0x1F],	\
	unpack_5_to_8_tw[(word>>5) & 0x1F],unpack_5_to_8_tw[word&0x1F])

#define ARGB565_TW( word )		\
	ARGB8888(0xFF,unpack_5_to_8_tw[(word>>11) & 0x1F],	\
	unpack_6_to_8_tw[(word>>5) & 0x3F],unpack_5_to_8_tw[word&0x1F])
//( 0xFF000000 | unpack_5_to_8[(word>>11) & 0x1F] | unpack_5_to_8[(word>>5) & 0x3F]<<8 | unpack_5_to_8[word&0x1F]<<16 )

#define ARGB4444_TW( word )	\
	ARGB8888( unpack_4_to_8_tw[(word&0xF000)>>(12)],unpack_4_to_8_tw[(word&0xF00)>>(8)],unpack_4_to_8_tw[(word&0xF0)>>(4)],unpack_4_to_8_tw[(word&0xF)] )

template<class PixelPacker>
__forceinline u32 YUV422(s32 Y,s32 Yu,s32 Yv)
{
	Yu-=128;
	Yv-=128;

	//s32 B = (76283*(Y - 16) + 132252*(Yu - 128))>>16;
	//s32 G = (76283*(Y - 16) - 53281 *(Yv - 128) - 25624*(Yu - 128))>>16;
	//s32 R = (76283*(Y - 16) + 104595*(Yv - 128))>>16;
	
	s32 R = Y + Yv*11/8;				// Y + (Yv-128) * (11/8) ?
	s32 G = Y - (Yu*11 + Yv*22)/32;		// Y - (Yu-128) * (11/8) * 0.25 - (Yv-128) * (11/8) * 0.5 ?
	s32 B = Y + Yu*110/64;				// Y + (Yu-128) * (11/8) * 1.25 ?

	return ARGB8888(255,clamp(0,255,R),clamp(0,255,G),clamp(0,255,B));
}

u32 fastcall twiddle_fast(u32 x,u32 y,u32 x_sz,u32 y_sz);
#define twop twiddle_fast

//pixel packers !
struct pp_dx
{
	__forceinline 
		static u32 pack(u8 A,u8 R,u8 G,u8 B)
	{
		return ( ((A)<<24) | ((R)<<16) | ((G)<<8) | ((B)<<0));
	}
};
struct pp_gl
{
	__forceinline 
		static u32 pack(u8 A,u8 R,u8 G,u8 B)
	{
		return ( ((A)<<24) | ((B)<<16) | ((G)<<8) | ((R)<<0));
	}
};

//pixel convertors !
#define pixelcvt_start(name,x,y) template<class PixelPacker> \
struct name \
{ \
	static const u32 xpp=x;\
	static const u32 ypp=y;	\
	__forceinline static void fastcall Convert(PixelBuffer* pb,u8* data) \
{

#define pixelcvt_end } }
#define pixelcvt_next(name,x,y) pixelcvt_end;  pixelcvt_start(name,x,y)
//Non twiddled
pixelcvt_start(conv565_PL,4,1)
{
	//convert 4x1 565 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,ARGB565_PL(p_in[0]));
	//1,0
	pb->prel(1,ARGB565_PL(p_in[1]));
	//2,0
	pb->prel(2,ARGB565_PL(p_in[2]));
	//3,0
	pb->prel(3,ARGB565_PL(p_in[3]));
}
pixelcvt_next(conv1555_PL,4,1)
{
	//convert 4x1 1555 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,ARGB1555_PL(p_in[0]));
	//1,0
	pb->prel(1,ARGB1555_PL(p_in[1]));
	//2,0
	pb->prel(2,ARGB1555_PL(p_in[2]));
	//3,0
	pb->prel(3,ARGB1555_PL(p_in[3]));
}
pixelcvt_next(conv4444_PL,4,1)
{
	//convert 4x1 4444 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,ARGB4444_PL(p_in[0]));
	//1,0
	pb->prel(1,ARGB4444_PL(p_in[1]));
	//2,0
	pb->prel(2,ARGB4444_PL(p_in[2]));
	//3,0
	pb->prel(3,ARGB4444_PL(p_in[3]));
}
pixelcvt_next(convYUV_PL,4,1)
{
	//convert 4x1 4444 to 4x1 8888
	u32* p_in=(u32*)data;


	s32 Y0 = (p_in[0]>>8) &255; //
	s32 Yu = (p_in[0]>>0) &255; //p_in[0]
	s32 Y1 = (p_in[0]>>24) &255; //p_in[3]
	s32 Yv = (p_in[0]>>16) &255; //p_in[2]

	//0,0
	pb->prel(0,YUV422<PixelPacker>(Y0,Yu,Yv));
	//1,0
	pb->prel(1,YUV422<PixelPacker>(Y1,Yu,Yv));

	//next 4 bytes
	p_in+=1;

	Y0 = (p_in[0]>>8) &255; //
	Yu = (p_in[0]>>0) &255; //p_in[0]
	Y1 = (p_in[0]>>24) &255; //p_in[3]
	Yv = (p_in[0]>>16) &255; //p_in[2]

	//0,0
	pb->prel(2,YUV422<PixelPacker>(Y0,Yu,Yv));
	//1,0
	pb->prel(3,YUV422<PixelPacker>(Y1,Yu,Yv));
}
pixelcvt_end;
//twiddled 
pixelcvt_start(conv565_TW,2,2)
{
	//convert 4x1 565 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,0,ARGB565_TW(p_in[0]));
	//0,1
	pb->prel(0,1,ARGB565_TW(p_in[1]));
	//1,0
	pb->prel(1,0,ARGB565_TW(p_in[2]));
	//1,1
	pb->prel(1,1,ARGB565_TW(p_in[3]));
}
pixelcvt_next(conv1555_TW,2,2)
{
	//convert 4x1 1555 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,0,ARGB1555_TW(p_in[0]));
	//0,1
	pb->prel(0,1,ARGB1555_TW(p_in[1]));
	//1,0
	pb->prel(1,0,ARGB1555_TW(p_in[2]));
	//1,1
	pb->prel(1,1,ARGB1555_TW(p_in[3]));
}
pixelcvt_next(conv4444_TW,2,2)
{
	//convert 4x1 4444 to 4x1 8888
	u16* p_in=(u16*)data;
	//0,0
	pb->prel(0,0,ARGB4444_TW(p_in[0]));
	//0,1
	pb->prel(0,1,ARGB4444_TW(p_in[1]));
	//1,0
	pb->prel(1,0,ARGB4444_TW(p_in[2]));
	//1,1
	pb->prel(1,1,ARGB4444_TW(p_in[3]));
}
pixelcvt_next(convYUV_TW,2,2)
{
	//convert 4x1 4444 to 4x1 8888
	u16* p_in=(u16*)data;


	s32 Y0 = (p_in[0]>>8) &255; //
	s32 Yu = (p_in[0]>>0) &255; //p_in[0]
	s32 Y1 = (p_in[2]>>8) &255; //p_in[3]
	s32 Yv = (p_in[2]>>0) &255; //p_in[2]

	//0,0
	pb->prel(0,0,YUV422<PixelPacker>(Y0,Yu,Yv));
	//1,0
	pb->prel(1,0,YUV422<PixelPacker>(Y1,Yu,Yv));

	//next 4 bytes
	//p_in+=2;

	Y0 = (p_in[1]>>8) &255; //
	Yu = (p_in[1]>>0) &255; //p_in[0]
	Y1 = (p_in[3]>>8) &255; //p_in[3]
	Yv = (p_in[3]>>0) &255; //p_in[2]

	//0,1
	pb->prel(0,1,YUV422<PixelPacker>(Y0,Yu,Yv));
	//1,1
	pb->prel(1,1,YUV422<PixelPacker>(Y1,Yu,Yv));
}
pixelcvt_end;

pixelcvt_start(convPAL4_TW,4,4)
{
	u8* p_in=(u8*)data;
	u32* pal=&palette_lut[palette_index];

	pb->prel(0,0,pal[p_in[0]&0xF]);
	pb->prel(0,1,pal[(p_in[0]>>4)&0xF]);p_in++;
	pb->prel(1,0,pal[p_in[0]&0xF]);
	pb->prel(1,1,pal[(p_in[0]>>4)&0xF]);p_in++;

	pb->prel(0,2,pal[p_in[0]&0xF]);
	pb->prel(0,3,pal[(p_in[0]>>4)&0xF]);p_in++;
	pb->prel(1,2,pal[p_in[0]&0xF]);
	pb->prel(1,3,pal[(p_in[0]>>4)&0xF]);p_in++;

	pb->prel(2,0,pal[p_in[0]&0xF]);
	pb->prel(2,1,pal[(p_in[0]>>4)&0xF]);p_in++;
	pb->prel(3,0,pal[p_in[0]&0xF]);
	pb->prel(3,1,pal[(p_in[0]>>4)&0xF]);p_in++;

	pb->prel(2,2,pal[p_in[0]&0xF]);
	pb->prel(2,3,pal[(p_in[0]>>4)&0xF]);p_in++;
	pb->prel(3,2,pal[p_in[0]&0xF]);
	pb->prel(3,3,pal[(p_in[0]>>4)&0xF]);p_in++;
}
pixelcvt_next(convPAL8_TW,2,4)
{
	u8* p_in=(u8*)data;
	u32* pal=&palette_lut[palette_index];

	pb->prel(0,0,pal[p_in[0]]);p_in++;
	pb->prel(0,1,pal[p_in[0]]);p_in++;
	pb->prel(1,0,pal[p_in[0]]);p_in++;
	pb->prel(1,1,pal[p_in[0]]);p_in++;

	pb->prel(0,2,pal[p_in[0]]);p_in++;
	pb->prel(0,3,pal[p_in[0]]);p_in++;
	pb->prel(1,2,pal[p_in[0]]);p_in++;
	pb->prel(1,3,pal[p_in[0]]);p_in++;
}
pixelcvt_next(convPAL4_X_TW,4,4)
{
	u8* p_in=(u8*)data;
	//20 -> 16 + 4 (*16 to fixup step size)
	//goes to red
	pb->prel(0,0,(p_in[0]&0xF)<<20);
	pb->prel(0,1,(p_in[0]&0xF0)<<16);p_in++;
	pb->prel(1,0,(p_in[0]&0xF)<<20);
	pb->prel(1,1,(p_in[0]&0xF0)<<16);p_in++;

	pb->prel(0,2,(p_in[0]&0xF)<<20);
	pb->prel(0,3,(p_in[0]&0xF0)<<16);p_in++;
	pb->prel(1,2,(p_in[0]&0xF)<<20);
	pb->prel(1,3,(p_in[0]&0xF0)<<16);p_in++;

	pb->prel(2,0,(p_in[0]&0xF)<<20);
	pb->prel(2,1,(p_in[0]&0xF0)<<16);p_in++;
	pb->prel(3,0,(p_in[0]&0xF)<<20);
	pb->prel(3,1,(p_in[0]&0xF0)<<16);p_in++;

	pb->prel(2,2,(p_in[0]&0xF)<<20);
	pb->prel(2,3,(p_in[0]&0xF0)<<16);p_in++;
	pb->prel(3,2,(p_in[0]&0xF)<<20);
	pb->prel(3,3,(p_in[0]&0xF0)<<16);p_in++;
}
pixelcvt_next(convPAL8_X_TW,2,4)
{
	u8* p_in=(u8*)data;
		//			4+16 (red) |       4+6 (2+8) green
#define COL ((p_in[0]&0xF)<<20) | ((p_in[0]&0xF0)<<6)
	pb->prel(0,0,COL);p_in++;
	pb->prel(0,1,COL);p_in++;
	pb->prel(1,0,COL);p_in++;
	pb->prel(1,1,COL);p_in++;

	pb->prel(0,2,COL);p_in++;
	pb->prel(0,3,COL);p_in++;
	pb->prel(1,2,COL);p_in++;
	pb->prel(1,3,COL);p_in++;
}
pixelcvt_end;
//hanlder functions
template<class PixelConvertor>
void fastcall texture_PL(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height) {
	u32 p=0;
	pb->amove(0,0);

	Height/=PixelConvertor::ypp;
	Width/=PixelConvertor::xpp;

	for (u32 y=0;y<Height;y++)
	{
		for (u32 x=0;x<Width;x++)
		{
			u8* p = p_in;
			////if ((u32)p>=0x07200000)
			//	p=(u8*)0x07100000;
			PixelConvertor::Convert(pb,p);
			p_in+=8;

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}
template <int bore_level>
void fastcall texture_PL_RAW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	u32 sz=Width*2;
	Height>>=1;	//half
	
	pb->amove(0,0);
	pb->pixels_per_line>>=1;	//half y res , since we use 16 bit color

	for (u32 y=0;y<Height;y++)
	{
		memcpy(pb->p_current_line,p_in,sz);
		p_in+=sz;
		pb->rmovey(1);
	}
}

template<class PixelConvertor>
void fastcall texture_TW(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	u32 p=0;
	pb->amove(0,0);

	const u32 divider=PixelConvertor::xpp*PixelConvertor::ypp;

	unsigned long bcx_,bcy_;
	_BitScanReverse(&bcx_,Width);
	_BitScanReverse(&bcy_,Height);
	const u32 bcx=bcx_-3;
	const u32 bcy=bcy_-3;

	for (u32 y=0;y<Height;y+=PixelConvertor::ypp)
	{
		for (u32 x=0;x<Width;x+=PixelConvertor::xpp)
		{
			u8* p = &p_in[(twop(x,y,bcx,bcy)/divider)<<3];
			PixelConvertor::Convert(pb,p);

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}

template<class PixelConvertor>
void fastcall texture_VQ(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height)
{
	p_in+=256*4*2;
	u32 p=0;
	pb->amove(0,0);

	const u32 divider=PixelConvertor::xpp*PixelConvertor::ypp;
	unsigned long bcx_,bcy_;
	_BitScanReverse(&bcx_,Width);
	_BitScanReverse(&bcy_,Height);
	const u32 bcx=bcx_-3;
	const u32 bcy=bcy_-3;

	for (u32 y=0;y<Height;y+=PixelConvertor::ypp)
	{
		for (u32 x=0;x<Width;x+=PixelConvertor::xpp)
		{
			u8 p = p_in[twop(x,y,bcx,bcy)/divider];
			PixelConvertor::Convert(pb,&vq_codebook[p*8]);

			pb->rmovex(PixelConvertor::xpp);
		}
		pb->rmovey(PixelConvertor::ypp);
	}
}

//We ask the compiler to generate the templates here
//;)
//planar formats !
template void fastcall texture_PL<conv4444_PL<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<conv565_PL<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<conv1555_PL<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_PL<convYUV_PL<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//twiddled formats !
template void fastcall texture_TW<conv4444_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<conv565_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<conv1555_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convYUV_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convPAL4_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_TW<convPAL8_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//VQ formats !
template void fastcall texture_VQ<conv4444_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<conv565_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<conv1555_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
template void fastcall texture_VQ<convYUV_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
//template void fastcall texture_VQ<convPAL4_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
//template void fastcall texture_VQ<convPAL8_TW<pp_dx> >(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);

//Planar
#define argb1555to8888 texture_PL<conv1555_PL<pp_dx> >
#define argb565to8888 texture_PL<conv565_PL<pp_dx> >
#define argb4444to8888 texture_PL<conv4444_PL<pp_dx> >
#define YUV422to8888 texture_PL<convYUV_PL<pp_dx> >
#define ANYtoRAW texture_PL_RAW<1>

//Twiddle
#define argb1555to8888_TW texture_TW<conv1555_TW<pp_dx> >
#define argb565to8888_TW texture_TW<conv565_TW<pp_dx> >
#define argb4444to8888_TW texture_TW<conv4444_TW<pp_dx> >
#define YUV422to8888_TW texture_TW<convYUV_TW<pp_dx> >
#define PAL4to8888_TW texture_TW<convPAL4_TW<pp_dx> >
#define PAL8to8888_TW  texture_TW<convPAL8_TW<pp_dx> >
#define PAL4toX444_TW texture_TW<convPAL4_X_TW<pp_dx> >
#define PAL8toX444_TW  texture_TW<convPAL8_X_TW<pp_dx> >

//VQ
#define argb1555to8888_VQ texture_VQ<conv1555_TW<pp_dx> >
#define argb565to8888_VQ texture_VQ<conv565_TW<pp_dx> >
#define argb4444to8888_VQ texture_VQ<conv4444_TW<pp_dx> >
#define YUV422to8888_VQ texture_VQ<convYUV_TW<pp_dx> >
//#define PAL4to8888_VQ texture_VQ<convPAL4_TW<pp_dx> >
//#define PAL8to8888_VQ  texture_VQ<convPAL8_TW<pp_dx> >

/*
void fastcall texture_VQ_argb565(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_argb1555(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_argb4444(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_YUV422(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_PAL4(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_VQ_PAL8(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
*/

void fastcall texture_PAL4(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);
void fastcall texture_PAL8(PixelBuffer* pb,u8* p_in,u32 Width,u32 Height);


