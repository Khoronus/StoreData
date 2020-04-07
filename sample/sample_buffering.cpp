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

#include "buffer/buffer_headers.hpp"
#include "record/record_headers.hpp"

class MicroBufferObjDerived : public vb::MicroBufferObjBase
{
public:
	MicroBufferObjDerived() {}
	MicroBufferObjDerived(const std::string &msg, const cv::Mat &img) {
		msg_ = msg;
		img_ = img;
	}

	virtual void clone(MicroBufferObjDerived&obj) {
		obj.set_img(img_);
		obj.set_msg(msg_);
	};
	template <typename _Ty>
	_Ty& get_item(int id) {
		if (id == 0) return static_cast<_Ty>(msg_);
		if (id == 1) return static_cast<_Ty>(img_);
		return nullptr;
	}

	void set_msg(const std::string &msg) {
		msg_ = msg;
	}
	std::string& msg() {
		return msg_;
	}

	void set_img(const cv::Mat &img) {
		img_ = img;
	}
	cv::Mat& img() {
		return img_;
	}

private:

	std::string msg_;
	cv::Mat img_;
};

// ----------------------------------------------------------------------------
namespace
{


void test_recordcontainerfile_microbuffer() {
	/** @brief Container with the data to record
	*/
	std::map<int, storedata::RecordContainerFile> record_container;
	/** @brief Container with the small buffered data for margin
	*/
	std::map<int, vb::VolatileTimedBuffer> small_buffer;

	// Recorder ID
	int id = 0;
	// copy the secondary buffer in the primary recorder
	std::vector<vb::PtrMicrobuffer> vptr;
	small_buffer[id].get_ptr_containers(100, vptr);
	std::cout << "Transfer: " << vptr.size() << std::endl;
	record_container[id].push(vptr, false, -1);

	// boost the saving if the request to save is false
	// and complete the action
	record_container[id].set_save_boost(true);

	double t_internal = 0.123; // internal timestamp
	double save_frame_expire_time_sec = 500; // 

	std::string fname = "fname_test.txt";
	cv::Mat src(50, 50, CV_8UC3, cv::Scalar(0, 255, 0));
	// Add in the secondary buffer only if the current frame is not
	// required to be saved
	small_buffer[id].add_forceexpand(t_internal, 
		std::make_shared<MicroBufferObjDerived>(MicroBufferObjDerived(fname, src)));

	vptr.clear();
	small_buffer[id].get_ptr_containers(100, vptr);
	std::cout << "Transfer: " << vptr.size() << std::endl;
	record_container[id].push(vptr, false, -1);

	// Clean size of the frame expire
	small_buffer[id].clean(t_internal, save_frame_expire_time_sec);
}

/** @brief It record some image data
*/
void test_recordcontainerfile_mainbuffer() {

	/** @brief Container with the data to record
	*/
	std::map<int, storedata::RecordContainerFile> record_container;
	int id = 0;

	bool res = record_container[id].start();
	if (!res) {
		std::cout << "unable to start the record" << std::endl;
		return;
	}

	for (int i = 0; i < 100; ++i) {
		std::cout << i << std::endl;
		// expected buffer size
		std::string msg = "Buffering size: " +
			std::to_string(record_container[id].size_about());
		// Add the record to save on a separate thread
		storedata::RecordContainerData rcd;
		cv::Mat img(50, 50, CV_8UC3);
		rcd.copyFrom(img.data, img.cols * img.rows * img.channels());
		record_container[id].push("data\\" + std::to_string(i) + ".dat", 
			rcd, false, -1);
	}
	record_container[id].stop();
	record_container[id].wait_until_buffer_is_empty(1000, 5);
}

} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[], char *window_name)
{
	test_recordcontainerfile_microbuffer();
	test_recordcontainerfile_mainbuffer();
	return 0;
}
