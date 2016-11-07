#include "music.h"
#include <math.h>
#include <string.h>

template<typename T> T pow(T x, int y)
{
	unsigned n = y>0? y: -y;
	for(T z = T(1); ; x*=x)
	{
		if((n & 1) != 0) z*=x;
		if((n >>= 1) == 0) return (y<0? T(1)/z: z);
	}
}

template<typename T> struct SineIterator
{
	SineIterator() = default;
	SineIterator(T amplitude, T dphi=0)
	{
		S[0] = 0;
		S[1] = amplitude*sinf(dphi);
		K = 2*cosf(dphi);
	}

	SineIterator& operator++() {iterate(); return *this;}
	T operator*() const { return S[1]; }

private:
	T S[2], K;

	void iterate() {const T newS=K*S[1]-S[0]; S[0]=S[1]; S[1]=newS;}
};

static unsigned randSeed=3797834789;
inline unsigned urandom()
{
	return (randSeed*=16807) >> 16;
}

void sine_wave(float* dst, float* dstEnd, float volume, float freq)
{
	SineIterator<float> siniter(volume, 6.283f/SAMPLE_RATE*freq);
	while(dst<dstEnd) *dst++ += *siniter, ++siniter;
}

void exp_attenuation(float* dst, float* dstEnd, float coeff)
{
	coeff /= SAMPLE_RATE;
	float ek = 1.0f - coeff + coeff*coeff*0.5f;
	float expV = 1.0;
	while(dst<dstEnd) *dst++ *= expV, expV*=ek;
}

void sine_exp_wave(float* dst, float* dstEnd, float volume, float freq, float coeff)
{
	SineIterator<float> siniter(volume, 6.283f/SAMPLE_RATE*freq);
	coeff /= SAMPLE_RATE;
	float ek = 1.0f - coeff + coeff*coeff*0.5f;
	float expV = 1.0;
	while(dst<dstEnd)
	{
		*dst++ += (*siniter)*expV;
		expV *= ek;
		++siniter;
	}
}

void convert_to_shorts(short* dst, float* src, unsigned count)
{
	for(unsigned i=0; i<count; i++)
		*dst++ += short(*src++ * 32767.0f);
}


float temp[500000];

//Создаёт буфер из суммы 15 затухающих синусов с разными громкостями и скоростями затухания
void GuitarNote(short* dst, float frequency, float duration)
{
	int sampleCount = int(duration*SAMPLE_RATE);

	//Амплитуды гармоник
	static const float volumes[15] = {0.24f, 0.0855f, 0.031f, 0.0015f, 0.018f,
		0.0325f, 0.041f, 0.0204f, 0.0035f, 0.011f, 0.0236f, 0.01475f, 0.0006923f, 0.012f, 0.016f};

	//Коэффициенты затухания гармоник
	static const float attenuations[15] = {1.7f, 2.85f, 4.0f, 5.15f,
		6.3f, 7.45f, 8.6f, 9.75f, 10.9f, 12.05f, 13.2f, 14.35f, 15.5f, 16.65f, 17.8f};

	memset(temp, 0, sampleCount*sizeof(float));
	for(int h=0; h<15; h++)
	{
		sine_exp_wave(temp+h, temp+sampleCount, volumes[h]*0.7f, frequency*(h+1), attenuations[h]*0.5f);
	}
	convert_to_shorts(dst, temp, sampleCount);

	//Не оптимизированная, но более компактная версия
#if 0
	for(int h=0; h<15; h++)
	{
		memset(temp, 0, sampleCount*sizeof(float));
		sine_wave(temp, temp+sampleCount, volumes[h]*0.7f, frequency*(h+1));
		exp_attenuation(temp, temp+sampleCount, attenuations[h]*0.5f);
		convert_to_shorts(dst+h, temp, sampleCount-h);
	}

#endif
}

void VibraphoneNote(short* dst, float frequency, float duration)
{
	SineIterator<float> siniter(1.0f, 6.283f/SAMPLE_RATE*frequency*0.2f);
	int sampleCount = (int)(duration*SAMPLE_RATE);
	memset(temp, 0, sampleCount*sizeof(float));
	float freq = frequency, vol=0.3f;
	for(int o=0; o<5; o++) sine_wave(temp, temp+sampleCount, vol, freq), freq*=2, vol*=0.5f;
	exp_attenuation(temp, temp+sampleCount, 5.0f-duration);
	convert_to_shorts(dst, temp, sampleCount);
}

struct Note
{
	signed char note;
	unsigned char offsetBeforeNextIn50ms;
};



#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <mmsystem.h>

void MusicSynth(short* dst, const signed char pattern[], unsigned patternLength)
{
	for(unsigned i=0; i<patternLength; i++)
	{
		int n = pattern[i];
		float freq = 440.0f*pow(1.05946309f, n);
		GuitarNote(dst, freq, 1.1f);
		VibraphoneNote(dst+SAMPLE_RATE/3, freq, 1.0f);
		//GuitarNote(dst+SAMPLE_RATE/3, freq, 1.0f);
		//VibraphoneNote(dst+SAMPLE_RATE*2, 440.0f*pow(1.05946309f, n), 1.2f);
		GuitarNote(dst+SAMPLE_RATE*2/3, freq, 1.5f);
		dst += SAMPLE_RATE;
	}
	GuitarNote(dst, 440.0f*pow(1.05946309f, 5), 2.0f);
	dst += SAMPLE_RATE;
	GuitarNote(dst, 440.0f*pow(1.05946309f, 3), 3.0f);
	dst += 3*SAMPLE_RATE;
}

struct ParamStruct {short* dst; const signed char* pattern; unsigned patternLength;};

DWORD __stdcall MusicSynthThreadFunc(void* pParams)
{
	auto& p = *(ParamStruct*)pParams;
	MusicSynth(p.dst, p.pattern, p.patternLength);
	return 0;
}

static const signed char MusicPattern[]={
	5, 3, 7, 8, 8, 7, 7, 3, 7, 3, -2, -9, -16, 2, 5, -5, 7, -16, 14,
	-4, 2, -9, -7, 10, 14, 10, -4, -4, -2, 12, 12, 8, -4, 0, 3, 8, -9, 7,
	-7, 14, 3, 5, 8, 3, -22, 0, 2, -9, -4, 0, -12, -9, -4, 2, 7, -9, -10,
	-5, -5, -5, -4, -5, -10, -9, -10, -16, -4, -10, 3, -2, -10, -4, 2, 5, 5,
	0, 2, -7, 2, -16, -9, 8, 10, -4, 5, 14, -12, 0, 2, 5, -9, 7, -2, -7, 10,
	7, -9, -14, 5, 8, 0, 7, -12, -2, 5, 7, -2, 2, 2, -2, 5
};

const unsigned MusicPatternLength = sizeof(MusicPattern)/sizeof(MusicPattern[0]);

static ParamStruct ThreadParams;

void MusicInit(short* buf)
{
	/*Note pattern[20];
	unsigned patternLength = 5+urandom()%10;
	for(unsigned i=0; i<patternLength; i++)
	{
		pattern[i] = {signed char(-12+(int)urandom()%18), unsigned char(20)};
	}*/

	unsigned char interval = 20;
	//Note pattern[]={{3, interval}, {5, interval}, {-3, interval}, {7, interval},
	  //  {2, interval}, {5, interval}, {7, interval}, {3, interval}/*,{1, interval},{1, interval}, {5, interval}, {8, interval}*/};



	ThreadParams = {buf+22, MusicPattern, MusicPatternLength};
	CreateThread(nullptr, 81920, MusicSynthThreadFunc, &ThreadParams, 0, nullptr);
	//MusicSynth(buf+22, pattern, patternLength);

	int sampleCount = MusicPatternLength*SAMPLE_RATE+SAMPLE_RATE*9/2;

	const int wavHeader[11] = {
		0x46464952,
		sampleCount*2+36,
		0x45564157,
		0x20746D66,
		16,
		WAVE_FORMAT_PCM|(1<<16),
		SAMPLE_RATE,
		SAMPLE_RATE*sizeof(short),
		(sizeof(short))|((8*sizeof(short))<<16),
		0x61746164,
		int(sampleCount*sizeof(short))
	};
	memcpy(buf, wavHeader, sizeof(wavHeader));
}

