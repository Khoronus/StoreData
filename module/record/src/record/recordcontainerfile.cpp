/* @file recordcontainerfile.cpp
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

#include "record/inc/record/recordcontainerfile.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
RecordContainerFile::RecordContainerFile() {
	num_threads_ = 0;
	max_threads_ = 1;
	is_running_ = false;
	num_elems_microbuffer_approx_ = 0;
}
//-----------------------------------------------------------------------------
void RecordContainerFile::push(
	const std::string &fname, 
	RecordContainerData &rcd,
	bool do_use_max_size_buffer,
	size_t max_size_buffer_container) {

	// Too many elements in the buffer.
	if (do_use_max_size_buffer &&
		container_.size() > max_size_buffer_container) {
		return;
	}
	
	{
		std::lock_guard<std::mutex> lk(mtx_);
		// Add the data
		container_.push(std::make_pair(fname, rcd));
		data_ready_ = true;
	}

	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void RecordContainerFile::push(
	std::vector<vb::PtrMicrobuffer> &vptr,
	bool do_use_max_size_buffer,
	size_t max_size_buffer_container)
{
	// Too many elements in the buffer.
	if (do_use_max_size_buffer &&
		container_microbuffer_.size() > max_size_buffer_container) {
		return;
	}

	{
		std::lock_guard<std::mutex> lk(mtx_);
		// Add the data
		container_microbuffer_.push(vptr);
		data_ready_ = true;
	}

	cond_.notify_one();
}
//-----------------------------------------------------------------------------
bool RecordContainerFile::start() {
	std::lock_guard<std::mutex> lk(mtx_);
	if (is_running_) return false;
	continue_save_ = true;
	std::thread t1(&RecordContainerFile::internal_thread, this);
	t1.detach();
	return true;
}
//-----------------------------------------------------------------------------
void RecordContainerFile::stop() {
	continue_save_ = false;
	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void RecordContainerFile::close(int num_iterations, int wait_ms) {
	stop();
	wait_until_buffer_is_empty(num_iterations, wait_ms);
	wait_until_is_not_ready(num_iterations, wait_ms);
}
////-----------------------------------------------------------------------------
//void RecordContainer::internal_thread() {
// THIS FUNCTION DOES NOT SEEMS TO BE ANY VALID FOR THE FILE ANYMORE
//	is_running_ = true;
//	while (continue_save_) {
//
//		// Digest the microbuffer (if it exist)
//		std::vector<vb::PtrMicrobuffer> vptr;
//		{
//			std::unique_lock<std::mutex> lk(mtx_);
//			if (container_microbuffer_.size() > 0) {
//				vptr = container_microbuffer_.front();
//				container_microbuffer_.pop();
//			}
//		}
//		// Save all the frames in the microbuffer
//		if (vptr.size() > 0) {
//			// Count the number of elements in the microbuffer
//			num_elems_microbuffer_approx_ = 0;
//			for (auto &it : vptr) {
//				num_elems_microbuffer_approx_ += it->size();
//			}
//			//std::cout << "Save microbuffer: " << vptr.size() << std::endl;
//			for (auto &it : vptr) {
//				for (auto &it2 : *it) {
//					//std::cout << "SaveMB: " << it2.second.first << std::endl;
//					//std::cout << "it2: " << it2.second.second.size() << std::endl;
//					if (!it2.second.second.empty()) {
//						dWriteData(it2.second.first, it2.second.second);
//					}
//					// Decrease the counter
//					--num_elems_microbuffer_approx_;
//				}
//			}
//		}
//		num_elems_microbuffer_approx_ = 0;
//
//		// Digest the main buffer frames (it it exist)
//		std::pair<std::string, cv::Mat> data;
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
//				data = container_.front();
//				container_.pop();
//			}
//		}
//		// Process the data
//		if (!data.second.empty()) {
//
//			if (!save_boost_) {
//				//cv::imwrite(data.first, data.second);
//				dWriteData(data.first, data.second)
//			}
//			else {
//				// Use multiple threads to speedup. The system can 
//				// use a limited maximum number of threads
//				if (num_threads_ < max_threads_) {
//					std::lock_guard<std::mutex> lk(mtx_);
//					++num_threads_;
//
//					// Lambda function looks better for few lines of code
//					// If no detach, the thread is relased from memory (stack) and crush
//					std::string fname = data.first;
//					cv::Mat src = data.second;
//					std::thread([fname, src, this] {
//						// Use a lexical scope and lock_guard to safely lock the mutex only
//						// for the duration of std::cout usage.
//						//std::lock_guard<std::mutex> iolock(iomutex);
//						//std::cout << "Thread #" << ": on CPU " << GetCurrentProcessorNumber() << "\n";
//						//cv::imwrite(fname, src);
//						dWriteData(fname, src)
//							std::lock_guard<std::mutex> lk(mtx_);
//						--num_threads_;
//					}).detach();
//				}
//				else {
//					//cv::imwrite(data.first, data.second);
//					dWriteData(data.first, data.second)
//				}
//			}
//		}
//	}
//	is_running_ = false;
//}
//-----------------------------------------------------------------------------
void RecordContainerFile::internal_thread() {
	is_running_ = true;
	while (continue_save_) {

		// Digest the main buffer frames (it it exist)
		std::pair<std::string, RecordContainerData> tuple;
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
				tuple = container_.front();
				container_.pop();
			}
		}
		// Process the data
		if (tuple.second.data) {

			FILE *fp;
			fp = fopen(tuple.first.c_str(), "wb");
			if (fp) {
				fwrite(tuple.second.data, 1, tuple.second.size_bytes, fp);
				fclose(fp);
			}
			// Dispose the data
			tuple.second.dispose();
		}
	}
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void RecordContainerFile::set_save_boost(bool save_boost) {
	std::lock_guard<std::mutex> lk(mtx_);
	save_boost_ = save_boost;
}
//-----------------------------------------------------------------------------
size_t RecordContainerFile::size_about() {
	return container_.size() + num_elems_microbuffer_approx_;
}
//-----------------------------------------------------------------------------
size_t RecordContainerFile::size_about_micro() {
	return container_microbuffer_.size();
}
//-----------------------------------------------------------------------------
bool RecordContainerFile::is_running() {
	return is_running_;
}
//-----------------------------------------------------------------------------
bool RecordContainerFile::wait_until_is_not_ready(
	size_t num_iterations, 
	int sleep_ms) {
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
bool RecordContainerFile::wait_until_buffer_is_empty(
	size_t num_iterations, 
	int sleep_ms) {
	for (size_t i = 0; i < num_iterations; ++i) {
		//std::cout << "iA: " << i << " " << (size_about() > 0) << " ";
		if (is_running() && (size_about() > 0)) {
			//std::cout << size_about() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		} else {
			return true;
		}
	}
	return false;
}


} // namespace storedata
