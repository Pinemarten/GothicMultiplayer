
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

#include "CLog.h"

CLog::CLog(short mode, const char* file_log){
	output.open(file_log);
	this->output_type=mode;
	switch(mode){
		case OUT_HTML:
			output << "<html><head><title>G-MP Serv Log</title></head><body>";
			output.flush();
			break;
		case OUT_TXT:
		default:
			break;
	}
}

CLog::~CLog() {
	switch(this->output_type){
		case OUT_HTML:
			output << "</body></html>";
			output.flush();
			break;
		case OUT_TXT:
		default:
			break;
	}
	output.close();
}

void CLog::Write(int type, const char* msg){
	switch(this->output_type){
		case OUT_HTML:
			switch(type){
				case LOG_WARNING:
					output << "<font color=\x22yellow\x22>" << msg << "</font><br>";
					break;
				case LOG_ERROR:
					output << "<font color=\x22red\x22>" << msg << "</font><br>";
					break;
				case LOG_NORMAL:
				default:
					output << msg << "<br>";
					break;
			}
			break;
		default:
			output << msg << std::endl;
			break;
	}
	output.flush();
}
