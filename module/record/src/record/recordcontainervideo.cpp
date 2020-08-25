/* @file recordcontainervideo.cpp
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

#include "record/inc/record/recordcontainervideo.hpp"

namespace storedata
{

//-----------------------------------------------------------------------------
RecordContainerVideo::RecordContainerVideo() {
	num_threads_ = 0;
	max_threads_ = 1;
	is_running_ = false;
	num_elems_microbuffer_approx_ = 0;
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::push(
	const std::string &msg, 
	RecordContainerData &rcd,
	bool do_use_max_size_buffer,
	size_t max_size_buffer_container) {

	// Too many elements in the buffer.
	if (do_use_max_size_buffer &&
		container_.size() > (std::max)(static_cast<size_t>(0), 
			max_size_buffer_container)) {
		return;
	}

	{
		std::lock_guard<std::mutex> lk(mtx_);
		// Add the data
		container_.push(std::make_pair(msg, rcd));
		data_ready_ = true;
	}

	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::push(
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
bool RecordContainerVideo::start() {
	std::lock_guard<std::mutex> lk(mtx_);
	if (is_running_) return false;
	continue_save_ = true;
	std::thread t1(&RecordContainerVideo::internal_thread, this);
	t1.detach();
	return true;
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::stop() {
	continue_save_ = false;
	cond_.notify_one();
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::close(int num_iterations, int wait_ms) {
	stop();
	if (wait_until_buffer_is_empty(num_iterations, wait_ms) &&
		wait_until_is_not_ready(num_iterations, wait_ms)) {
		vw_.release();
		fout_.close();
		fout_.clear();
	}
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::internal_thread() {

	// save the microbuffered frames
	// The buffer is not recorded in order. 
	// It requires to read as
	/*
					for (std::vector<vb::PtrMicrobuffer>::reverse_iterator it = vptr.rbegin(); it != vptr.rend(); ++it) {
					for (auto &it2 : **it) {
						std::cout << "> " << it2.first << " " <<
							static_cast<MicroBufferObjDerived*>(it2.second.get())->msg() << std::endl;
						cv::imshow("m", static_cast<MicroBufferObjDerived*>(it2.second.get())->img());
						cv::waitKey(1);
					}
				}

	*/
	while (container_microbuffer_.size() > 0) {
		
		//// Digest the microbuffer (if it exist)
		//std::vector<vb::PtrMicrobuffer> vptr;
		//{
		//	std::unique_lock<std::mutex> lk(mtx_);
		//	if (container_microbuffer_.size() > 0) {
		//		vptr = container_microbuffer_.front();
		//		container_microbuffer_.pop();
		//	}
		//}
		//// Save all the frames in the microbuffer
		//if (vptr.size() > 0) {
		//	// Count the number of elements in the microbuffer
		//	num_elems_microbuffer_approx_ = 0;
		//	for (auto &it : vptr) {
		//		num_elems_microbuffer_approx_ += it->size();
		//	}
		//	//std::cout << "Save microbuffer: " << vptr.size() << std::endl;
		//	for (auto &it : vptr) {
		//		for (auto &it2 : *it) {
		//			//std::cout << "SaveMB: " << it2.second.first << std::endl;
		//			//std::cout << "it2: " << it2.second.second.size() << std::endl;
		//			if (it2.second &&
		//				!static_cast<cv::Mat*>(it2.second->get_item(1))->empty()) {
		//				cv::Mat img = *static_cast<cv::Mat*>(it2.second->get_item(1));
		//				filevideo_push_frame(vw_, fname_root_ + ".avi", img);
		//			}
		//			// Decrease the counter
		//			--num_elems_microbuffer_approx_;
		//		}
		//	}
		//}
		//num_elems_microbuffer_approx_ = 0;


		// Digest the microbuffer (if it exist)
		std::vector<vb::PtrMicrobuffer> vptr;
		{
			std::unique_lock<std::mutex> lk(mtx_);
			if (container_microbuffer_.size() > 0) {
				vptr = container_microbuffer_.front();
				container_microbuffer_.pop();
			}
		}

		// Count the number of elements in the microbuffer
		num_elems_microbuffer_approx_ = 0;
		for (auto &it : vptr) {
			num_elems_microbuffer_approx_ += it->size();
		}

		for (std::vector<vb::PtrMicrobuffer>::reverse_iterator it = vptr.rbegin(); it != vptr.rend(); ++it) {
			for (auto &it2 : **it) {
				//std::cout << "SaveMB: " << it2.second.first << std::endl;
				//std::cout << "it2: " << it2.second.second.size() << std::endl;
				if (it2.second &&
					!static_cast<cv::Mat*>(it2.second->get_item(1))->empty()) {
					cv::Mat img = *static_cast<cv::Mat*>(it2.second->get_item(1));
					filevideo_push_frame(vw_, fname_root_ + ".avi", img);
				}
				// Decrease the counter
				--num_elems_microbuffer_approx_;
			}
		}
		num_elems_microbuffer_approx_ = 0;

	}

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
		// check potential errors
		if (size_image_.width == 0 || size_image_.height == 0 ||
			fname_root_.size() == 0) {
			std::cout << "[e] size_image_:" << size_image_ <<
				" fname_root_:" << fname_root_ << std::endl;
			continue_save_ = false;
			continue;
		}

		// Process the data
		if (tuple.second.data) {
			// convert the binary in image
			cv::Mat img = cv::Mat(size_image_, CV_8UC3, tuple.second.data);
			filevideo_push_frame(vw_, fname_root_ + ".avi", img);

			// open a file with the frame information
			if (!fout_.is_open()) {
				fout_.open(fname_root_ + ".txt");
			}
			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() +
				" " + std::to_string(cv::getTickCount());
			fout_ << tuple.first << "|" << timestamp << std::endl;

			//// codify
			//{
			//	std::vector< std::string > vmsg, testmsg;
			//	vmsg.push_back("This is an example");
			//	vmsg.push_back("of how");
			//	vmsg.push_back("a data is codified in an image $%&#?!");

			//	// Codify the strings
			//	int x = data_block_offset, y = data_block_offset;
			//	storedata::codify::CodifyImage::merge_strings2image(vmsg, data_block_size,
			//		data_block_offset, m_data_block, x, y);

			//	// combine the images
			//	cv::Mat m(frame_size, img.type());
			//	img.copyTo(m(cv::Rect(0, 0, img.cols, img.rows)));
			//	m_data_block.copyTo(m(cv::Rect(0, img.rows, m_data_block.cols, m_data_block.rows)));
			//	// record
			//	filevideo_push_frame(vw_, tuple.first, m);
			//}

			// Dispose the data
			tuple.second.dispose();
		}
	}
	is_running_ = false;
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::set_save_boost(bool save_boost) {
	std::lock_guard<std::mutex> lk(mtx_);
	save_boost_ = save_boost;
}
//-----------------------------------------------------------------------------
size_t RecordContainerVideo::size_about() {
	return container_.size() + num_elems_microbuffer_approx_;
}
//-----------------------------------------------------------------------------
size_t RecordContainerVideo::size_about_micro() {
	return container_microbuffer_.size();
}
//-----------------------------------------------------------------------------
bool RecordContainerVideo::is_running() {
	return is_running_;
}
//-----------------------------------------------------------------------------
bool RecordContainerVideo::wait_until_is_not_ready(size_t num_iterations, int sleep_ms) {
	for (size_t i = 0; i < num_iterations; ++i) {
		if (is_running()) {
			cond_.notify_one();
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		}
		else {
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool RecordContainerVideo::wait_until_buffer_is_empty(
	size_t num_iterations, 
	int sleep_ms) {
	for (size_t i = 0; i < num_iterations; ++i) {
		//std::cout << "iA: " << i << " " << (size_about() > 0) << " ";
		if (is_running() && (size_about() > 0)) {
			cond_.notify_one();
			//std::cout << size_about() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
		} else {
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::set_size_image(const cv::Size &size_image) {
	size_image_ = size_image;
}
//-----------------------------------------------------------------------------
void RecordContainerVideo::set_fname_root(const std::string &fname_root) {
	fname_root_ = fname_root;
	vw_.release();
	fout_.close();
	fout_.clear();
}//-----------------------------------------------------------------------------
void RecordContainerVideo::filevideo_push_frame(cv::VideoWriter &vw,
	const std::string &fname, cv::Mat &img) {
	if (!vw.isOpened()) {
		vw.open(fname,
			cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30, img.size(),
			true);
	}
	vw << img;
}

} // namespace storedata