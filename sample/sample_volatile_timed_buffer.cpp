#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include "buffer/buffer_headers.hpp"


void main()
{
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) return;

	vb::VolatileTimedBuffer small_buffer_;

	int ff = 0;
	int ss = 0;
	int mm = 0;
	int hh = 0;

	// Container with the margin buffer
	std::vector<vb::PtrMicrobuffer> vptr;

	double start = cv::getTickCount();
	bool continue_capture = true;
	bool bufferize = false;
	while (continue_capture) {
		cv::Mat m;
		vc >> m;
		if (m.empty()) continue;

		// Increase the timecode. It simulated a sync timecode
		// in the format hh:mm:ss:ff
		++ff;
		if (ff >= 30) { ++ss; ff = 0; }
		if (ss >= 60) { ++mm; ss = 0; }
		if (mm >= 60) { ++hh; mm = 0; }
		std::string timecode = std::to_string(hh) + ":" +
			std::to_string(mm) + ":" + std::to_string(ss) + ":" +
			std::to_string(ff);
		double tnow = (cv::getTickCount() - start) / cv::getTickFrequency();
		cv::putText(m, timecode + " " + std::to_string(tnow), cv::Point(50, 50), 0, 1,
			cv::Scalar(0, 0, 255), 2);

		if (bufferize) {
			if (!small_buffer_.add_forceexpand(tnow, timecode, m)) {
				std::cout << "buffer critical error" << std::endl;
			}
		}
		small_buffer_.clean(tnow, 1.5);
		std::cout << "Buffer size: " << small_buffer_.size() << std::endl;

		cv::imshow("m", m);
		char c = cv::waitKey(1);

		switch (c) {
		case 27:
			continue_capture = false;
			break;
		case 's':
			bufferize = true;
			break;
		case 'p':
			bufferize = false;
			break;
		case 'i':
			small_buffer_.get_ptr_containers(5, vptr);
			std::cout << "Copy the current small buffer: " << vptr.size() << std::endl;
			break;
		case 'o':
			vptr.clear();
			break;
		case 't':
			for (auto &it : vptr) {
				for (auto &it2 : *it) {
					std::cout << "> " << it2.first << " " << it2.second.first << std::endl;
				}
			}
		}
	}
}