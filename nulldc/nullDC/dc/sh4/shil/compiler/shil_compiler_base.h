#include "types.h"
#include "dc\sh4\shil\shil.h"
#include "emitter\emitter.h"
#include "emitter\regalloc\x86_sseregalloc.h"

extern u32 T_jcond_value;
extern u32 reg_pc_temp_value;

void shil_compiler_init(x86_block* block,IntegerRegAllocator* ira,FloatRegAllocator* fra);
void shil_compile(shil_opcode* op);

struct roml_patch
{
	x86_Label* p4_access;
	u8 resume_offset;
	x86_Label* exit_point;
	u32 asz;
	u32 type;

	x86_reg reg_addr;
	x86_reg reg_data;
	bool is_float;
};

extern vector<roml_patch> roml_patch_list;
void apply_roml_patches();