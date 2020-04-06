/**
* @file recordcontainerbase.hpp
* @brief Header of the defined class
*
* @section LICENSE
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR/AUTHORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* @original author Alessandro Moro <alessandromoro.italy@gmail.com>
* @bug No known bugs.
* @version 0.2.0.0
*
*/


#ifndef STOREDATA_RECORD_RECORDCONTAINERBASE_HPP__
#define STOREDATA_RECORD_RECORDCONTAINERBASE_HPP__

#include <vector>
#include <iostream>
#include <chrono>
#include <thread>             // std::thread, std::this_thread::yield
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#include <opencv2/opencv.hpp>

#include "buffer/buffer_headers.hpp"
#include "logger/inc/logger/log.hpp"

#include "record_defines.hpp"

//#define dWriteData(a, b) try { cv::imwrite(a, b); }                            \
//                         catch(...) { std::cout << "Default exception"; }

#define dWriteData(a, b) try { 	FILE *fp; \
                         fp = fopen(a.c_str(), "w"); \
                         fwrite(b.data, 1, b.size_bytes, fp); \
                         fclose(fp); }                   \
                         catch(...) { std::cout << "Default exception"; }

//#define dWriteData(a, b) try { } catch(...){std::cout << "Default exception";}

namespace storedata
{

struct STOREDATA_RECORD_EXPORT RecordContainerData
{
	void* data;
	size_t size_bytes;
	RecordContainerData() : data(nullptr) {}
	void copyFrom(const void* src, size_t src_size_bytes) {
		if (data) dispose();
		size_bytes = src_size_bytes;
		data = malloc(size_bytes);
		memcpy(data, src, size_bytes);
	}
	void copyFrom(RecordContainerData &obj) {
		if (data) dispose();
		size_bytes = obj.size_bytes;
		data = malloc(size_bytes);
		memcpy(data, obj.data, obj.size_bytes);
	}
	void dispose() {
		if (data) { free(data); data = nullptr; }
	}
};

/** @brief Class to record all the frames currently captured
*/
class RecordContainerBase
{

public:

	STOREDATA_RECORD_EXPORT RecordContainerBase() {};

	/** @brief It push a new frame to save
	*/
	STOREDATA_RECORD_EXPORT virtual void push(const std::string &fname, RecordContainerData &rcd) = 0;

	/** @brief It push a new microbuffer to save
	*/
	STOREDATA_RECORD_EXPORT virtual void push(std::vector<vb::PtrMicrobuffer> &vptr) = 0;

	/** @brief It starts the thread
	*/
	STOREDATA_RECORD_EXPORT virtual bool start() = 0;

	/** @brief It stops the thread
	*/
	STOREDATA_RECORD_EXPORT virtual void stop() = 0;

	/** @brief Thread used to save the current container of images
	*/
	STOREDATA_RECORD_EXPORT virtual void internal_thread() = 0;
};


} // namespace storedata

#endif // STOREDATA_RECORD_RECORDCONTAINERBASE_HPP__