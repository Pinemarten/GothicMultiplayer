/*////////////////////////////////////////////////////////////////////////////

This file is part of the G2Ext SDK headers.

//////////////////////////////////////////////////////////////////////////////

The G2Ext SDK headers

Copyright © 2009, 2010 by Paindevs and Patrick Vogel

All Rights reserved.

THE WORK (AS DEFINED BELOW) IS PROVIDED
UNDER THE TERMS OF THIS CREATIVE COMMONS
PUBLIC LICENSE ("CCPL" OR "LICENSE").
THE WORK IS PROTECTED BY COPYRIGHT AND/OR
OTHER APPLICABLE LAW. ANY USE OF THE WORK
OTHER THAN AS AUTHORIZED UNDER THIS LICENSE
OR COPYRIGHT LAW IS PROHIBITED.

BY EXERCISING ANY RIGHTS TO THE WORK PROVIDED
HERE, YOU ACCEPT AND AGREE TO BE BOUND BY THE
TERMS OF THIS LICENSE. TO THE EXTENT THIS
LICENSE MAY BE CONSIDERED TO BE A CONTRACT,
THE LICENSOR GRANTS YOU THE RIGHTS CONTAINED
HERE IN CONSIDERATION OF YOUR ACCEPTANCE OF
SUCH TERMS AND CONDITIONS.

Full license at http://creativecommons.org/licenses/by-nc/3.0/legalcode

// Modified by Gothic Multiplayer Team (pampi, skejt23, mecio)

/////////////////////////////////////////////////////////////////////////////*/

#pragma once

#include "ztypes.hpp"
#include "macros.hpp"

/** Gothic string class: zSTRING */
class zSTRING
{
private:
	char data[20];

public:
	//.text:00402AF0 ; public: __thiscall zSTRING::zSTRING(void)
	/** Constructor
	* @usable Ingame only
	*/
	zSTRING()
	{
		XCALL(0x00402AF0);
	};

	~zSTRING()
	{
		XCALL(0x401160);
	}

	//.text:004010C0 ; public: __thiscall zSTRING::zSTRING(char const *)
	/** Constructor
	* @param  lpStr text
	* @usable Ingame only
	*/
	zSTRING(const char* lpStr)
	{
		XCALL(0x004010C0);
	};

	//.text:00416500 ; public: __thiscall zSTRING::zSTRING(class zSTRING const &)
	/** Constructor
	* @param  zstr text
	* @usable Ingame only
	*/
	zSTRING(const class zSTRING & zstr)
	{
		XCALL(0x00416500);
	};

	//.text:007928D0 ; public: __thiscall zSTRING::zSTRING(class zSTRING const *)
	/** Constructor
	* @param  zstr text
	* @usable Ingame only
	*/
	zSTRING(const class zSTRING* zstr)
	{
		XCALL(0x007928D0);
	};

	//.text:00435870 ; public: __thiscall zSTRING::zSTRING(int)
	zSTRING(int str)
	{
		XCALL(0x00435870);
	};

	//.text:0051AC80 ; public: __thiscall zSTRING::zSTRING(char)
	zSTRING(char str)
	{
		XCALL(0x0051AC80);
	};

	//.text:0067A7B0 ; public: class zSTRING & __thiscall zSTRING::operator+=(char const *)
	/** += operator for LPCSTRs
	* @usable Ingame only
	*/
	zSTRING & operator+=(LPCSTR p1)
	{
		XCALL(0x0067A7B0);
	};

	//.text:004CFAF0 ; public: class zSTRING & __thiscall zSTRING::operator=(char const *)
	/** = operator for LPCSTRs
	* @usable Ingame only
	*/
	zSTRING & operator=(LPCSTR p1)
	{
		XCALL(0x004CFAF0);
	};

	//.text:0059CEB0 ; public: class zSTRING & __thiscall zSTRING::operator=(class zSTRING const &)
	/** = operator for zSTRINGs
	* @usable Ingame only
	*/
	zSTRING & operator=(const zSTRING & p1)
	{
		XCALL(0x0059CEB0);
	};

	/** Insert description. 
	* @usable Ingame only
	*/
	zSTRING & operator+(const zSTRING & p1)
	{
		this->Insert(this->Length(), p1);
		return *this;
	};

	/** Insert description. 
	* @usable Ingame only
	*/
	zSTRING & operator+(const char* p1)
	{
		this->Insert(this->Length(), zSTRING(p1));
		return *this;
	};

	//.text:0059D010 ; public: void __thiscall zSTRING::Clear(void)
	/** This method clears the zSTRING
	* @usable Ingame only
	*/
	void Clear(void)
	{
		XCALL(0x0059D010);
	};

	//.text:0046BAA0 ; public: int __thiscall zSTRING::Delete(unsigned int, unsigned long)
	/** This method deletes a string in a specific range from the zSTRING
	* @param p1 marker begin
	* @param p2 marker end
	* @usable Ingame only
	*/
	void Delete(unsigned int pos1, unsigned long pos2)
	{
		XCALL(0x0046BAA0);	
	};

	//.text:0046B990 ; public: int __thiscall zSTRING::DeleteRight(unsigned int)
	/** This method deletes a string from the end of the zSTRING in reverse order
	* @param pos characters to delete
	* @usable Ingame only
	*/
	void DeleteRight(unsigned int pos)
	{
		XCALL(0x0046B990);
	};

	//.text:0046B400 ; public: int __thiscall zSTRING::Insert(unsigned int, class zSTRING const &)
	/** This method inserts a zSTRING into a zSTRING on a specific position 
	* @param pos
	* @param p2
	* @usable Ingame only
	*/
	int Insert(unsigned int pos, const zSTRING & p2)
	{
		XCALL(0x0046B400);
	};

	//.text:00674210 ; public: bool __thiscall zSTRING::IsEmpty(void)const
	/** This method returns whether the zSTRING is empty or not
	* @return true when zSTRING is empty - false when not
	* @usable Ingame only
	*/
	bool IsEmpty(void)
	{
		XCALL(0x00674210)	
	};

	//.text:0059D0E0 ; public: int __thiscall zSTRING::Length(void)const
	/** This method returns the length of the zSTRING
	* @return length of zSTRING
	* @usable Ingame only
	*/
	int Length(void)
	{
		XCALL(0x0059D0E0)
	};

	//.text:0046A9F0 ; public: class zSTRING & __thiscall zSTRING::Lower(void)
	/** Lowers all characters
	* @usable Ingame only
	*/
	zSTRING & Lower(void)
	{
		XCALL(0x0046A9F0)
	};

	//.text:0046B6A0 ; public: int __thiscall zSTRING::Overwrite(unsigned int, class zSTRING const &)
	/** This method overwrites a specific section of the zSTRING
	* @param pos position of overwrite
	* @param p2 string to overwrite with
	* @usable Ingame only
	*/
	int Overwrite(unsigned int pos, const zSTRING & p2)
	{
		XCALL(0x0046B6A0);
	};

	//.text:0059D110 ; public: int __thiscall zSTRING::Search(char const *, unsigned int)const
	/** This method searches a string in the zSTRING
	* @param  Search keyword
	* @param  Position to start the search from
	* @return Position of searched string. 0 if the string has not been found.
	* @usable Ingame only
	*/
	int Search(char const * p1, unsigned int p2 = 1)
	{
		XCALL(0x0059D110);
	};

	//.text:00492680 ; public: int __thiscall zSTRING::Search(class zSTRING const &, unsigned int)const
	/** This method searches a string in the zSTRING
	* @param  Search keyword
	* @param  Position to start the search from
	* @return Position of searched string. 0 if the string has not been found.
	*/
	int Search(class zSTRING const & p1, unsigned int p2 = 1)
	{
		XCALL(0x00492680);
	};

	//.text:0046CA20 ; public: int __thiscall zSTRING::SearchRev(class zSTRING const &, unsigned int)const
	/** This method searches a string in the zSTRING from the end of the string as start (reverse search)
	* @param  Search keyword
	* @param  Position to start the search from
	* @return Position of searched string. 0 if the string has not been found.
	*/
	int SearchRev(class zSTRING const & p1, unsigned int p2)
	{
		XCALL(0x0046CA20);	
	};

	//.text:0046AB00 ; public: class zSTRING & __thiscall zSTRING::Upper(void)
	/** This method makes the whole string upper case.
	* @usable Ingame only
	*/
	zSTRING & Upper(void)
	{
		XCALL(0x0046AB00);	
	};

   //.text:0046CC60 ; public: bool __cdecl zSTRING::Sprintf(char const *, ...)
	/** Sprintf?
	* @usable Ingame only
	*/
	bool Sprintf(const char* Str, ...)
	{
		XCALL(0x0046CC60);	
	};

	//.text:0046AC10 ; public: class zSTRING & __thiscall zSTRING::UpperFirstLowerRest(void)
	/** This method makes the first letter of the string upper case and the rest lower case
	* @usable Ingame only
	*/
	zSTRING & UpperFirstLowerRest(void)
	{
		XCALL(0x0046AC10);
	};

	//.text:004639D0 ; public: char * __thiscall zSTRING::ToChar(void)const
	/** This method converts the zSTRING to a char*
	* @return char* format string
	* @usable Ingame only
	*/
	const char* ToChar(void)
	{
		XCALL(0x004639D0);
	};

	//.text:004936C0 ; public: float __thiscall zSTRING::ToFloat(void)const
	/** This method converts the zSTRING to a float
	* @return float value
	* @usable Ingame only
	*/
	float ToFloat(void)
	{
		XCALL(0x004936C0);
	};

	//.text:0059D0F0 ; public: long __thiscall zSTRING::ToInt(void)const
	/** This method converts the zSTRING to an integer
	* @return int value
	* @usable Ingame only
	*/
	long ToInt(void)
	{
		XCALL(0x0059D0F0);
	};
};
