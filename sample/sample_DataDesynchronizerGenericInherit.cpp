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

	void close() {
		vw_.release();
		fout_.close();
		fout_.clear();
		do_set_fname_root_ = true;
	}

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
	void frecorder(std::unique_ptr<storedata::AtomicContainerDataInherit> &rcd) {

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
		if (rcd->getObject(0)) {
			// convert the binary in image
			//cv::Mat img = cv::Mat(size_image_, CV_8UC3, rcd->data);
			std::unique_ptr<cv::Mat> *p = static_cast<std::unique_ptr<cv::Mat>*>(rcd->getObject(0));

			filevideo_push_frame(vw_, fname_root_ + ".avi", 
				*p->get());

			// open a file with the frame information
			if (!fout_.is_open()) {
				fout_.open(fname_root_ + ".txt");
			}
			// calculates a timestamp
			std::string timestamp =
				storedata::DateTime::get_date_as_string() +
				" " + std::to_string(cv::getTickCount());
			fout_ << *static_cast<std::string*>(rcd->getObject(1)) << "|" << timestamp << std::endl;
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


//template <typename _Ty>
//class AtomicContainerDataInheritD : public storedata::AtomicContainerDataInherit
//{
//public:
//
//	AtomicContainerDataInheritD() {}
//	~AtomicContainerDataInheritD() {
//		std::cout << "disposeD" << std::endl;
//	}
//
//	void* getObject(int which) {
//		if (which == 0) return &k;
//		if (which == 1) return &msg_;
//		return nullptr;
//	}
//
//	void dispose() {
//		std::cout << "disposeD" << std::endl;
//		k.
//	}
//
//
//	void moveFrom(std::unique_ptr<_Ty> &obj) {
//		k = std::move(obj);
//	}
//	void moveTo(std::unique_ptr<_Ty> &obj) {
//		obj = std::move(k);
//	}
//
//	void set_msg(const std::string &msg) {
//		msg_ = msg;
//	}
//
//private:
//
//	std::unique_ptr<_Ty> k;
//	std::string msg_;
//};


class AtomicContainerDataInheritD : public storedata::AtomicContainerDataInherit
{
public:

	AtomicContainerDataInheritD() {}
	~AtomicContainerDataInheritD() {
		//std::cout << "~AtomicContainerDataInheritD" << std::endl;
	}

	void* getObject(int which) {
		if (which == 0) return &k;
		if (which == 1) return &msg_;
		return nullptr;
	}

	/** @brief The data must be properly disposed otherwise a memory leak occurs
	*/
	void dispose() {
		//k->release();
		//k.reset();
	}


	void moveFrom(std::unique_ptr<cv::Mat> &obj) {
		k = std::move(obj);
	}
	void moveTo(std::unique_ptr<cv::Mat> &obj) {
		obj = std::move(k);
	}

	void set_msg(const std::string &msg) {
		msg_ = msg;
	}

private:

	std::unique_ptr<cv::Mat> k;
	std::string msg_;
};


class AtomicContainerDataInheritE : public storedata::AtomicContainerDataInherit
{
public:

	AtomicContainerDataInheritE() {}

	void* getObject(int which) {
		if (which == 0) return &k;
		if (which == 1) return &msg_;
		return nullptr;
	}

	void moveFrom(std::unique_ptr<cv::Mat> &obj) {
		k = std::move(obj);
	}
	void moveTo(std::unique_ptr<cv::Mat> &obj) {
		obj = std::move(k);
	}

	void set_msg(const std::string &msg) {
		msg_ = msg;
	}

private:

	std::unique_ptr<cv::Mat> k;
	std::string msg_;
};


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

		AtomicContainerDataInheritD acdt;
		acdt.moveFrom(ptr);

		if (ptr) cv::imshow("m", *ptr.get());
		std::unique_ptr<cv::Mat> *p = static_cast<std::unique_ptr<cv::Mat>*>(acdt.getObject(0));
		if (!p->get()->empty()) cv::imshow("t", *p->get());
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
	std::map<int, storedata::DataDesynchronizerGenericInherit> record_container;
	/** @brief Video data recorder which accepts data from DataDesynchronizer
	*/
	VideoDataRecorder vdr;
	record_container[0].set_cbk_func_inherit(std::bind(&VideoDataRecorder::frecorder, &vdr,
		std::placeholders::_1));

	vdr.set_fname_root("data\\sample_DataDesynchronizerGenericInherit");
	vdr.set_size_image(cv::Size(640, 480));

	double start = cv::getTickCount();
	bool continue_capture = true;
	bool bufferize = false;
	int num_frame = 0;
	while (continue_capture) {

		auto t1 = std::chrono::high_resolution_clock::now();

		auto ptr = std::make_unique<cv::Mat>();
		vc >> *ptr.get();
		if (ptr.get()->empty()) continue;

		if (ptr) {
			cv::imshow("m", *ptr);
		} else {
			cv::Mat m(50, 50, CV_8UC3, cv::Scalar(0, 255));
			cv::imshow("m", m);
		}

		// Save the current frame
		if (bufferize) {

			// [+] 20170812 Changed the record format
			std::string msg = "data\\F0_" + std::to_string(num_frame);
			// Add the record to save on a separate thread
			// https://stackoverflow.com/questions/17473900/unique-ptr-to-a-derived-class-as-an-argument-to-a-function-that-takes-a-unique-p
			// https://stackoverflow.com/questions/8114276/how-do-i-pass-a-unique-ptr-argument-to-a-constructor-or-a-function/8114913#8114913
			auto rcd0 =
				std::make_unique<AtomicContainerDataInheritD>();
			rcd0->set_msg(msg);
			rcd0->moveFrom(ptr);
			std::unique_ptr<storedata::AtomicContainerDataInherit> rcd1 = std::move(rcd0);
			record_container[0].push(rcd1);
			++num_frame;
		}
		//std::cout << "Size: " << record_container[0].size_about() << std::endl;

		//if (ptr) {
		//	cv::imshow("m", *ptr);
		//} else {
		//	cv::Mat m(50, 50, CV_8UC3, cv::Scalar(0,255));
		//	cv::imshow("m", m);
		//}
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
			record_container[0].close();
			vdr.close();
			break;
		}

		auto t2 = std::chrono::high_resolution_clock::now();

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	}

	record_container[0].set_save_boost(true);
	record_container[0].close();
}