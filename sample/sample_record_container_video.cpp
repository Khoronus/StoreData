#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "record/inc/record/recordcontainervideo.hpp"

void main()
{
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) return;

	/** @brief Container with the data to record
	*/
	std::map<int, RecordContainerVideo> record_container;
	//record_container[0].set_fname_root("data\\sample_RecordContainerVideo");
	//record_container[0].set_size_image(cv::Size(640, 480));

	double start = cv::getTickCount();
	bool continue_capture = true;
	bool bufferize = false;
	int num_frame_buffer = 0;
	long long duration_total = 0;
	long long duration_sum_elems = 0;
	int num_frame = 0;
	while (continue_capture) {

		auto t1 = std::chrono::high_resolution_clock::now();

		cv::Mat m;
		vc >> m;
		if (m.empty()) continue;

		// Save the current frame
		if (bufferize) {

			// [+] 20170812 Changed the record format
			std::string fname = "data\\F0_" + std::to_string(num_frame_buffer);
			// Add the record to save on a separate thread
			RecordContainerData rcd0;
			rcd0.copyFrom(m.data, m.cols * m.rows * (m.step / m.cols));
			record_container[0].push(fname, rcd0);
			++num_frame_buffer;
		}

		cv::imshow("m", m);
		char c = cv::waitKey(1);

		switch (c) {
		case 27:
			continue_capture = false;
			break;
		case 's':
			bufferize = true;
			record_container[0].set_fname_root("data\\sample_RecordContainerVideo");
			record_container[0].set_size_image(cv::Size(640, 480));
			record_container[0].start();
			break;
		case 'p':
			bufferize = false;
			record_container[0].set_save_boost(true);
			record_container[0].close(10, 100);
			break;
		}
		++num_frame;

		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		duration_total += duration;
		duration_sum_elems += 1;
		if (num_frame % 30 == 0) {
			std::cout << "Size: " << record_container[0].size_about() << " " << record_container[0].size_about_micro() << std::endl;
			std::cout << "ct: " << duration_total / duration_sum_elems << std::endl;
			duration_total = duration;
			duration_sum_elems = 1;
		}
	}

	record_container[0].set_save_boost(true);
	for (auto &it : record_container) {
		it.second.close(10, 100);
	}
}