/* @file recordcontainergeneric.cpp
 * @brief Main file with the example for the hog descriptor and visualization.
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
 * @author Alessandro Moro <alessandromoro.italy@gmail.com>
 * @bug No known bugs.
 * @version 0.1.0.0
 * 
 */

#include "buffer/inc/buffer/DataDesynchronizerGenericFaster.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
DataDesynchronizerGenericFaster::DataDesynchronizerGenericFaster() {
	num_threads_ = 0;
	max_threads_ = 1;
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::push(
	std::unique_ptr<AtomicContainerDataFaster> &rcd) {
	{
		std::lock_guard<std::mutex> lk(mtx_);
		// Add the data
		container_.push(std::move(rcd));
		data_ready_ = true;
	}

	cond_.notify_one();
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericFaster::start() {
	std::lock_guard<std::mutex> lk(mtx_);
	if (is_running_) return false;
	continue_save_ = true;
	std::thread t1(&DataDesynchronizerGenericFaster::internal_thread, this);
	t1.detach();
	return true;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::stop() {
	continue_save_ = false;
	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::close() {
	stop();
	wait_until_is_not_ready(10, 100);
	wait_until_buffer_is_empty(10, 100);
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::internal_thread() {

	is_running_ = true;
	while (continue_save_) {

		// Digest the main buffer frames (it it exist)
		std::unique_ptr<AtomicContainerDataFaster> element;
		{
			std::unique_lock<std::mutex> lk(mtx_);
			// no data to save
			if (container_.size() == 0) {
				data_ready_ = false;
				//std::cout << "wait: " << data_ready_ << std::endl;
				cond_.wait(lk, [this] { return data_ready_; });
				//std::cout << "end wait: " << data_ready_ << std::endl;
			}
			// Pop the data
			// Test if there is data to prevent error due to spurious awake
			//std::cout << "Container: " << container_.size() << std::endl;
			if (container_.size() > 0) {
				element = std::move(container_.front());
				container_.pop();
			}
		}

		// if the callback function does exist
		if (callback_func_) {
			callback_func_(element);

			// dispose the data
			if (element->data) {
				element->dispose();
			}
		}
	}
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::set_save_boost(bool save_boost) {
	std::lock_guard<std::mutex> lk(mtx_);
	save_boost_ = save_boost;
}
//-----------------------------------------------------------------------------
size_t DataDesynchronizerGenericFaster::size_about() {
	return container_.size();
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericFaster::is_running() {
	return is_running_;
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericFaster::wait_until_is_not_ready(size_t num_iterations, int sleep_ms) {
	for (size_t i = 0; i < num_iterations; ++i) {
		if (is_running()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
		else {
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericFaster::wait_until_buffer_is_empty(
	size_t num_iterations, 
	int sleep_ms) {
	for (size_t i = 0; i < num_iterations; ++i) {
		std::cout << "iA: " << i << " " << (size_about() > 0) << " ";
		if (is_running() && (size_about() > 0)) {
			std::cout << size_about() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
		else {
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericFaster::set_cbk_func_faster(
	cbk_func_faster callback_func) {
	callback_func_ = callback_func;
}




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
//		{
//			std::lock_guard<std::mutex> lk(mtx_);
//			// Add the data
//			container_.push(std::move(rcd));
//			data_ready_ = true;
//		}
//
//		cond_.notify_one();
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
