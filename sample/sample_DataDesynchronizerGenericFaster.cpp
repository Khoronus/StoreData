/* @file sample_DataDesynchronizerGenericFaster.cpp
 * @brief Example to desynchronize data for other operations (i.e. recording).
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

#include <opencv2/opencv.hpp>

#include "buffer/buffer_headers.hpp"
#include "record/record_headers.hpp"


/** @brief Class to record all the frames currently captured

	@ThreadSafe : NO
*/
class VideoDataRecorder
{

public:

	VideoDataRecorder() {}

	/** @brief Set the size of the image
	*/
	void set_size_image(const cv::Size &size_image) {
		req_size_image_ = size_image;
		do_set_size_image_ = true;
	}

	/** @brief It sets the filename root used to record the data
	*/
	void set_fname_root(const std::string &fname_root) {
		req_fname_root_ = fname_root;
		do_set_fname_root_ = true;
	}

	/** @brief Function to record the data.

	Function to record the data.
	*/
	void frecorder(std::unique_ptr<storedata::AtomicContainerDataFaster> &rcd) {

		// it solves the requests
		if (do_set_fname_root_) {
			vw_.release();
			fout_.close();
			fout_.clear();
			fname_root_ = req_fname_root_;
			do_set_fname_root_ = false;
		}
		if (do_set_size_image_) {
			size_image_ = req_size_image_;
			do_set_size_image_ = false;
		}

		// check potential errors
		if (size_image_.width == 0 || size_image_.height == 0 ||
			fname_root_.size() == 0) {
			std::cout << "[e] size_image_:" << size_image_ <<
				" fname_root_:" << fname_root_ << std::endl;
			return;
		}


		// Process the data
		if (rcd->data()) {
			// convert the binary in image
			cv::Mat img = cv::Mat(size_image_, CV_8UC3, rcd->data());
			filevideo_push_frame(vw_, fname_root_ + ".avi", img);

			// open a file with the frame information
			if (!fout_.is_open()) {
				fout_.open(fname_root_ + ".txt");
			}
			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() +
				" " + std::to_string(cv::getTickCount());
			fout_ << rcd->unique_msg() << "|" << timestamp << std::endl;
		}
	}


private:

	/** @brief Video recorder
	*/
	cv::VideoWriter vw_;
	/** @brief File recorder
	*/
	std::ofstream fout_;

	/** @brief If true, it sets the image size
	*/
	bool do_set_size_image_;
	/** @brief If true, it sets the fname root
	*/
	bool do_set_fname_root_;

	/** @brief Size of the image to record
	*/
	cv::Size req_size_image_;
	cv::Size size_image_;

	/** @brief Name of the video and meta to save
	*/
	std::string req_fname_root_;
	std::string fname_root_;

	/** @brief It saves a video frame
	*/
	void filevideo_push_frame(cv::VideoWriter &vw, const std::string &fname,
		cv::Mat &img) {
		if (!vw.isOpened()) {
			vw.open(fname,
				cv::VideoWriter::fourcc('D', 'I', 'V', 'X'), 30, img.size(),
				true);
		}
		vw << img;
	}

};

class BaseClass
{
public:

	BaseClass() {};
	virtual ~BaseClass() {};

	virtual void* getObject() = 0;
};

class DerivedA : public BaseClass
{
public:

	DerivedA() { k = 3; }

	int k;
	void* getObject() {
		return &k;
	}
	void getFrom() {
		std::cout << "DerivedA" << std::endl;
	}
};

class DerivedB : public BaseClass
{
public:

	DerivedB() { k = "ciao"; }

	std::string k;
	void* getObject() {
		return &k;
	}
	void getFrom() {
		std::cout << "DerivedB" << std::endl;
	}
};

template <typename _Ty>
class DerivedC : public BaseClass
{
public:

	DerivedC() { }

	std::unique_ptr<_Ty> k;
	void* getObject() {
		return &k;
	}

	void moveFrom(std::unique_ptr<_Ty> &obj) {
		k = std::move(obj);
	}
	void moveTo(std::unique_ptr<_Ty> &obj) {
		obj = std::move(k);
	}
};


/** @brief Container data to transfer data between threads. It copies the data.
*/
template <typename _Ty>
struct AtomicContainerDataTest
{
	std::unique_ptr<_Ty> data;

	AtomicContainerDataTest() : data(nullptr) {}
	void moveFrom(std::unique_ptr<_Ty> &obj) {
		data = std::move(obj);
	}
	void moveTo(std::unique_ptr<_Ty> &obj) {
		obj = std::move(data);
	}

	void copyFrom(const void* src, size_t src_size_bytes) {
		if (data) dispose();
		data = malloc(src_size_bytes);
		memcpy(data, src, src_size_bytes);
	}
	void copyFrom(AtomicContainerDataTest &obj) {
		if (data) dispose();
		data = malloc(obj.size_bytes);
		memcpy(data, obj.data, obj.size_bytes);
	}
	void dispose() {
		if (data) { free(data); data = nullptr; }
	}

};


void func(BaseClass &bc) {
	std::cout << typeid(bc).name() << std::endl;
	if (strcmp(typeid(bc).name(),"class DerivedA") == 0) {
		int *s = static_cast<int*>(bc.getObject());
		std::cout << "k:" << *s << std::endl;
	}
	if (strcmp(typeid(bc).name(), "class DerivedB") == 0) {
		std::string *s = static_cast<std::string*>(bc.getObject());
		std::cout << "k:" << *s << std::endl;
	}
	if (strcmp(typeid(bc).name(), "class DerivedC<class cv::Mat>") == 0) {
		auto *s = static_cast<std::unique_ptr<cv::Mat>*>(bc.getObject());
		std::cout << "k:" << s->get()->size() << std::endl;
	}
}

void test_atomic_container() {

	{
		DerivedA a;
		func(a);
		DerivedB b;
		func(b);
		DerivedC<cv::Mat> c;
		std::unique_ptr<cv::Mat> img = std::make_unique<cv::Mat>(200, 300, CV_8UC3);
		c.moveFrom(img);
		func(c);
	}

	{
		std::unique_ptr<char[]> ptr;
		char* arr = new char[100];
		strncpy(arr, "This is a sample string", strlen("This is a sample string"));
		std::cout << "arr: " << arr << std::endl;

		ptr = std::move(std::unique_ptr<char[]>(arr));
		std::cout << "ptr: " << ptr.get() << std::endl;
		std::cout << "arr: " << arr << std::endl;
	}

	{
		std::unique_ptr<char[]> ptr;
		char* arr = new char[100];
		strncpy(arr, "This is a sample string", strlen("This is a sample string"));
		std::cout << "arr: " << arr << std::endl;
			
		ptr = std::move(std::unique_ptr<char[]>(arr));
		std::cout << "ptr: " << ptr.get() << std::endl;
		std::cout << "arr: " << arr << std::endl;
	}

	cv::VideoCapture vc(0);
	if (!vc.isOpened()) return;

	bool continue_capture = true;
	bool bufferize = false;
	int num_frame = 0;
	std::unique_ptr<cv::Mat> ptr;
	while (continue_capture) {
		ptr = std::unique_ptr<cv::Mat>(new cv::Mat());
		vc >> *ptr.get();
		if (ptr.get()->empty()) continue;

		AtomicContainerDataTest<cv::Mat> acdt;
		acdt.moveFrom(ptr);

		if (ptr) cv::imshow("m", *ptr.get());
		if (acdt.data) cv::imshow("t", *acdt.data.get());
		cv::waitKey();
	}
}

// ----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	//test_atomic_container();
	//return;
	//read_saved_data();
	//return;
	//test();
	//test_binary("fname.data");
	//return;

	cv::VideoCapture vc(0);
	if (!vc.isOpened()) return -1;

	/** @brief Container with the data to record
	*/
	std::map<int, storedata::DataDesynchronizerGenericFaster> record_container;
	/** @brief Video data recorder which accepts data from DataDesynchronizer
	*/
	VideoDataRecorder vdr;
	record_container[0].set_cbk_func_faster(std::bind(&VideoDataRecorder::frecorder, &vdr,
		std::placeholders::_1));

	vdr.set_fname_root("data\\sample_DataDesynchronizerGenericFaster");
	vdr.set_size_image(cv::Size(640, 480));

	bool continue_capture = true;
	bool bufferize = false;
	int num_frame = 0;
	while (continue_capture) {
		cv::Mat m;
		vc >> m;
		if (m.empty()) continue;

		// Save the current frame
		if (bufferize) {

			// [+] 20170812 Changed the record format
			std::string msg = "data\\F0_" + std::to_string(num_frame);
			// Add the record to save on a separate thread
			auto rcd0 = 
				std::make_unique<storedata::AtomicContainerDataFaster>();
			rcd0->set_unique_msg(msg);
			rcd0->copyFrom(m.data, m.cols * m.rows * (m.step / m.cols));
			record_container[0].push(rcd0);
			++num_frame;
		}
		std::cout << "Size: " << record_container[0].size_about() << std::endl;

		cv::imshow("m", m);
		char c = cv::waitKey(1);

		switch (c) {
		case 27:
			continue_capture = false;
			break;
		case 's':
			bufferize = true;
			record_container[0].start();
			break;
		case 'n':
			bufferize = false;
			record_container[0].stop();
			break;
		case 'p':
			bufferize = false;
			break;
		}
	}

	record_container[0].set_save_boost(true);
	for (auto &it : record_container) {
		it.second.stop();
		if (!it.second.wait_until_is_not_ready(100, 5)) {
			std::cout << "Failed to stop " << it.first << std::endl;
		}
	}
	
	return 0;
}
