#ifdef usethis
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

#endif // usethis

//#define TSafeQ
#ifdef TSafeQ

#include <atomic>
#include <thread>
#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>
// a thread-safe queue with a maximal size based on std::list<>::splice()
// https://www.educba.com/c-thread-safe-queue/
template <typename T>
class ThreadSafeQ
{
private:
	// check whether the Q is running or closed
	enum class State
	{
		OPEN,
		CLOSED
	};
	State state; // The current state
	size_t currSize; // The current size
	size_t maxSize; // The maximal size
	std::condition_variable cvPush, cvPop; // The condition variables to use for pushing/popping
	std::mutex mutex; // The mutex for locking the Q
	std::list<T> list; // The list that the Q is implemented with
public:
	// enum to return to check whether could push or pop or queue was closed
	enum QueueResult
	{
		OK,
		CLOSED
	};
	// create and initialize the Q with maximum size
	explicit ThreadSafeQ(size_t maxSize = 0) : state(State::OPEN), currSize(0), maxSize(maxSize)
	{}
	// Push data to Q, if queue is full then  blocks
	void push(T const& data)
	{
		// Creating temporary Q
		decltype(list) tmpl;
		tmpl.push_back(data);
		// Push with lock
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until space is there in the Q
			while (currSize == maxSize)
				cvPush.wait(lock);
			// Check whether the Q is closed or not and pushing is allowed
			if (state == State::CLOSED)
				throw std::runtime_error("The queue is closed and trying to push.");
			// Pushing to Q
			currSize += 1;
			list.splice(list.end(), tmpl, tmpl.begin());
			// popping thread to wake up
			if (currSize == 1u)
				cvPop.notify_one();
		}
	}
	// Push data to Q with rvalue reference
	void push(T&& data)
	{
		// Create temporary queue.
		decltype(list) tmpl;
		tmpl.push_back(data);
		// Push with lock
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until space is there in the Q
			while (currSize == maxSize)
				cvPush.wait(lock);
			// Check whether the Q is closed or not and pushing is allowed
			if (state == State::CLOSED)
				throw std::runtime_error("The queue is closed and trying to push.");
			// Pushing to Q
			currSize += 1;
			list.splice(list.end(), tmpl, tmpl.begin());
			// popping thread to wake up
			cvPop.notify_one();
		}
	}
	// Poping value from Q and write to var
	// If successful, OK is returned, else if the Q is empty and was closed, then CLOSED is returned
	QueueResult pop(T& var)
	{
		decltype(list) tmpl;
		// Pop data to the tmpl
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until there is data, if there is no data
			while (list.empty() && state != State::CLOSED)
				cvPop.wait(lock);
			// cannot return anything, if the Q was closed and the list is empty
			if (list.empty() && state == State::CLOSED)
				return CLOSED;
			// If data found
			currSize -= 1;
			tmpl.splice(tmpl.begin(), list, list.begin());
			// one pushing thread wake up
			cvPush.notify_one();
		}
		// data write to var
		var = tmpl.front();
		return OK;
	}
	// No pushing data when the queue is closed
	void close() noexcept
	{
		std::unique_lock<std::mutex> lock(mutex);
		state = State::CLOSED;
		// all consumers notify
		cvPop.notify_all();
	}
};
int main()
{
	unsigned const No_THREADS = 5;
	unsigned const CO = 3;
	std::vector<std::thread> producers, consumers;
	std::mutex cerrMutex;
	std::atomic<int> x(0);
	ThreadSafeQ<int> queue(6);
	// producers are creating
	for (unsigned i = 0; i < No_THREADS; ++i)
		producers.push_back(std::thread([&, i]() {
		for (unsigned j = 0; j < CO; ++j)
		{
			// I/O locked
			{
				std::lock_guard<std::mutex> lock(cerrMutex);
				std::cerr << "THREAD #" << i << " pushing " << i * No_THREADS + j << " into Queue.\n";
			}
			queue.push(i * No_THREADS + j);
		}
			}));
	// consumers are creating
	for (unsigned i = No_THREADS; i < 2 * No_THREADS; ++i)
		consumers.push_back(std::thread([&, i]() {
		int j = -1;
		ThreadSafeQ<int>::QueueResult result;
		while ((result = queue.pop(j)) != ThreadSafeQ<int>::CLOSED)
		{
			std::lock_guard<std::mutex> lock(cerrMutex);
			std::cerr << "THREAD #" << i << " received: " << j << " from Queue.\n";
		}
		// I/O locked
		{
			std::lock_guard<std::mutex> lock(cerrMutex);
			std::cerr << "THREAD #" << i << " is done.\n";
		}
			}));
	// Waiting for producers
	{
		std::lock_guard<std::mutex> lock(cerrMutex);
		std::cerr << "Queue is closing.\n";
	}
	for (auto& t : producers)
		t.join();
	//  queue is closed
	{
		std::lock_guard<std::mutex> lock(cerrMutex);
		std::cerr << "Queue is closing.\n";
	}
	queue.close();
	// Wait for consumers.
	{
		std::lock_guard<std::mutex> lock(cerrMutex);
		std::cerr << "Waiting for consumers...\n";
	}
	for (auto& t : consumers)
		t.join();
	return 0;
}


#endif // ThreadSafeQ

#define DataRec
#ifdef DataRec

#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <mutex>
#include <chrono>

#include <opencv2/opencv.hpp>

std::string gen_random(const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}


class DataRecorder
{
public:

	DataRecorder() : is_running_(false), do_continue_(true) {}
	~DataRecorder() {
		Stop();
		Join();
	}

	void PushData(const std::string& filename, const cv::Mat& img, const std::string& str) {
		// Acquire the lock.
		std::scoped_lock lock(mtx_);
		q_filename_.push(filename);
		q_images_.push(img);
		q_msg_.push(str);
	}

	bool PopData(std::string& filename, cv::Mat& img, std::string& str) {
		// Acquire the lock.
		std::scoped_lock lock(mtx_);
		if (q_images_.size()) return false;
		filename = q_filename_.front();
		str = q_msg_.front();
		img = q_images_.front();
		q_filename_.pop();
		q_msg_.pop();
		q_images_.pop();
		return true;
	}

	void Start() {
		t_recorder_ = std::thread(&DataRecorder::ThreadRecording, this);
	}

	void Stop() {
		do_continue_ = false;
	}

	void Join() {
		t_recorder_.join();
	}

	bool is_running() {
		return is_running_;
	}

	size_t exact_size() {
		std::scoped_lock lock(mtx_);
		return q_images_.size();
	}

	size_t approx_size() {
		return q_images_.size();
	}

	void ThreadRecording() {
		is_running_ = true;
		while (do_continue_) {
			std::string filename;
			std::string str;
			cv::Mat img;
			bool res = PopData(filename, img, str);
			if (res == false) {
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
				continue;
			}
			std::cout << "Pop:" << filename << " " << str << std::endl;
			cv::imshow("img", img);
			cv::waitKey(1);
		}
		is_running_ = false;
	}

private:
	std::mutex mtx_;
	std::thread t_recorder_;
	std::atomic_bool do_continue_;
	std::atomic_bool is_running_;

	std::queue<std::string> q_filename_;
	std::queue<cv::Mat> q_images_;
	std::queue<std::string> q_msg_;
};


// a thread-safe queue with a maximal size based on std::list<>::splice()
// https://www.educba.com/c-thread-safe-queue/
template <typename T>
class ThreadSafeQ
{
private:
	// check whether the Q is running or closed
	enum class State
	{
		OPEN,
		CLOSED
	};
	State state; // The current state
	size_t currSize; // The current size
	size_t maxSize; // The maximal size
	std::condition_variable cvPush, cvPop; // The condition variables to use for pushing/popping
	std::mutex mutex; // The mutex for locking the Q
	std::list<T> list; // The list that the Q is implemented with
public:
	// enum to return to check whether could push or pop or queue was closed
	enum QueueResult
	{
		OK,
		CLOSED
	};
	// create and initialize the Q with maximum size
	explicit ThreadSafeQ(size_t maxSize = 0) : state(State::OPEN), currSize(0), maxSize(maxSize)
	{}
	// Push data to Q, if queue is full then  blocks
	void push(T const& data)
	{
		// Creating temporary Q
		decltype(list) tmpl;
		tmpl.push_back(data);
		// Push with lock
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until space is there in the Q
			while (currSize == maxSize)
				cvPush.wait(lock);
			// Check whether the Q is closed or not and pushing is allowed
			if (state == State::CLOSED)
				throw std::runtime_error("The queue is closed and trying to push.");
			// Pushing to Q
			currSize += 1;
			list.splice(list.end(), tmpl, tmpl.begin());
			// popping thread to wake up
			if (currSize == 1u)
				cvPop.notify_one();
		}
	}
	// Push data to Q with rvalue reference
	void push(T&& data)
	{
		// Create temporary queue.
		decltype(list) tmpl;
		tmpl.push_back(data);
		// Push with lock
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until space is there in the Q
			while (currSize == maxSize)
				cvPush.wait(lock);
			// Check whether the Q is closed or not and pushing is allowed
			if (state == State::CLOSED)
				throw std::runtime_error("The queue is closed and trying to push.");
			// Pushing to Q
			currSize += 1;
			list.splice(list.end(), tmpl, tmpl.begin());
			// popping thread to wake up
			cvPop.notify_one();
		}
	}
	// Poping value from Q and write to var
	// If successful, OK is returned, else if the Q is empty and was closed, then CLOSED is returned
	QueueResult pop(T& var)
	{
		decltype(list) tmpl;
		// Pop data to the tmpl
		{
			std::unique_lock<std::mutex> lock(mutex);
			// wait until there is data, if there is no data
			while (list.empty() && state != State::CLOSED)
				cvPop.wait(lock);
			// cannot return anything, if the Q was closed and the list is empty
			if (list.empty() && state == State::CLOSED)
				return CLOSED;
			// If data found
			currSize -= 1;
			tmpl.splice(tmpl.begin(), list, list.begin());
			// one pushing thread wake up
			cvPush.notify_one();
		}
		// data write to var
		var = tmpl.front();
		return OK;
	}
	// No pushing data when the queue is closed
	void close() noexcept
	{
		std::unique_lock<std::mutex> lock(mutex);
		state = State::CLOSED;
		// all consumers notify
		cvPop.notify_all();
	}
	// Current approximate size
	size_t size() {
		return currSize;
	}
};


class DataRecorder2
{
public:

	DataRecorder2() : is_running_(false), do_continue_(true), q_filename_(10), q_images_(10), q_msg_(10) {}
	~DataRecorder2() {
		Stop();
		Join();
	}

	/** @brief Push new data
	*
	* The function push new data in the container only if it started.
	*/
	void PushData(const std::string& filename, const std::shared_ptr<cv::Mat>& img, const std::string& str) {
		if (is_running_) {
			// Acquire the lock.
			//std::scoped_lock lock(mtx_);
			q_filename_.push(filename);
			q_images_.push(img);
			q_msg_.push(str);
		}
	}

	void PopData(std::string& filename, std::shared_ptr<cv::Mat>& img, std::string& str) {
		// Acquire the lock.
		//std::scoped_lock lock(mtx_);
		q_filename_.pop(filename);
		q_images_.pop(img);
		q_msg_.pop(str);
	}

	void Start() {
		do_continue_ = true;
		t_recorder_ = std::thread(&DataRecorder2::ThreadRecording, this);
	}

	void Stop() {
		q_filename_.close();
		q_images_.close();
		q_msg_.close();
		do_continue_ = false;
	}

	void Join() {
		t_recorder_.join();
	}

	bool is_running() {
		return is_running_;
	}

	size_t approximate_size() {
		//std::scoped_lock lock(mtx_);
		return q_images_.size();
	}

	size_t approx_size() {
		return q_images_.size();
	}

	void ThreadRecording() {
		is_running_ = true;
		while (do_continue_) {
			std::string filename;
			std::string str;
			std::shared_ptr<cv::Mat> img;
			PopData(filename, img, str);
			//std::cout << "Pop:" << filename << " " << str << std::endl;

			// Save the D1
			std::ofstream file(filename + ".txt", std::ios::out | std::ios::binary);
			if (file.is_open())
			{
				file.write(str.c_str(), str.size());
				file.close();
				//std::cout << "File saved successfully." << std::endl;
			}
			else {
				//std::cerr << "Failed to open file." << std::endl;
			}

			// Save the image
			if (img && !img->empty()) {
				//cv::imshow("img", *img);
				//cv::waitKey(1);
				cv::imwrite(filename + ".png", *img);
			}
		}
		is_running_ = false;
	}

private:
	std::mutex mtx_;
	std::thread t_recorder_;
	std::atomic_bool do_continue_;
	std::atomic_bool is_running_;

	ThreadSafeQ<std::string> q_filename_;
	ThreadSafeQ<std::shared_ptr<cv::Mat>> q_images_;
	ThreadSafeQ<std::string> q_msg_;
};

int main(int argc, char* argv[]) {
	//cv::VideoCapture vc("D:\\workspace\\programs\\ThirdPartyLib\\ffmpeg-20200205-e6891d1-win64-static\\bin\\videouniv20220307\\IMG_0935.mp4");
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		return 0;
	}
	DataRecorder2 data_recorder;
	data_recorder.Start();

	// create a random message
	int num_frame = 0;
	while (true) {

		auto start = std::chrono::high_resolution_clock::now();

		// Code to measure elapsed time goes here
		cv::Mat m;
		vc >> m;
		if (m.empty()) break;

		std::string msg = gen_random(50);

		cv::imshow("m", m);
		//std::cout << "msg:[" << msg << "]" << std::endl;
		if (cv::waitKey(1) == 27) break;

		std::string fname = "raw_data/filename_" + std::to_string(num_frame);
		data_recorder.PushData(fname, std::make_shared<cv::Mat>(m), msg);

		std::cout << "approx_size:" << data_recorder.approx_size() << std::endl;

		++num_frame;

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

		std::cout << "Elapsed time: " << duration.count() << " microseconds" << std::endl;
	}
	data_recorder.Stop();

	int count = 0;
	while (data_recorder.is_running() && count < 100) {
		std::cout << "count:" << count << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
		++count;
	}
	std::cout << "is_running:" << data_recorder.is_running() << std::endl;

	return 0;
}

#endif // DataRec