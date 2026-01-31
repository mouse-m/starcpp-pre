/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.

*/

#ifndef STAR_ERROR_H
#define STAR_ERROR_H

#include<exception>
#include<string>
#define ensure(expression)\
if(!bool(expression))\
throw runtime_error("Assertion \""#expression"\" failed")

namespace STAR_CPP {
	class StarError : public std::exception {
	private:
		std::string message;
		
	public:
		StarError(const std::string& msg) : message(msg) {}
		
		virtual const char* what() const noexcept override {
			return message.c_str();
		}
	};
	
	class Star_CantOpenFileError : public StarError {
	public:
		Star_CantOpenFileError(const std::string& filename, const std::string& what = "Cannot open file: ")
		: StarError(what + filename) {}
	};
	
	class Star_FileNotFoundError : public StarError {
	public:
		Star_FileNotFoundError(const std::string& filename, const std::string& what = "File not found: ")
		: StarError(what + filename) {}
	};
	
	class Star_DirectoryError : public StarError {
	public:
		Star_DirectoryError(const std::string& dirname, const std::string& what = "Directory error: ")
		: StarError(what + dirname) {}
	};
}

#endif
