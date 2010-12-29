#include "PuruPuru.h"

int GetStatusKey (int port, int type, int input)
{
	bool* key = keyboard_map;	

	int num = input & 0xFF;

	if((input>>24) & inKEY)
	{
		switch(type)
		{
			case AXIS:	  return key[num]? 32767:0;				
			case TRIGGER: return key[num]? 255:0;
			case DIGITAL: return key[num];				
		}
	}

	return 0;
}

int GetStateXInput (int port, int type, int input)
{				
	bool key[256] = {0};
	
	if(joysticks[port].keys)
		memcpy(key,keyboard_map,sizeof(key));
		
	XInputGetState( joysticks[port].ID, &xoyinfo[port].state );
	port = joysticks[port].ID;	

	int num = input & 0xFF;
	int mode = input >> 24;

	switch(type)
	{
	case AXIS:
		{
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

							if(axis < 0) return  -axis;
							else		 return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

							if(axis < 0) return  -axis;
							else		 return	 0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;

							if(axis > 0) return  axis;
							else		 return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;

							if(axis > 0) return  axis;
							else		 return	 0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT)
						return xoyinfo[port].state.Gamepad.bLeftTrigger * 128;
					else
						return xoyinfo[port].state.Gamepad.bRightTrigger * 128;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)?32767:0;
						}
					case X360_DOWN:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)?32767:0;
						}
					case X360_LEFT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)?32767:0;
						}
					case X360_RIGHT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)?32767:0;
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)?32767:0;
						}
					case X360_BACK:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)?32767:0;
						}
					case X360_LS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)?32767:0;
						}
					case X360_RS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)?32767:0;
						}
					case X360_LB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)?32767:0;
						}
					case X360_RB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)?32767:0;
						}
					case X360_A:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)?32767:0;
						}
					case X360_B:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)?32767:0;
						}
					case X360_X:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)?32767:0;
						}
					case X360_Y:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)?32767:0;
						}
					}
				}
			case inKEY:
				{					
					return (key[num])?32767:0;
				}
			}				
		}
	case TRIGGER:
		{
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;							

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

							if(axis < 0)
							{
								if(axis < -255) axis = -255;
								return  -axis;
							}
							else return   0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX/128;							

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY/128;

							if(axis > 0)
							{
								if(axis > 255) axis = 255;
								return  axis;
							}
							else return	 0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT)
						return xoyinfo[port].state.Gamepad.bLeftTrigger;
					else if(num == X360_RT)
						return xoyinfo[port].state.Gamepad.bRightTrigger;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)?255:0;
						}
					case X360_DOWN:
						{	
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)?255:0;
						}
					case X360_LEFT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)?255:0;
						}
					case X360_RIGHT:
						{							
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)?255:0;
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START)?255:0;
						}
					case X360_BACK:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)?255:0;
						}
					case X360_LS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)?255:0;
						}
					case X360_RS:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)?255:0;
						}
					case X360_LB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)?255:0;
						}
					case X360_RB:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)?255:0;
						}
					case X360_A:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A)?255:0;
						}
					case X360_B:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B)?255:0;
						}
					case X360_X:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X)?255:0;
						}
					case X360_Y:
						{
							return (xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)?255:0;
						}
					}
				}
			case inKEY:
				{
					return (key[num])?255:0;					
				}
			}												
		}				
	case DIGITAL:	
		{				
			switch(mode)
			{
			case inAXIS_0:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;
							return (axis < -22000)?1:0;							
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;
							return (axis < -22000)?1:0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;
							return (axis < -22000)?1:0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;
							return (axis < -22000)?1:0;
						}
					}
				}
			case inAXIS_1:
				{
					switch(num)
					{
					case X360_LX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLX;
							return (axis > 22000)?1:0;							
						}
					case X360_LY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbLY;
							return (axis > 22000)?1:0;
						}
					case X360_RX:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRX;
							return (axis > 22000)?1:0;
						}
					case X360_RY:
						{
							SHORT axis = xoyinfo[port].state.Gamepad.sThumbRY;
							return (axis > 22000)?1:0;
						}
					}
				}
			case inTRIGGER:
				{
					if(num == X360_LT) 
						return (xoyinfo[port].state.Gamepad.bLeftTrigger > 100)?1:0;
					else
						return (xoyinfo[port].state.Gamepad.bRightTrigger > 100)?1:0;
				}
			case inHAT:
				{
					switch(num)
					{
					case X360_UP:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
						}
					case X360_DOWN:
						{	
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						}
					case X360_LEFT:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						}
					case X360_RIGHT:
						{
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						}
					}
				}
			case inBUTTON:
				{
					switch(num)
					{	
					case X360_START:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
						}
					case X360_BACK:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
						}
					case X360_LS:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
						}
					case X360_RS:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
						}
					case X360_LB:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						}
					case X360_RB:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						}
					case X360_A:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
						}
					case X360_B:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
						}
					case X360_X:
						{						
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
						}
					case X360_Y:
						{							
							return( xoyinfo[port].state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
						}
					}
				}
			case inKEY:
				{
					return key[num];					
				}
			}					
		}
	}
	
	return 0;
}


// 22000 AXIS to BUTTON threshold.

int GetStateSDL (int port, int type, int input)
{							
	bool key[256] = {0};
	
	if(joysticks[port].keys)
		memcpy(key,keyboard_map,sizeof(key));
	
	port = joysticks[port].ID; // From Dreamcast port, to device port.

	int num = input & 0xFF;
	int mode = input >> 24;
	int currentHat = SDL_JoystickGetHat(joyinfo[port].joy, 0);	

	switch(type)
	{
	case AXIS:
		{
			switch(mode)
			{
			case inAXIS_0: 
				{
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num);

					if(axis < 0)  return -axis;						
					else		  return 0;					
				}
			case inAXIS_1: 
				{
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num);
											
					if(axis > 0)  return  axis;						
					else		  return 0;										
				}
			case inBUTTON:
				{
					if( SDL_JoystickGetButton(joyinfo[port].joy, num) )
						return 32767;
					else
						return 0;
				}
			case inHAT:
				{
					if(num & currentHat) return 32767;
					else				 return 0;	
				}
			case inKEY:
				{
					if(key[num]) return 32767;
					else		 return 0;
				}
			}				
		}
	case TRIGGER:
		{
			switch(mode)
			{
			case inAXIS_0: 
				{
					
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num)/128;										
							
					if(axis < 0)
					{
						if(axis < -255) axis = -255;						
						
						return -axis;
					}
					else return 0;
					
				}
			case inAXIS_1: 
				{
					
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num)/128;										
							
					if(axis > 0)
					{
						if(axis > 255) axis = 255;

						return  axis;
					}
					else return 0;
					
				}
			case inBUTTON: 
				{
					if( SDL_JoystickGetButton(joyinfo[port].joy, num) )
						return 255;
					else
						return 0;
				}
			case inHAT: 
				{
					if(num & currentHat) return 255;
					else				 return 0;	
				}
			case inKEY:
				{
					if(key[num]) return 255;
					else		 return 0;
				}
			}
		}				
	case DIGITAL:	
		{
			switch(mode)
			{
			case inAXIS_0: 
				{						
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num);
											
					if(axis <-22000) return 1;
					else             return 0;					
				}
			case inAXIS_1: 
				{						
					int axis = SDL_JoystickGetAxis(joyinfo[port].joy, num);
					
					if(axis > 22000) return 1;
					else		     return 0;										
				}
			case inBUTTON:
				{
					return SDL_JoystickGetButton(joyinfo[port].joy, num);
				}
			case inHAT: 
				{	
					if(num & currentHat) return 1;
					else				 return 0;						
				}
			case inKEY: 
				{
					return key[num];
				}
			}
		}
	}

	return 0;
}


int GetInputStatus(int port, int type, int input)
{		
	switch(joysticks[port].controllertype)
	{
	case CTL_TYPE_JOYSTICK_SDL: 
		{
			SDL_JoystickUpdate(); 
			return GetStateSDL(port, type, input);
		}

	case CTL_TYPE_JOYSTICK_XINPUT:
		{
			XInputGetState(port, &xoyinfo[port].state); 
			return GetStateXInput(port, type, input);
		}
	case CTL_TYPE_KEYBOARD:
		{
			return GetStatusKey(port, type, input);
		}
	}

	return 0;
}


void GetJoyStatus(int controller)
{	
#ifdef BUILD_NAOMI

	joystate[controller].control[CTLN_D_UP]	   = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_D_UP]);
	joystate[controller].control[CTLN_D_DOWN]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_D_DOWN]);	
	joystate[controller].control[CTLN_D_LEFT]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_D_LEFT]);
	joystate[controller].control[CTLN_D_RIGHT] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_D_RIGHT]);		
	
	joystate[controller].control[CTLN_BUTTON1] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON1]);
	joystate[controller].control[CTLN_BUTTON2] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON2]);
	joystate[controller].control[CTLN_BUTTON3] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON3]);
	joystate[controller].control[CTLN_BUTTON4] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON4]);
	joystate[controller].control[CTLN_BUTTON5] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON5]);
	joystate[controller].control[CTLN_BUTTON6] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_BUTTON6]);
	
	joystate[controller].control[CTLN_START] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_START]);
	joystate[controller].control[CTLN_COIN]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_COIN]);

	joystate[controller].control[CTLN_SERVICE1] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_SERVICE1]);
	joystate[controller].control[CTLN_SERVICE2] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_SERVICE2]);					

	joystate[controller].control[CTLN_TEST1] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_TEST1]);
	joystate[controller].control[CTLN_TEST2] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAPN_TEST2]);					
#else
	joystate[controller].control[CTL_MAIN_X] = 0;
	joystate[controller].control[CTL_MAIN_Y] = 0;

	joystate[controller].control[CTL_MAIN_X] -= GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_XL]);
	joystate[controller].control[CTL_MAIN_X] += GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_XR]);				
		
	joystate[controller].control[CTL_MAIN_Y] -= GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_YU]);		
	joystate[controller].control[CTL_MAIN_Y] += GetInputStatus(controller, AXIS, joysticks[controller].control[MAP_A_YD]);		

	joysticks[controller].halfpress = GetInputStatus(controller, DIGITAL, joysticks[controller].control[MAP_HALF]) != 0;

	joystate[controller].control[CTL_L_SHOULDER] = GetInputStatus(controller, TRIGGER, joysticks[controller].control[MAP_LT]);
	joystate[controller].control[CTL_R_SHOULDER] = GetInputStatus(controller, TRIGGER, joysticks[controller].control[MAP_RT]);
		
	if ( joysticks[controller].halfpress ) 
	{
		joystate[controller].control[CTL_L_SHOULDER] /= 2;
		joystate[controller].control[CTL_R_SHOULDER] /= 2;

		joystate[controller].control[CTL_MAIN_X] /= 2;
		joystate[controller].control[CTL_MAIN_Y] /= 2;
	}				
				
	joystate[controller].control[CTL_A_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_A]);
	joystate[controller].control[CTL_B_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_B]);
	joystate[controller].control[CTL_X_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_X]);
	joystate[controller].control[CTL_Y_BUTTON] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_Y]);
	joystate[controller].control[CTL_START]	   = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_START]);				

	joystate[controller].control[CTL_D_PAD_UP]	  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_UP]);
	joystate[controller].control[CTL_D_PAD_DOWN]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_DOWN]);	
	joystate[controller].control[CTL_D_PAD_LEFT]  = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_LEFT]);
	joystate[controller].control[CTL_D_PAD_RIGHT] = GetInputStatus(controller, DIGITAL,  joysticks[controller].control[MAP_D_RIGHT]);	
#endif
}

void Names2Control(int port)
{
	for(int i=0; i<16; i++)
	{
		wchar* temp = joysticks[port].names[i];
		int num = _wtoi(&temp[1]);

		if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_SDL)
		{
			bool plus = false;
			if(temp[0] == L'A' && temp[sizeof(temp)/2] == L'+') plus = true;

			switch(temp[0])
			{
			case L'A': 
				{						
					if(plus) joysticks[port].control[i] = (inAXIS_1<<24) + num;
					else	 joysticks[port].control[i] = (inAXIS_0<<24) + num;
				}break;

			case L'B':
				{
					joysticks[port].control[i] = (inBUTTON<<24) + num;
				}break;

			case L'H':
				{
					joysticks[port].control[i] = (inHAT<<24) + num;
				}break;
			}
		}
		else if(joysticks[port].controllertype == CTL_TYPE_JOYSTICK_XINPUT)
		{
			if(temp[0] == L'L' && temp[1] == L'X' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_LX;
			}
			else if(temp[0] == L'L' && temp[1] == L'X' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_LX;
			}
			else if(temp[0] == L'L' && temp[1] == L'Y' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_LY;
			}
			else if(temp[0] == L'L' && temp[1] == L'Y' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_LY;
			}
			else if(temp[0] == L'R' && temp[1] == L'X' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_RX;
			}
			else if(temp[0] == L'R' && temp[1] == L'X' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_RX;
			}
			else if(temp[0] == L'R' && temp[1] == L'Y' && temp[2] == '+')
			{
				joysticks[port].control[i] = (inAXIS_1<<24) + X360_RY;
			}
			else if(temp[0] == L'R' && temp[1] == L'Y' && temp[2] == '-')
			{
				joysticks[port].control[i] = (inAXIS_0<<24) + X360_RY;
			}
			else if(temp[0] == L'L' && temp[1] == L'T')
			{
				joysticks[port].control[i] = (inTRIGGER<<24) + X360_LT;
			}
			else if(temp[0] == L'R' && temp[1] == L'T')
			{
				joysticks[port].control[i] = (inTRIGGER<<24) + X360_RT;
			}
			else if(temp[0] == L'U' && temp[1] == L'P')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_UP;
			}
			else if(temp[0] == L'D' && temp[1] == L'O' && temp[2] == 'W')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_DOWN;
			}
			else if(temp[0] == L'L' && temp[1] == L'E' && temp[2] == 'F')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_LEFT;
			}
			else if(temp[0] == L'R' && temp[1] == L'I' && temp[2] == 'G')
			{
				joysticks[port].control[i] = (inHAT<<24) + X360_RIGHT;
			}
			else if(temp[0] == L'S' && temp[1] == L'T' && temp[2] == 'A')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_START;
			}
			else if(temp[0] == L'B' && temp[1] == L'A' && temp[2] == 'C')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_BACK;
			}
			else if(temp[0] == L'L' && temp[1] == L'S')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_LS;
			}
			else if(temp[0] == L'R' && temp[1] == L'S')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_RS;
			}
			else if(temp[0] == L'L' && temp[1] == L'B')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_LB;
			}
			else if(temp[0] == L'R' && temp[1] == L'B')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_RB;
			}
			else if(temp[0] == L'A' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_A;
			}
			else if(temp[0] == L'B' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_B;
			}
			else if(temp[0] == L'X' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_X;
			}
			else if(temp[0] == L'Y' && temp[1] == '.')
			{
				joysticks[port].control[i] = (inBUTTON<<24) + X360_Y;
			}			
		}

		if(temp[0]==L'K')
			joysticks[port].control[i] = (inKEY<<24) + num;
	}
}