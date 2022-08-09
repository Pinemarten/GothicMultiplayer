
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
zBitStream zrobiony na ostatnie potrzeby GMP
*/
#ifndef ZBITSTREAM
#define ZBITSTREAM
#define MAX_BS_SIZE 1280
#include <string>

class zBitStream{
private:
	int err;
	union{
		int WHOCARE;
		struct{ short it, size;};
	};
	unsigned char data[MAX_BS_SIZE];

	void wb(unsigned char d);
	unsigned char rb();
public:
	void operator<< (const char*);
	void operator<< (unsigned char);
	void operator<< (unsigned short);
	void operator<< (unsigned int);
	void operator<< (unsigned long);
	void operator<< (unsigned long long);
	void operator<< (char);
	void operator<< (short);
	void operator<< (int);
	void operator<< (long);
	void operator<< (long long);
	void operator<< (bool);
	void operator<< (float);
	void operator>> (unsigned char&);
	void operator>> (unsigned short&);
	void operator>> (unsigned int&);
	void operator>> (unsigned long&);
	void operator>> (unsigned long long&);
	void operator>> (char&);
	void operator>> (short&);
	void operator>> (int&);
	void operator>> (long&);
	void operator>> (long long&);
	void operator>> (bool&);
	void operator>> (float&);
	void operator>> (std::string&);

	/* Empty zBitStream */
	zBitStream();
	/* Make clean memory */
	~zBitStream();
	/* Compress bitstream - currently not working*/
	void Compress();
	/* Clean memory and reset bitstream position */
	void Clear();
	/* Decompress bitstream - currently not working */
	void Decompress();
	/* Returns length and set pointer to  data */
	unsigned int GetData(void*& ptr);
	/* Read bool */
	bool ReadBit();
	/* Go to begin */
	void Rewind();
	/* Write bool */
	void WriteBit(bool bit);
};
#endif