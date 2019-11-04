/**
* @file recordcontainer.hpp
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


#ifndef SENSORCORECORE_SENSORCORECOREWORLD_RECORDCONTAINER_HPP__
#define SENSORCORECORE_SENSORCORECOREWORLD_RECORDCONTAINER_HPP__

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
class RecordContainer
{

public:

	STOREDATA_RECORD_EXPORT RecordContainer();

	/** @brief It push a new frame to save
	*/
	STOREDATA_RECORD_EXPORT void push(const std::string &fname, RecordContainerData &rcd);

	/** @brief It push a new microbuffer to save
	*/
	STOREDATA_RECORD_EXPORT void push(std::vector<vb::PtrMicrobuffer> &vptr);

	/** @brief It starts the thread
	*/
	STOREDATA_RECORD_EXPORT bool start();

	/** @brief It stops the thread
	*/
	STOREDATA_RECORD_EXPORT void stop();

	/** @brief Thread used to save the current container of images
	*/
	STOREDATA_RECORD_EXPORT void internal_thread();

	/** @brief It sets the save boosting. If true it use multiple threads to
	save.
	*/
	STOREDATA_RECORD_EXPORT void set_save_boost(bool save_boost);

	/** @brief It returns the about size of the writing queue
	*/
	STOREDATA_RECORD_EXPORT size_t size_about();

	/** @brief It returns the about size of the microbuffer
	*/
	STOREDATA_RECORD_EXPORT size_t size_about_micro();

	/** @brief It returns the running status
	*/
	STOREDATA_RECORD_EXPORT bool is_running();

	/** @brief Since the internal thread is detached, it is necessary to 
	           wait until is not ready.

		The function try to stop for n iterations and wait m ms.
		@return It returns true in case of success. False otherwise.
	*/
	STOREDATA_RECORD_EXPORT bool wait_until_is_not_ready(size_t num_iterations, int sleep_ms);

	/** @brief Since the internal thread is detached, it is necessary to
		   wait until is not ready.

		The function try to stop for n iterations and wait m ms.
		@return It returns true in case of success. False otherwise.
	*/
	STOREDATA_RECORD_EXPORT bool wait_until_buffer_is_empty(size_t num_iterations, int sleep_ms);

private:

	/** @brief Mutex to prevent race condition
	*/
	std::mutex mtx_;
	/** @brief If there is no data to save, it stops
	*/
	std::condition_variable cond_;
	/** @brief Var used to the condition variable
	*/
	bool data_ready_ = false;
	/** @brief Number of threads that are writing (it does not include the
	class internal thread)
	*/
	int num_threads_;
	/** @brief Maximum numbers of threads that can be run (except the
	internal thread)
	*/
	int max_threads_;

	/** @brief If true it creates as many threads as possible
	*/
	bool save_boost_;

	/** @brief If true it continues to save the data
	*/
	bool continue_save_;
	/** @brief If true is running the internal thread
	*/
	bool is_running_;

	/** @brief Container with the data to save
	*/
	std::queue<std::pair<std::string, RecordContainerData>> container_;

	/** @brief Container with the microbuffer to save
	*/
	std::queue<std::vector<vb::PtrMicrobuffer>> container_microbuffer_;
	/** @brief Approximatively how many elements are under saving for the 
	           microbuffer.
	*/
	size_t num_elems_microbuffer_approx_;
};




#endif // SENSORCORECORE_SENSORCORECOREWORLD_RECORDCONTAINER_HPP__