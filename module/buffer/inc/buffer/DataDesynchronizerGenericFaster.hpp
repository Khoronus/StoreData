/**
* @file DataDesynchronizerGenericFaster.hpp
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


#ifndef STOREDATA_BUFFER_RECORDCONTAINERGENERICFASTER_HPP__
#define STOREDATA_BUFFER_RECORDCONTAINERGENERICFASTER_HPP__

#include <vector>
#include <iostream>
#include <chrono>
#include <thread>             // std::thread, std::this_thread::yield
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

#include <opencv2/opencv.hpp>

#include "logger/inc/logger/log.hpp"
#include "AtomicContainerDataFaster.hpp"

namespace storedata
{

/** @brief Function to process the data

	Callback function to process the data.
*/
typedef std::function<void(std::unique_ptr<AtomicContainerDataFaster> &rcd)> cbk_func_faster;


/** @brief Class to record all the frames currently captured
*/
class DataDesynchronizerGenericFaster
{

public:

	STOREDATA_BUFFER_EXPORT DataDesynchronizerGenericFaster();

	/** @brief It push a new frame to save
	*/
	STOREDATA_BUFFER_EXPORT void push(std::unique_ptr<AtomicContainerDataFaster> &rcd);

	/** @brief It starts the thread
	*/
	STOREDATA_BUFFER_EXPORT bool start();

	/** @brief It stops the thread
	*/
	STOREDATA_BUFFER_EXPORT void stop();

	/** @brief It close the recording files
	*/
	STOREDATA_BUFFER_EXPORT void close();

	/** @brief Thread used to save the current container of images
	*/
	STOREDATA_BUFFER_EXPORT void internal_thread();

	/** @brief It sets the save boosting. If true it use multiple threads to
	save.
	*/
	STOREDATA_BUFFER_EXPORT void set_save_boost(bool save_boost);

	/** @brief It returns the about size of the writing queue
	*/
	STOREDATA_BUFFER_EXPORT size_t size_about();

	/** @brief It returns the running status
	*/
	STOREDATA_BUFFER_EXPORT bool is_running();

	/** @brief Since the internal thread is detached, it is necessary to 
	           wait until is not ready.

		The function try to stop for n iterations and wait m ms.
		@return It returns true in case of success. False otherwise.
	*/
	STOREDATA_BUFFER_EXPORT bool wait_until_is_not_ready(size_t num_iterations, int sleep_ms);

	/** @brief Since the internal thread is detached, it is necessary to
		   wait until is not ready.

		The function try to stop for n iterations and wait m ms.
		@return It returns true in case of success. False otherwise.
	*/
	STOREDATA_BUFFER_EXPORT bool wait_until_buffer_is_empty(size_t num_iterations, int sleep_ms);

	/** @brief It sets the callback for the function that record data
	*/
	STOREDATA_BUFFER_EXPORT void set_cbk_func_faster(
		cbk_func_faster callback_func);

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

	/** @brief Container with the data to save and a message associated
	*/
	std::queue<std::unique_ptr<AtomicContainerDataFaster>> container_;

	/** @brief Callback recorder function
	*/
	cbk_func_faster callback_func_;
};


////-----------------------------------------------------------------------------
//template <typename _Ty>
//DataDesynchronizerGenericFaster<_Ty>::DataDesynchronizerGenericFaster() {
//	num_threads_ = 0;
//	max_threads_ = 1;
//	is_running_ = false;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//void DataDesynchronizerGenericFaster<_Ty>::push(
//	std::unique_ptr<AtomicContainerDataFaster<_Ty>> &rcd) {
//	{
//		std::lock_guard<std::mutex> lk(mtx_);
//		// Add the data
//		container_.push(std::move(rcd));
//		data_ready_ = true;
//	}
//
//	cond_.notify_one();
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//bool DataDesynchronizerGenericFaster<_Ty>::start() {
//	std::lock_guard<std::mutex> lk(mtx_);
//	if (is_running_) return false;
//	continue_save_ = true;
//	std::thread t1(&DataDesynchronizerGenericFaster::internal_thread, this);
//	t1.detach();
//	return true;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//void DataDesynchronizerGenericFaster<_Ty>::stop() {
//	continue_save_ = false;
//	cond_.notify_one();
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//void DataDesynchronizerGenericFaster<_Ty>::internal_thread() {
//
//	is_running_ = true;
//	while (continue_save_) {
//
//		// Digest the main buffer frames (it it exist)
//		std::unique_ptr<AtomicContainerDataFaster> element;
//		{
//			std::unique_lock<std::mutex> lk(mtx_);
//			// no data to save
//			if (container_.size() == 0) {
//				data_ready_ = false;
//				//std::cout << "wait: " << data_ready_ << std::endl;
//				cond_.wait(lk, [this] { return data_ready_; });
//				//std::cout << "end wait: " << data_ready_ << std::endl;
//			}
//			// Pop the data
//			// Test if there is data to prevent error due to spurious awake
//			//std::cout << "Container: " << container_.size() << std::endl;
//			if (container_.size() > 0) {
//				element = std::move(container_.front());
//				container_.pop();
//			}
//		}
//
//		// if the callback function does exist
//		if (callback_func_) {
//			callback_func_(element);
//
//			// dispose the data
//			if (element->data) {
//				element->dispose();
//			}
//		}
//	}
//	is_running_ = false;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//void DataDesynchronizerGenericFaster<_Ty>::set_save_boost(bool save_boost) {
//	std::lock_guard<std::mutex> lk(mtx_);
//	save_boost_ = save_boost;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//size_t DataDesynchronizerGenericFaster<_Ty>::size_about() {
//	return container_.size();
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//bool DataDesynchronizerGenericFaster<_Ty>::is_running() {
//	return is_running_;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//bool DataDesynchronizerGenericFaster<_Ty>::wait_until_is_not_ready(size_t num_iterations, int sleep_ms) {
//	for (size_t i = 0; i < num_iterations; ++i) {
//		if (is_running()) {
//			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
//		}
//		else {
//			return true;
//		}
//	}
//	return false;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//bool DataDesynchronizerGenericFaster<_Ty>::wait_until_buffer_is_empty(
//	size_t num_iterations,
//	int sleep_ms) {
//	for (size_t i = 0; i < num_iterations; ++i) {
//		std::cout << "iA: " << i << " " << (size_about() > 0) << " ";
//		if (is_running() && (size_about() > 0)) {
//			std::cout << size_about() << std::endl;
//			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
//		}
//		else {
//			return true;
//		}
//	}
//	return false;
//}
////-----------------------------------------------------------------------------
//template <typename _Ty>
//void DataDesynchronizerGenericFaster<_Ty>::set_cbk_func_faster(
//	std::function<void(std::unique_ptr<AtomicContainerDataFaster<_Ty>> &rcd)> callback_func) {
//	callback_func_ = callback_func;
//}

} // namespace storedata

#endif // STOREDATA_BUFFER_RECORDCONTAINERGENERICFASTER_HPP__