
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

#include "CGmpServ.h"
#include "platform_depend.h"
#include "CCompression.h"
#include <cstdlib>
#include <ctime>

const char * pass = "yourpass";
unsigned char b[1024], c[1024];
const char *lolo="Chcialbym zjesc dobry obiad o 15.\0";

int main(int argc, char **argv)
{
	CCompression *comp=new CCompression();
	srand(static_cast<unsigned int>(time(NULL)));
	CGmpServ serv(pass, argc, argv);	//naszym głównym portem będzie "martwy" port
	if (!serv.Init())
	{
		puts("Server initialization failed!");
		exit(0);
	}
	while(1)
	{
		CGmpServ::sPacket p;
		do{
			continue;
		} while(serv.Receive(p));
		serv.DoRespawns();
		serv.SendSpamMessage();
		serv.RegenerationHPMP();
		serv.Run();
		/*if(serv.Receive(p) && p.type == CGmpServ::PT_MSG)
		{
			serv.Send(p.data);
			puts(p.data.c_str());
		}*/
		Sleep(1);
	}
	delete comp;
	return 0;
}
