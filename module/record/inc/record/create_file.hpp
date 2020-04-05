/* @file create_file.hpp
* @brief Header of the defined class.
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

#ifndef STOREDATA_RECORD_CREATE_FILE_HPP__
#define STOREDATA_RECORD_CREATE_FILE_HPP__

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#define BOOST_BUILD
#ifdef BOOST_BUILD

// Include Boost headers for system time and threading
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/thread.hpp"

#endif

#include "record_defines.hpp"
#include "storedata_time.hpp"
#include "storedata_typedef.hpp"

namespace storedata
{

const int kSuccess = 1;
const int kFail = 0;
const int kOutOfMemory = -1;
const int kDataIsEmpty = -2;
const int kFileIsNotOpen = -3;

/** @brief Class to capture video with a maximum size.
*/
class MemorizeFileManager
{
  public:

	  STOREDATA_RECORD_EXPORT MemorizeFileManager();

	  STOREDATA_RECORD_EXPORT ~MemorizeFileManager();

	  /** @brief Release the current stream
	  */
	  STOREDATA_RECORD_EXPORT void release();

	  /** @brief Setup the data
	  */
	  STOREDATA_RECORD_EXPORT void setup(size_t memory_max_allocable,
		  const std::string &filename);

	  /** @brief Generate a target binary file.
	  */
	  STOREDATA_RECORD_EXPORT int generate(const std::string &appendix, bool append);

	  /** @brief Check if the current video can memorize the frame.
	  */
	  STOREDATA_RECORD_EXPORT int check_memory(size_t size);

	  /** @brief Push an image in the video container
	  */
	  STOREDATA_RECORD_EXPORT int push(const std::vector<char> &data);

  private:

	  /** @brief Path and name of the file to memorize
	  */
	  std::string filename_;
	  /** @brief Memory allocated
	  */
	  size_t memory_expected_allocated_;
	  /** @brief Max memory allocable
	  */
	  size_t memory_max_allocable_;

	  /** @brief Stream to the output file.
	  */
	  std::ofstream fout_;

	/** @brief Function to get the file size.
	*/
	static std::ifstream::pos_type filesize(const std::string &filename)
	{
		std::ifstream in(filename.c_str(), std::ifstream::ate | std::ifstream::binary);
		return in.tellg();
	}



};


/** @brief Class to define the property for each video
*/
class FileGeneratorParams
{
public:

	void set_filename(const std::string &filename) {
		filename_ = filename;
	}
	const std::string& filename() const {
		return filename_;
	}
private:

	std::string filename_;
};


/** @brief Class to manage the 
*/
class FileGeneratorManagerAsync
{
public:

	STOREDATA_RECORD_EXPORT FileGeneratorManagerAsync();

	STOREDATA_RECORD_EXPORT ~FileGeneratorManagerAsync();

	/** @brief Setup the data to memorize
	*/
	STOREDATA_RECORD_EXPORT int setup(
		unsigned int max_memory_allocable,
		std::map<int, FileGeneratorParams> &vgp, int framerate);

	/** @brief Check if the video stream is in writing mode
	*/
	STOREDATA_RECORD_EXPORT bool under_writing();

	/** @brief Function to add the frames to videos. It is run in a separate
	           thread.
	*/
	STOREDATA_RECORD_EXPORT void procedure();

	STOREDATA_RECORD_EXPORT void check();

	/** @brief Try to push the frame data in a file.

		Try to push the frame data in a file.
		@param[in] data_in The data to save in a file. The data_in key is used to save the file.
	*/
	STOREDATA_RECORD_EXPORT int push_data(const std::map<int, std::vector<char> > &data_in);

	/** @brief Close the video
	*/
	STOREDATA_RECORD_EXPORT void close();

	/** @brief Set the verbose status
	*/
	STOREDATA_RECORD_EXPORT void set_verbose(bool verbose);

	/** @brief It sets the callback for the function that create a new file
	*/
	STOREDATA_RECORD_EXPORT void set_callback_createfile(
		cbk_fname_changed callback_createfile);

  private:

#ifdef BOOST_BUILD
	/** @brief Writing thread
	*/
	boost::thread* thr_;
	boost::mutex mutex_;
#endif

	/** @brief If TRUE the variable is under writing.
	*/
	bool under_writing_;

	// set framerate to record and capture at
	int framerate_;

	// Get the properties from the camera
	//double width_;
	//double height_;

	// Create a matrix to keep the retrieved frame
	std::map<int, std::vector<char> > data_in_;

	//vars
#ifdef BOOST_BUILD
	boost::posix_time::time_duration td_, td1_;
	boost::posix_time::ptime nextFrameTimestamp_, 
		currentFrameTimestamp_, initialLoopTimestamp_, finalLoopTimestamp_;
#endif
	int delayFound_;
	int totalDelay_;

	/** @brief Create files
	*/
	std::map<int, MemorizeFileManager* > m_files_;

	/** @brief If TRUE it is in verbose mode
	*/
	bool verbose_;

	/** @brief Callback function when a file is created
	*/
	cbk_fname_changed callback_createfile_;
};

} // namespace storedata

#endif // STOREDATA_RECORD_CREATE_FILE_HPP__
