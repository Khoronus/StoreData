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
void test_MemorizeVideoManager() {
	storedata::MemorizeVideoManager mvm;
	mvm.setup(100, "data\\MemorizeVideoManager_", 640, 480, 30);

	int num_file = 0;
	for (int i = 0; i < 255; ++i) {
		cv::Mat m(480, 640, CV_8UC3, cv::Scalar::all(i));
		
		if (mvm.push(m) <= 0) {
			// release locked file
			mvm.release();
			// try to generate a new file
			std::cout << "[!] generate a new file:" << num_file << std::endl;
			mvm.generate(std::to_string(num_file));
			++num_file;
			// push the data
			if (mvm.push(m) <= 0) {
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
void test_VideoGeneratorManagerAsync() {

	// File Generator manager
	storedata::VideoGeneratorManagerAsync vgm_;
	// File Generator parameters
	std::map<int, storedata::VideoGeneratorParams> vgp_;

	std::cout << "insert FileGeneratorParam" << std::endl;
	vgp_.insert(std::make_pair(0, storedata::VideoGeneratorParams()));
	std::string filename = "data\\VideoGeneratorManagerAsync_";
	std::cout << "filename: " << filename << std::endl;
	vgp_[0].set_filename(filename);
	vgp_[0].set_width(640);
	vgp_[0].set_height(480);
	unsigned int max_memory_allocable = 100000;
	int fps = 1000;
	vgm_.setup(max_memory_allocable, vgp_, fps);
	vgm_.set_callback_createfile(std::bind(&name_changed, std::placeholders::_1));

	for (int i = 0; i < 255; ++i) {
		cv::Mat m(480, 640, CV_8UC3, cv::Scalar::all(i));

		std::string s = "This is the sample line " + std::to_string(i) + '\n';
		std::vector<char> v;
		std::copy(s.begin(), s.end(), std::back_inserter(v));

		if (!vgm_.under_writing()) {
			// Prepare the container for the data to transmit
			std::map<int, cv::Mat> m_data;
			m_data[0] = m;
			if (!vgm_.push_data_write_not_guarantee_can_replace(m_data)) { std::cout << "lost:" << i << std::endl; }
		} else {
			std::cout << "[-] writing: " << i << std::endl;
		}
	}
	vgm_.close();
}

//-----------------------------------------------------------------------------
void main() {
	test_MemorizeVideoManager();
	test_VideoGeneratorManagerAsync();
}