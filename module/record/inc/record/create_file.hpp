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

#include <future>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <queue>

#define BOOST_BUILD
#ifdef BOOST_BUILD

// Include Boost headers for system time and threading
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/thread.hpp"

#endif

#include "record_defines.hpp"
#include "storedata_time.hpp"
#include "storedata_typedef.hpp"
#include "create_base.hpp"

namespace storedata
{

/** @brief Class to memorize a file until memory is availeble.
*/
class MemorizeFileManager : public MemorizeManagerBase
{
  public:

	  STOREDATA_RECORD_EXPORT MemorizeFileManager();

	  STOREDATA_RECORD_EXPORT ~MemorizeFileManager();

	  /** @brief Release the current stream
	  */
	  STOREDATA_RECORD_EXPORT void release() override;

	  /** @brief Setup the data

		  @param[in] memory_max_allocable Max memory that can be recorded in 
		                                  the file.
		  @param[in] filename Name of the file (root). An appendix will be add.
		  @param[in] extension Extension add to the end of the file (i.e. ".dat")

	  */
	  STOREDATA_RECORD_EXPORT void setup(
		  size_t memory_max_allocable,
		  const std::string &filename,
		  const std::string &dot_extension);

	  /** @brief Generate a target binary file.

		@param[in] appendix C
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
	  /** @brief Extension to the file
	  */
	  std::string dot_extension_;
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
	void set_dot_extension(const std::string &dot_extension) {
		dot_extension_ = dot_extension;
	}
	const std::string& dot_extension() const {
		return dot_extension_;
	}
private:

	std::string filename_;
	std::string dot_extension_;
};


/** @brief Class to manage the writing of the data in a file asynchronously.
	
	This class has a potential issue to lose or overwrite the data
	before it is saved.

	This class allows the loss of frames or data during the writing.
	However, it is guarantee that during the writing, the data is not
	modified.

	@brief ThreadSafe
	@Warning Data writing is not guarantee. Not feedback returned.
*/
class FileGeneratorManagerAsync
{
public:

	STOREDATA_RECORD_EXPORT FileGeneratorManagerAsync();

	STOREDATA_RECORD_EXPORT ~FileGeneratorManagerAsync();

	/** @brief Setup the data to memorize

		Setup the data to memorize.
		@param[in] max_memory_allocable Maximum amount of memory allowed for a 
		                                file.
		@param[in] fgp Container with file generator parameters. The total 
		               number of files that can be created at one time is equal
					   to the number of instances used (same ID).
	*/
	STOREDATA_RECORD_EXPORT int setup(
		unsigned int max_memory_allocable,
		std::map<int, FileGeneratorParams> &fgp, int record_framerate);

	/** @brief Check if the video stream is in writing mode
	*/
	STOREDATA_RECORD_EXPORT bool under_writing();

	/** @brief Function to add the frames to file. It is run in a separate
	           thread.
	*/
	STOREDATA_RECORD_EXPORT bool procedure();

	STOREDATA_RECORD_EXPORT void check();

	/** @brief Try to push the data to a designed file writer.

		Try to push the data to a designed file writer.
		@param[in] data_in The data to save in a file. The data_in key is used 
		                   to select which file writer will be used.
	*/
	STOREDATA_RECORD_EXPORT int push_data_write_not_guarantee_can_replace(
		const std::map<int, std::vector<char> > &data_in);

	/** @brief Close the file
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
	int record_framerate_;

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
