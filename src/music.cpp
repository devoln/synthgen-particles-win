#include "music.h"
#include "util.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <Windows.h>
#include <mmsystem.h>

#include <cmath>
#include <cstring>

enum {SAMPLE_RATE = 44100};


template<typename T> T powi(T x, int y)
{
	unsigned n = y > 0? y: -y;
	for(T z = T(1); ; x *= x)
	{
		if((n & 1) != 0) z *= x;
		if((n >>= 1) == 0) return (y < 0? T(1) / z: z);
	}
}

template<typename T> struct SineIterator
{
	SineIterator() = default;
	SineIterator(T amplitude, T dphi = 0):
		mS{0, amplitude*sinf(dphi)},
		mK(2*cosf(dphi)) {}

	void Next()
	{
		const T newS = mK * mS[1] - mS[0];
		mS[0] = mS[1];
		mS[1] = newS;
	}
	T Value() const {return mS[1];}

private:
	T mS[2], mK;
};

static unsigned randSeed = 3797834789;
inline unsigned urandom()
{
	return (randSeed *= 16807) >> 16;
}

static void sineWave(float* dst, float* dstEnd, float volume, float freq)
{
	SineIterator<float> siniter(volume, 6.283f/SAMPLE_RATE*freq);
	while(dst < dstEnd)
	{
		*dst++ += siniter.Value();
		siniter.Next();
	}
}

static void expAttenuation(float* dst, float* dstEnd, float coeff)
{
	coeff /= SAMPLE_RATE;
	float ek = 1.0f - coeff + coeff*coeff*0.5f;
	float expV = 1.0;
	while(dst < dstEnd)
	{
		*dst++ *= expV;
		expV *= ek;
	}
}

static void sineExpWave(float* dst, float* dstEnd, float volume, float freq, float coeff)
{
	SineIterator<float> siniter(volume, 6.283f/SAMPLE_RATE*freq);
	coeff /= SAMPLE_RATE;
	float ek = 1.0f - coeff + coeff*coeff*0.5f;
	float expV = 1.0;
	while(dst < dstEnd)
	{
		*dst++ += expV*siniter.Value();
		expV *= ek;
		siniter.Next();
	}
}

static void convertToShorts(short* dst, float* src, unsigned count)
{
	for(unsigned i = 0; i < count; i++)
		*dst++ += short(*src++ * 32767.0f);
}


static float temp[500000];

void GuitarNote(short* dst, float frequency, float duration)
{
	int sampleCount = int(duration*SAMPLE_RATE);

	static const float harmonicAmplitudes[15] = {
		0.24f, 0.0855f, 0.031f, 0.0015f, 0.018f,
		0.0325f, 0.041f, 0.0204f, 0.0035f, 0.011f,
		0.0236f, 0.01475f, 0.0006923f, 0.012f, 0.016f
	};

	static const float harmonicAttenuationCoeffs[15] = {
		1.7f, 2.85f, 4.0f, 5.15f, 6.3f,
		7.45f, 8.6f, 9.75f, 10.9f, 12.05f,
		13.2f, 14.35f, 15.5f, 16.65f, 17.8f
	};

	memset(temp, 0, sampleCount*sizeof(float));
	for(int h = 0; h < 15; h++)
		sineExpWave(temp, temp + sampleCount,
			harmonicAmplitudes[h] * 0.7f,
			frequency*(h + 1),
			harmonicAttenuationCoeffs[h] * 0.5f);
	convertToShorts(dst, temp, sampleCount);

	// Slower version producing smaller code
#if 0
	for(int h = 0; h < 15; h++)
	{
		memset(temp, 0, sampleCount*sizeof(float));
		sineWave(temp, temp + sampleCount, volumes[h]*0.7f, frequency*(h+1));
		expAttenuation(temp, temp+sampleCount, attenuations[h]*0.5f);
		convertToShorts(dst+h, temp, sampleCount - h);
	}
#endif
}

void VibraphoneNote(short* dst, float frequency, float duration)
{
	SineIterator<float> siniter(1.0f, 0.2f*6.283f/SAMPLE_RATE*frequency);
	int sampleCount = int(duration*SAMPLE_RATE);
	memset(temp, 0, sampleCount*sizeof(float));
	float freq = frequency;
	float vol = 0.3f;
	for(int octave = 0; octave < 5; octave++)
	{
		sineWave(temp, temp + sampleCount, vol, freq);
		freq *= 2;
		vol *= 0.5f;
	}
	expAttenuation(temp, temp+sampleCount, 5.0f - duration);
	convertToShorts(dst, temp, sampleCount);
}

struct Note
{
	signed char note;
	unsigned char offsetBeforeNextIn50ms;
};


void MusicSynth(short* dst, const signed char pattern[], unsigned patternLength)
{
	for(unsigned i = 0; i < patternLength; i++)
	{
		int n = pattern[i];
		float freq = 440.0f*powi(1.05946309f, n);
		GuitarNote(dst, freq, 1.1f);
		VibraphoneNote(dst+SAMPLE_RATE/3, freq, 1.0f);
		GuitarNote(dst+SAMPLE_RATE*2/3, freq, 1.5f);
		dst += SAMPLE_RATE;
	}
	GuitarNote(dst, 440.0f*powi(1.05946309f, 5), 2.0f);
	dst += SAMPLE_RATE;
	GuitarNote(dst, 440.0f*powi(1.05946309f, 3), 3.0f);
	dst += 3*SAMPLE_RATE;
}

struct ParamStruct {short* dst; const signed char* pattern; unsigned patternLength;};

DWORD __stdcall MusicSynthThreadFunc(void* pParams)
{
	auto& p = *static_cast<ParamStruct*>(pParams);
	MusicSynth(p.dst, p.pattern, p.patternLength);
	return 0;
}

constexpr signed char MusicPattern[]={
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
	unsigned char interval = 20;

	ThreadParams = {buf+22, MusicPattern, MusicPatternLength};
	CreateThread(nullptr, 65536, MusicSynthThreadFunc, &ThreadParams, 0, nullptr);

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
