#define OP_ON 1
#define OP_OFF 2

//TA stuff

//Rendering stuff :)
#define REND_D3D  1
#define REND_OGL  2
#define REND_SW   3
#define REND_D3D_V2 4
#define REND_NONE   5
#define REND_D3D11  6

#define REND_API REND_D3D


//Debugging stuff 
#define DO_VERIFY OP_OFF


//DO NOT EDIT -- overrides for default acording to build options
#ifdef _DEBUG
#undef DO_VERIFY
#define DO_VERIFY OP_ON
#endif