/*
MIT License

Copyright (c) 2026 bhcbhc001
Copyright (c) 2026 minermouse

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

#ifndef STAR_IO_H
#define STAR_IO_H

#include<fstream>
#include<filesystem>
#include<thread>
#include<atomic>
#include<system_error>

#include "utils/StarError.h"
#include "utils/subprocess.h"

namespace STAR_CPP {
	namespace fs = std::filesystem;
	
	class IO {
	private:
		std::string _input_file;
		std::string _output_file;
		std::string _output_dir;
		std::ofstream _input;
		std::ofstream _output;
		
		static const size_t BUFFER_SIZE = 8192;
		
		std::string _get_full_path(const std::string& filename) {
			if (_output_dir.empty()) return filename;
			fs::path dir(_output_dir);
			std::error_code ec;
			if (!fs::exists(dir, ec)) fs::create_directories(dir, ec);
			return (dir / filename).string();
		}
		
	public:
		IO(const std::string& input_name = "", const std::string& output_name = "", const std::string& output_dir = "")
		: _input_file(input_name), _output_file(output_name), _output_dir(output_dir) {}
		
		~IO() {
			if (_input.is_open()) _input.close();
			if (_output.is_open()) _output.close();
		}
		
		void set_output_dir(const std::string& output_dir) {
			_output_dir = output_dir;
			if (_input.is_open()) _input.close();
			if (_output.is_open()) _output.close();
		}
		
		std::string get_output_dir() const { return _output_dir; }
		
		void set_output_filename(const std::string& output_name) { _output_file = output_name; }
		std::string get_output_filename() const { return _output_file; }
		
		void set_input_filename(const std::string& input_name) { _input_file = input_name; }
		std::string get_input_filename() const { return _input_file; }
		
		template<typename T>
		void input(const T& data) {
			if (_input_file.empty()) return;
			if (!_input.is_open()) {
				std::string full_path = _get_full_path(_input_file);
				_input.open(full_path);
				if (!_input.is_open()) throw Star_CantOpenFileError(full_path, "Cannot open input file: ");
			}
			_input << data;
		}
		
		template<typename T>
		void output(const T& data) {
			if (_output_file.empty()) return;
			if (!_output.is_open()) {
				std::string full_path = _get_full_path(_output_file);
				_output.open(full_path, std::ios::app);
				if (!_output.is_open()) throw Star_CantOpenFileError(full_path, "Cannot open output file: ");
			}
			_output << data;
		}
		
		using func = IO& (*)(IO&);
		IO& operator<<(func manip) { return manip(*this); }
		
		template<typename T>
		IO& operator<<(const T& data) {
			input(data);
			return *this;
		}
		
		void reopen() {
			if (_input.is_open()) _input.close();
			if (_output.is_open()) _output.close();
			
			if (!_input_file.empty()) {
				std::string full_path = _get_full_path(_input_file);
				_input.open(full_path);
				if (!_input.is_open()) throw Star_CantOpenFileError(full_path, "Cannot reopen input file");
			}
			if (!_output_file.empty()) {
				std::string full_path = _get_full_path(_output_file);
				_output.open(full_path, std::ios::app);
				if (!_output.is_open()) throw Star_CantOpenFileError(full_path, "Cannot reopen output file");
			}
		}
		
		void createof(const std::string& executable_path) {
			std::string final_input_path = _get_full_path(_input_file);
			std::string final_output_path = _get_full_path(_output_file);
			
			if (!_output_dir.empty()) {
				std::error_code ec;
				if (fs::exists(_input_file, ec) && !fs::equivalent(_input_file, final_input_path, ec)) fs::remove(_input_file, ec);
				ec.clear();
				if (fs::exists(_output_file, ec) && !fs::equivalent(_output_file, final_output_path, ec)) fs::remove(_output_file, ec);
			}
			
			if (_input.is_open()) { _input.flush(); _input.close(); }
			if (_output.is_open()) { _output.flush(); _output.close(); }
			
			const char* command_line[] = { executable_path.c_str(), nullptr };
			struct subprocess_s process;
			
			if (subprocess_create(command_line, subprocess_option_inherit_environment | subprocess_option_enable_async, &process) != 0)
				throw std::runtime_error("Failed to create subprocess: " + executable_path);
			
			std::atomic<bool> input_error{false};
			std::atomic<bool> output_error{false};
			
			std::thread input_thread([&]() {
				FILE* fin = fopen(final_input_path.c_str(), "rb");
				FILE* pin = subprocess_stdin(&process);
				char buffer[BUFFER_SIZE];
				
				if (fin && pin) {
					size_t bytes_read;
					while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fin)) > 0) {
						if (fwrite(buffer, 1, bytes_read, pin) != bytes_read) {
							input_error = true;
							break;
						}
					}
				}
				
				if (fin) fclose(fin);
				if (pin) {
					fclose(pin);
					process.stdin_file = nullptr;
				}
			});
			
			std::thread output_thread([&]() {
				FILE* fout = fopen(final_output_path.c_str(), "wb");
				FILE* pout = subprocess_stdout(&process);
				char buffer[BUFFER_SIZE];
				
				if (fout && pout) {
					int bytes_read;
					while ((bytes_read = subprocess_read_stdout(&process, buffer, BUFFER_SIZE)) > 0) {
						fwrite(buffer, 1, bytes_read, fout);
					}
				}
				
				if (fout) fclose(fout);
				if (pout) {
					fclose(pout);
					process.stdout_file = nullptr;
				}
			});
			
			if (input_thread.joinable()) input_thread.join();
			if (output_thread.joinable()) output_thread.join();
			
			if (input_error) throw std::runtime_error("Error writing to subprocess stdin");
			if (output_error) throw std::runtime_error("Error reading from subprocess stdout");
			
			int return_code = 0;
			subprocess_join(&process, &return_code);
			subprocess_destroy(&process);
			
			if (return_code != 0)
				throw std::runtime_error("Program exited with code: " + std::to_string(return_code));
			
			_output.open(final_output_path, std::ios::app);
			if (!_output.is_open())
				throw Star_CantOpenFileError(final_output_path, "Cannot reopen output file");
		}
	};
}

#endif

