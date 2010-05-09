--nullDC AICA plugin--

Basic emulation of aica:

--Chanel
All stream types implemented
Loop implemented
Pitch implemented
AEG not implemented
FEG not implemented
ALFO not implemented
PLFO not implemented

--Misc
Timers implemented
e68k implemented , needs check vs real hardware
AICA interrupt controller implemented
DSP not implemented



Aica is a powerfull audio generator:
64 chanels , spcm8/16,yapdcm/s,noise as input , with PAN/VOL settings with each chanel
16 channel - 128 step dsp w/ ring buffer (thats ~ 5.6M steps/second !)
16 or 18 bit DAC (selectable)

The output of each chanel are merged to the DSP input regs w/ volume control , and to the final mix regs , we/ pan and volume control
The output of each dsp chanel are merged w/ to the final mix regs w/ pan & volume control
The EXTS0 channels (2 , l/r , cdda) are merged to the final mix regs w/ pan & volume control


