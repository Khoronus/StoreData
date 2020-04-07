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
	void frecorder(const std::string &msg, storedata::AtomicContainerData &rcd) {

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
		if (rcd.data) {
			// convert the binary in image
			cv::Mat img = cv::Mat(size_image_, CV_8UC3, rcd.data);
			filevideo_push_frame(vw_, fname_root_ + ".avi", img);

			// open a file with the frame information
			if (!fout_.is_open()) {
				fout_.open(fname_root_ + ".txt");
			}
			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() +
				" " + std::to_string(cv::getTickCount());
			fout_ << msg << "|" << timestamp << std::endl;
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

	virtual void getFrom() = 0;
};

class DerivedA : public BaseClass
{
public:

	DerivedA() {}

	void getFrom() {
		std::cout << "DerivedA" << std::endl;
	}
};

class DerivedB : public BaseClass
{
public:

	DerivedB() {}

	void getFrom() {
		std::cout << "DerivedB" << std::endl;
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
		size_bytes = src_size_bytes;
		data = malloc(size_bytes);
		memcpy(data, src, size_bytes);
	}
	void copyFrom(AtomicContainerDataTest &obj) {
		if (data) dispose();
		size_bytes = obj.size_bytes;
		data = malloc(size_bytes);
		memcpy(data, obj.data, obj.size_bytes);
	}
	void dispose() {
		if (data) { free(data); data = nullptr; }
	}

};

void func(BaseClass &bc) {
	std::cout << typeid(bc).name() << std::endl;
	bc.getFrom();
}

void test_atomic_container() {

	{
		DerivedA a;
		func(a);
		DerivedB b;
		func(b);
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


void main()
{
	//test_atomic_container();
	//return;
	//read_saved_data();
	//return;
	//test();
	//test_binary("fname.data");
	//return;

	cv::VideoCapture vc(0);
	if (!vc.isOpened()) return;

	/** @brief Container with the data to record
	*/
	std::map<int, storedata::DataDesynchronizerGeneric> record_container;
	/** @brief Video data recorder which accepts data from DataDesynchronizer
	*/
	VideoDataRecorder vdr;
	record_container[0].set_cbk_func(std::bind(&VideoDataRecorder::frecorder, &vdr, 
		std::placeholders::_1, std::placeholders::_2));

	vdr.set_fname_root("data\\sample_DataDesynchronizerGeneric");
	vdr.set_size_image(cv::Size(640, 480));

	double start = cv::getTickCount();
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
			storedata::AtomicContainerData rcd0;
			rcd0.copyFrom(m.data, m.cols * m.rows * (m.step / m.cols));
			record_container[0].push(msg, rcd0);
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
}