/* @file main.cpp
 * @brief Main file with the example for the hog descriptor and visualization.
 *
 * @section LICENSE
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
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


#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "record/record_headers.hpp"

/** @brief It tests the MemorizeFileManager (sync recorder)
*/
void test_MemorizeFileManager() {
	storedata::MemorizeFileManager mfm;
	mfm.setup(200, "data\\MemorizeFileManager_", ".dat");

	int num_file = 0;
	for (int i = 0; i < 100; ++i) {
		std::string s = "This is the sample line " + std::to_string(i) + '\n';
		std::vector<char> v;
		std::copy(s.begin(), s.end(), std::back_inserter(v));
		
		if (mfm.push(v) <= 0) {
			// release locked file
			mfm.release();
			// try to generate a new file
			std::cout << "[!] generate a new file:" << num_file << std::endl;
			mfm.generate(std::to_string(num_file), false);
			++num_file;
			// push the data
			if (mfm.push(v) <= 0) {
				std::cout << "[-] error in the file generation." << std::endl;
			}
		}
	}
}

/** @brief Function called when a new file is created
*/
void name_changed(const std::string &fname) {
	std::cout << "The new filename is: " << fname << std::endl;
}

/** @brief Test FileGeneratorManagerAsync
*/
void test_FileGeneratorManagerAsync() {

	// File Generator manager
	storedata::FileGeneratorManagerAsync fgm_;
	// File Generator parameters
	std::map<int, storedata::FileGeneratorParams> fgp_;

	std::cout << "insert FileGeneratorParam" << std::endl;
	fgp_.insert(std::make_pair(0, storedata::FileGeneratorParams()));
	std::string filename = "data\\FileGeneratorManagerAsync_";
	std::string dot_ext = ".dat";
	std::cout << "filename: " << filename << std::endl;
	fgp_[0].set_filename(filename);
	fgp_[0].set_dot_extension(dot_ext);
	unsigned int max_memory_allocable = 100000;
	int fps = 1000;
	fgm_.setup(max_memory_allocable, fgp_, fps);
	fgm_.set_callback_createfile(std::bind(&name_changed, std::placeholders::_1));

	for (int i = 0; i < 1000; ++i) {
		std::string s = "This is the sample line " + std::to_string(i) + '\n';
		std::vector<char> v;
		std::copy(s.begin(), s.end(), std::back_inserter(v));

		if (!fgm_.under_writing()) {
			// Prepare the container for the data to transmit
			std::map<int, std::vector<char> > m_data;
			m_data[0] = v;
			if (!fgm_.push_data_can_replace(m_data)) { std::cout << "lost:" << i << std::endl; }
		} else {
			std::cout << "[-] writing: " << i << std::endl;
		}
	}
	fgm_.close();
}

//-----------------------------------------------------------------------------
void main() {
	//test_MemorizeFileManager();
	test_FileGeneratorManagerAsync();
}