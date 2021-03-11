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
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>

#include "record/record_headers.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief It creates a folder if necessary
*/
void create_folder(const std::string &folder) {
	boost::filesystem::path dir(folder);
	if (boost::filesystem::create_directory(dir)) {
		std::cout << "[+] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
	else {
		std::cout << "[-] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	}
}


/** @brief Change the name of the file.

Function from the callback
*/
std::string global_fname;
void name_changed(const std::string &fname) {
	std::cout << "name_changed: " << fname << std::endl;
	global_fname = fname;
}

/** @brief It records a video and a message
*/
int sample_PlayerRecorder_file_write() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::PlayerRecorder pr;
	pr.set_callback_createfile(std::bind(&name_changed,
		std::placeholders::_1));
	pr.setup_file("data\\record_PlayerRecorder_file_", ".dat", 
		100000000, 100);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		pr.record_file(curr, 1, msg);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}
	return 0;
}

/** @brief It records a video
*/
int sample_PlayerRecorder_video_write() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::PlayerRecorder pr;
	std::map<int, cv::Mat> sources;
	sources[0] = cv::Mat(480, 640, CV_8UC3);
	pr.setup_video(sources, "data\\record_PlayerRecorder_video_", 30 * 10, 30, 30);

	bool do_continue = true;
	while (do_continue) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		sources[0] = curr;
		pr.record_video(sources);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		char c = cv::waitKey(1);

		switch (c) {
		case 27:
		case 'q':
			do_continue = false;
			break;
		case '0':
			{
				pr.close();
				pr.setup_video(sources, "data\\record_PlayerRecorder_video2_", 30 * 10, 30, 30);
			}
			break;
		}
	}
	return 0;
}


} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[])
{
	create_folder("data");
	create_folder("unpack");

	// example to save/read video and message in binary file
	{
		// record a video
		sample_PlayerRecorder_file_write();

		// Record a video and save some simple data
		storedata::PlayerRecorder pr;
		//global_fname = "2020_04_06_03_27_01";
		pr.read_file("data\\record_PlayerRecorder_file_" + global_fname + ".dat", 60);
		unsigned int index_start = 0;
		pr.unpack("data\\record_PlayerRecorder_file_" + global_fname + ".dat", 60, "unpack", index_start);
	}

	// example to save video async
	{
		sample_PlayerRecorder_video_write();
	}

	return 0;
}
