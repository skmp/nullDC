//////////////////////////////////////////////////////////////////////////
vibThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)VibrationThread,
						 &purupuru_cond[0], 0, NULL);
InitializeCriticalSection(&vibSection);
//////////////////////////////////////////////////////////////////////////
TerminateThread(vibThread, 0);
CloseHandle(vibThread);
//////////////////////////////////////////////////////////////////////////
u32 tickCount;
u32 tickAutoStop;
u32 tickIncPeriod;
void* vibThread;
CRITICAL_SECTION vibSection;

// translate config to xinput values
void VibrationThread(void* condition);

// Currently slow as balls...TODO
void VibrationThread(void* condition)
{
	while (true)
	{
		EnterCriticalSection(&vibSection);

		purupuru_cond_t* cond = (purupuru_cond_t*)condition;
		// Left = low freq, right = high freq
		XINPUT_VIBRATION vib;
		vib.wLeftMotorSpeed = 0;
		vib.wRightMotorSpeed = 0;

		WORD intensity = 0;
		u16 intensityReal = 0;

		UVibConfig* config = &cond->config;

		LeaveCriticalSection(&vibSection);
		Sleep(1); 
		EnterCriticalSection(&vibSection);
		tickCount = GetTickCount();
		tickAutoStop = tickCount + (cond->AST * 250);
		u8 periodsPast = 0;
		u32 startTime = tickCount;
		u32 timeLength = tickAutoStop - startTime;
		if (config->INC)
			tickIncPeriod = timeLength / config->INC;

		if (config->CNT && tickCount < tickAutoStop)
		{
			if (config->Ppow)
				intensity = intensityReal = config->Ppow * (65535 / 7);
			else if (config->Mpow)
				intensity = intensityReal = config->Mpow * (65535 / 7);

			while (config->CNT && (intensity || periodsPast)) // The continuous mode loop
			{
				LeaveCriticalSection(&vibSection);
				Sleep(1); // Force kernel to release a timeslice
				EnterCriticalSection(&vibSection);
				tickCount = GetTickCount();
				if (tickCount > tickAutoStop)
				{
					config->CNT = 0; // break loop AND don't re-enter if
					intensity = 0;
				}

				if (config->INC && (config->INH || config->EXH) && tickCount >= ((tickIncPeriod * periodsPast) + startTime))
				{
					// it's time to handle con/divergence
					if (config->INH)
					{
						// diminishing
						if (periodsPast == config->INC - 1)
							intensity = 0;
						else
							intensity -= intensityReal / config->INC;
					}
					else
					{
						// rising
						if (periodsPast == 0)
							intensity = 0;
						else
							intensity += intensityReal / config->INC;
					}
					periodsPast++;

					if (periodsPast == config->INC)
						config->INC = 0;
					DEBUG_LOG("    %s intensity %i pPast %i\n", config->INH?"Convergence":"Divergence", intensity,periodsPast);
				}

				u8 fm = cond->srcSettings.FM1 - cond->srcSettings.FM0;
				if (config->Freq > (u8)((fm / 4)*3))	// top 1/4
					vib.wRightMotorSpeed = intensity;
				else if (config->Freq < (u8)(fm / 3))// low 1/3
					vib.wLeftMotorSpeed = intensity;
				else							// mid
				{
					vib.wRightMotorSpeed = intensity;
					vib.wLeftMotorSpeed = intensity;
				}
				// Actually set that crap! D:
				// !!! One hopes the xpad players are synonymous!
				XInputSetState(cont_conds[purupuru_cond->XPadPlayer].XPadPlayer, &vib);
			}
		}
		else if (config->Mpow || config->Ppow)
		{
			if (config->Ppow)
			{
				intensity = config->Ppow * (65535 / 7);
				config->Ppow = 0;
			}
			else
			{
				intensity = config->Mpow * (65535 / 7);
				config->Mpow = 0;
			}

			u8 fm = cond->srcSettings.FM1 - cond->srcSettings.FM0;
			if (config->Freq > (u8)((fm / 4)*3))	// top 1/4
				vib.wRightMotorSpeed = intensity;
			else if (config->Freq < (u8)(fm / 3))// low 1/3
				vib.wLeftMotorSpeed = intensity;
			else							// mid
			{
				vib.wRightMotorSpeed = intensity;
				vib.wLeftMotorSpeed = intensity;
			}
			XInputSetState(cont_conds[purupuru_cond->XPadPlayer].XPadPlayer, &vib);
			//Sleep(800); // x360pad spin up time (the motor is slow...) :(
		}
		else
		{
			vib.wLeftMotorSpeed = 0;
			vib.wRightMotorSpeed = 0;
			XInputSetState(cont_conds[purupuru_cond->XPadPlayer].XPadPlayer, &vib);
		}
		LeaveCriticalSection(&vibSection);
	}
}