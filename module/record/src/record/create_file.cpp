/* @file create_file.cpp
* @brief Body of the defined class.
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
* @version 0.3.0.0
*
*/

#include "record/inc/record/create_file.hpp"

namespace storedata
{

// ----------------------------------------------------------------------------
MemorizeFileManager::MemorizeFileManager() {}
// ----------------------------------------------------------------------------
MemorizeFileManager::~MemorizeFileManager() {
	release();
}
// ----------------------------------------------------------------------------
void MemorizeFileManager::release() {
	fout_.close();
	fout_.clear();
}
// ----------------------------------------------------------------------------
void MemorizeFileManager::setup(size_t memory_max_allocable,
	const std::string &filename,
	const std::string &dot_extension) {

	memory_expected_allocated_ = 0;
	memory_max_allocable_ = memory_max_allocable;
	filename_ = filename;
	dot_extension_ = dot_extension;
}
// ----------------------------------------------------------------------------
int MemorizeFileManager::generate(const std::string &appendix, bool append) {
	if (!fout_.is_open()) {
		std::string filename = filename_ + appendix + dot_extension_;
		std::cout << filename << std::endl;
		// get the current time
		if (append) {
			fout_.open(filename.c_str(), std::ios::binary | std::ios::app);
		} else {
			fout_.open(filename.c_str(), std::ios::binary);
		}
		// Get the file size
		memory_expected_allocated_ = filesize(filename.c_str());
		return kSuccess;
	}
	return kFail;
}
// ----------------------------------------------------------------------------
int MemorizeFileManager::check_memory(size_t size) {
	//std::cout << ">> " << memory_expected_allocated_ << " " << size << " " << memory_max_allocable_ << std::endl;
	if (memory_expected_allocated_ + size < memory_max_allocable_) {
		return kSuccess;
	}
	return kFail;
}
// ----------------------------------------------------------------------------
int MemorizeFileManager::push(const std::vector<char> &data) {
	if (!fout_.is_open()) return kFileIsNotOpen;

	if (data.size() > 0) {
		if (check_memory(data.size())) {
			fout_.write(&data[0], data.size());
			fout_.flush();
			memory_expected_allocated_ += data.size();
			return kSuccess;
		} else {
			// out of memory
			return kOutOfMemory;
		}
	}
	// The data is empty
	return kDataIsEmpty;
}
// ----------------------------------------------------------------------------
FileGeneratorManagerAsync::FileGeneratorManagerAsync(){
	verbose_ = false;
	under_writing_ = false;
}
// ----------------------------------------------------------------------------
FileGeneratorManagerAsync::~FileGeneratorManagerAsync() {
	close();
}
// ----------------------------------------------------------------------------
int FileGeneratorManagerAsync::setup(
	unsigned int max_memory_allocable,
	std::map<int, FileGeneratorParams> &vgp, int record_framerate) {
	std::cout << "FileGeneratorManagerAsync::setup" << std::endl;
	int return_status = kSuccess;

	// set framerate to record and capture at
	record_framerate_ = record_framerate;

#ifdef BOOST_BUILD

	// initialize initial timestamps
	nextFrameTimestamp_ = boost::posix_time::microsec_clock::local_time();
	currentFrameTimestamp_ = nextFrameTimestamp_;
	td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

#endif

	// Get the appendix to add to the video
	std::string appendix = DateTime::time2string();
	std::cout << "FileGeneratorManagerAsync::setup:appendix: " <<
		appendix << std::endl;
	for (int i = 0; i < appendix.length(); i++)
	{
		if (appendix[i] == ':') appendix[i] = '_';
	}
	// callback to inform that a new file will be created
	if (callback_createfile_) {
		callback_createfile_(appendix);
	}

	// Create the video writer
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = vgp.begin(); it != vgp.end(); it++)
#else
	for (std::map<int, FileGeneratorParams>::const_iterator it = vgp.begin(); it != vgp.end(); it++)
#endif		
	{
		m_files_[it->first] = new MemorizeFileManager();
		m_files_[it->first]->setup(max_memory_allocable,
			it->second.filename(), it->second.dot_extension());
		if (!m_files_[it->first]->generate(appendix, false)) {
			return_status = kFail;
		}
	}

	//number_addframe_requests_ = 0;
	under_writing_ = false;
	return return_status;
}
// ----------------------------------------------------------------------------
bool FileGeneratorManagerAsync::under_writing() {
	return under_writing_;
}
// ----------------------------------------------------------------------------
bool FileGeneratorManagerAsync::procedure() {

#ifdef BOOST_BUILD

	bool result_out = false;

	boost::mutex::scoped_lock lock(mutex_, boost::try_to_lock);
	if (lock) {
		under_writing_ = true;

		//determine current elapsed time
		currentFrameTimestamp_ = boost::posix_time::microsec_clock::local_time();
		td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

		// wait for X microseconds until 1second/framerate time has passed after previous frame write
		if (td_.total_microseconds() >= 1000000 / record_framerate_) {

			//	 determine time at start of write
			initialLoopTimestamp_ = boost::posix_time::microsec_clock::local_time();

			// Check the memory
			bool memory_ok = true;

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
			for (auto it = data_in_.begin(); it != data_in_.end(); it++)
#else
			for (std::map<int, std::vector<char> >::const_iterator it = data_in_.begin(); it != data_in_.end(); it++)
#endif
			{
				// Test the data
				if (m_files_.find(it->first) != m_files_.end()) {
					if (!m_files_[it->first]->check_memory(it->second.size())) {
						memory_ok = false;
						break;
					}
				}
			}
			// At least one file has not enough memory.
			// Close all the files and create a new one
			if (!memory_ok) {

				// Get the appendix to add to the video
				std::string appendix = DateTime::time2string();
				for (int i = 0; i < appendix.length(); i++)
				{
					if (appendix[i] == ':') appendix[i] = '_';
				}
				// callback to inform that a new file will be created
				if (callback_createfile_) {
					callback_createfile_(appendix);
				}

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
				for (auto it = m_files_.begin(); it != m_files_.end(); it++)
#else
				for (std::map<int, MemorizeFileManager* >::const_iterator it = m_files_.begin(); it != m_files_.end(); it++)
#endif		
				{
					it->second->release();
					it->second->generate(appendix, false);
				}
			}

			// Add the data
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
			for (auto it = data_in_.begin(); it != data_in_.end(); it++)
#else
			for (std::map<int, std::vector<char> >::const_iterator it = data_in_.begin(); it != data_in_.end(); it++)
#endif		
			{
				// Test if the file manager exists
				if (m_files_.find(it->first) != m_files_.end()) {
					// If able to write to disk
					if (m_files_[it->first]->push(it->second) ==
						kSuccess) {
						result_out = true;
					}
				}
			}

			//write previous and current frame timestamp to console
			if (verbose_) {
				std::cout << nextFrameTimestamp_ << " " << currentFrameTimestamp_ << " ";
			}

			// add 1second/framerate time for next loop pause
			nextFrameTimestamp_ = nextFrameTimestamp_ +
				boost::posix_time::microsec(1000000 / record_framerate_);

			// reset time_duration so while loop engages
			td_ = (currentFrameTimestamp_ - nextFrameTimestamp_);

			//determine and print out delay in ms, should be less than 1000/FPS
			//occasionally, if delay is larger than said value, correction will occur
			//if delay is consistently larger than said value, then CPU is not powerful
			// enough to capture/decompress/record/compress that fast.
			finalLoopTimestamp_ = boost::posix_time::microsec_clock::local_time();
			td1_ = (finalLoopTimestamp_ - initialLoopTimestamp_);
			delayFound_ = td1_.total_milliseconds();
			if (verbose_) {
				std::cout << delayFound_ << std::endl;
			}
		}
		under_writing_ = false;
	}
	return result_out;

#endif
}
// ----------------------------------------------------------------------------
void FileGeneratorManagerAsync::check() {
	std::cout << "FileGeneratorManagerAsync::check(): " << under_writing_ << 
		std::endl;//" " << number_addframe_requests_ << std::endl;
}
// ----------------------------------------------------------------------------
int FileGeneratorManagerAsync::push_data_can_replace(
	const std::map<int, std::vector<char> > &data_in) {

	bool write_success = false;

#ifdef BOOST_BUILD
	if (!under_writing_) {
		{
			boost::mutex::scoped_lock lock(mutex_, boost::try_to_lock);
			if (lock) {
				//++number_addframe_requests_;
#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
				for (auto it = data_in.begin(); it != data_in.end(); it++)
#else
				for (std::map<int, std::vector<char> >::const_iterator it = data_in.begin(); it != data_in.end(); it++)
#endif		
				{
					data_in_[it->first] = it->second;
				}
				write_success = true;
			}
		}
		//boost::thread* thr = new boost::thread(
		//	boost::bind(&FileGeneratorManagerAsync::procedure, this));
		if (write_success) {
			std::async(&FileGeneratorManagerAsync::procedure, this);
		}

		// used only for test to detect lost frames
		//container_future_.push(std::async(&FileGeneratorManagerAsync::procedure, this));
		//return kSuccess;
	}
#endif
	//return kFail;
	return write_success;
}
// ----------------------------------------------------------------------------
void FileGeneratorManagerAsync::close() {
#ifdef BOOST_BUILD
	while (under_writing_ ){ //|| number_addframe_requests_ > 0) {
		//std::cout << under_writing_ << " " << number_addframe_requests_ << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
#endif

#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
	for (auto it = m_files_.begin(); it != m_files_.end(); it++)
#else
	for (std::map<int, MemorizeFileManager* >::const_iterator it = m_files_.begin(); it != m_files_.end(); it++)
#endif		
	{
		delete it->second;
	}
	m_files_.clear();

	// used only for test to detect lost frames
	//// Container with the future result
	//while (!container_future_.empty())
	//{
	//	std::cout << "res:" << container_future_.front().get() << std::endl;
	//	container_future_.pop();
	//}
}
// ----------------------------------------------------------------------------
void FileGeneratorManagerAsync::set_verbose(bool verbose) {
	verbose_ = verbose;
}
// ----------------------------------------------------------------------------
void FileGeneratorManagerAsync::set_callback_createfile(
	cbk_fname_changed callback_createfile) {
	callback_createfile_ = callback_createfile;
}


} // namespace storedata
