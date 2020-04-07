/* @file DataDesynchronizerGenericInherit.cpp
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

#include "buffer/inc/buffer/DataDesynchronizerGenericInherit.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
	DataDesynchronizerGenericInherit::DataDesynchronizerGenericInherit() {
	num_threads_ = 0;
	max_threads_ = 1;
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericInherit::push(
	std::unique_ptr<AtomicContainerDataInherit> &rcd) {
	{
		std::lock_guard<std::mutex> lk(mtx_);
		// Add the data
		container_.push(std::move(rcd));
		data_ready_ = true;
	}

	cond_.notify_one();
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericInherit::start() {
	std::lock_guard<std::mutex> lk(mtx_);
	if (is_running_) return false;
	continue_save_ = true;
	std::thread t1(&DataDesynchronizerGenericInherit::internal_thread, this);
	t1.detach();
	return true;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericInherit::stop() {
	continue_save_ = false;
	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericInherit::close() {
	stop();
	wait_until_is_not_ready(10, 100);
	wait_until_buffer_is_empty(10, 100);
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericInherit::internal_thread() {

	is_running_ = true;
	while (continue_save_) {

		// Digest the main buffer frames (it it exist)
		std::unique_ptr<AtomicContainerDataInherit> element;
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
			//element->dispose();
			//element.reset();

			//// dispose the data (no need)
			//if (element->data) {
			//	element->dispose();
			//}
		}
	}
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void DataDesynchronizerGenericInherit::set_save_boost(bool save_boost) {
	std::lock_guard<std::mutex> lk(mtx_);
	save_boost_ = save_boost;
}
//-----------------------------------------------------------------------------
size_t DataDesynchronizerGenericInherit::size_about() {
	return container_.size();
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericInherit::is_running() {
	return is_running_;
}
//-----------------------------------------------------------------------------
bool DataDesynchronizerGenericInherit::wait_until_is_not_ready(size_t num_iterations, int sleep_ms) {
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
bool DataDesynchronizerGenericInherit::wait_until_buffer_is_empty(
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
void DataDesynchronizerGenericInherit::set_cbk_func_inherit(
	cbk_func_inherit callback_func) {
	callback_func_ = callback_func;
}


} // namespace storedata
