/* @file sample_rawrecord.cpp
 * @brief Example of recording data in raw format.
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
 * @version 0.1.1.0
 * 
 */


#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include <opencv2/opencv.hpp>

//#define BOOST_BUILD
//#define BOOST_NO_CXX11_SCOPED_ENUMS
//#include <boost/filesystem.hpp>
//#undef BOOST_NO_CXX11_SCOPED_ENUMS

#include "record/record_headers.hpp"

// ----------------------------------------------------------------------------
namespace
{

/** @brief It creates a folder if necessary
*/
void create_folder(const std::string &folder) {
	std::filesystem::path dir(folder);
	if (std::filesystem::create_directory(dir)) {
		std::cout << "[+] Root::sanity_check: create " <<
			dir.string().c_str() << std::endl;
	} else {
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

/** @brief It records a video with a raw recorder
*/
int record_raw() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	// Store the data recorded
	storedata::RawRecorder pr;
	pr.set_callback_createfile(std::bind(&name_changed,
		std::placeholders::_1));
	pr.setup("data_recordraw\\record_", ".dat", 10000, 100);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		//std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 " + std::to_string((float)rand() / RAND_MAX);
		pr.record(msg);

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}

} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[])
{
	create_folder("data_recordraw");
	create_folder("unpack");

	// record a video with a raw data saver
	if (true)
	{
		record_raw();
		// Record a video and save some simple data
		storedata::RawRecorder pr;
		std::string fname = "data_recordraw\\record_" + global_fname + ".dat";
		std::cout << "[!] open: " << fname << std::endl;
		pr.read_all_raw(fname, 60);
	}

	return 0;
}
