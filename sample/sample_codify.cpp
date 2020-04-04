#include "codify/codify.hpp"
#include "UDPKomatsu2Arm.hpp"

int data_block_size = 1;
int data_block_offset = 1;

/** @readme Possible DECODE sample
*/
void read2(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "expected: " << argv[0] << " <fname_video>" << std::endl;
		return;
	}
	cv::VideoCapture vc(argv[1]);// "E:\\ImPACT\\record_2018-10-31.07_42_43.avi");
	if (!vc.isOpened()) {
		std::cout << "Unable to open: " << argv[1] << std::endl;
		return;
	}

	UDPKomatsu2Arm udp_komatsu_2arm;

	cv::Mat tmp(2048, 2048, CV_8UC3);
	//cv::Mat m_data_block(30, frame_width, CV_8UC3, cv::Scalar::all(0));
	cv::Mat m_data_block;
	size_t msg_len_max_bytes = 500;
	storedata::codify::CodifyImage::estimate_data_size(tmp, 
		msg_len_max_bytes, data_block_size,
		data_block_offset, m_data_block);

	// Create a binary file where to save the received data
	std::string str = "_test";
	std::ofstream myFile;
	myFile.open("data" + str + ".bin", std::ios::out | std::ios::binary);
	char msg[64] = "UDPKomatsu2Arm v1.0.0 64byte";
	myFile.write(msg, 64);

	bool write_img = true;
	if (vc.isOpened()) {
		int num_frame = 0;
		while (true)
		{
			cv::Mat m;
			vc >> m;
			if (m.empty()) {
				break;
			}
			if (write_img) {
				write_img = false;
				cv::imwrite("m.png", m);
			}
			int x = data_block_offset, y = data_block_offset;

			unsigned char data[2048];
			size_t len = 0;
			cv::imwrite("data.png", m(cv::Rect(0, tmp.rows, tmp.cols, 
				m.rows - tmp.rows)));
			storedata::codify::CodifyImage::image2data(m(cv::Rect(0, tmp.rows, 
				tmp.cols, m.rows - tmp.rows)), x, y, data_block_size, 
				sizeof(len), data_block_offset, data, 2048, len);
			//std::cout << "msg_out: " << msg_out << std::endl;
			// get the timecode
			char msg[1024];
			memcpy(msg, &data[384], len - 384);
			msg[len - 384] = '\0';
			std::cout << "msg: " << msg << std::endl;
			// save the data about the robot pose (raw file)
            int size = 384;
			myFile.write(reinterpret_cast<const char*>(&data[0]), size);
			udp_komatsu_2arm.convert(reinterpret_cast<const char*>(&data[0]), 
				size);
			// show the image
			cv::resize(m, m, cv::Size(512, 512));
			cv::imshow("m", m);
			if (cv::waitKey(1) == 27) break;
		}
	}

}


/** @brief Test record
*/
void write() {
	//Open the default video camera
	cv::VideoCapture cap(0);

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		std::cout << "Cannot open the video camera" << std::endl;
		return;
	}

	int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)); //get the width of frames of the video
	int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)); //get the height of frames of the video

	cv::Mat tmp(frame_height, frame_width, CV_8UC3);
	//cv::Mat m_data_block(30, frame_width, CV_8UC3, cv::Scalar::all(0));
	cv::Mat m_data_block;
	size_t msg_len_max_bytes = 500;
	storedata::codify::CodifyImage::estimate_data_size(tmp, msg_len_max_bytes, 
		data_block_size, data_block_offset, m_data_block);

	cv::Size frame_size(frame_width, frame_height + m_data_block.rows);
	int frames_per_second = 10;

	//Create and initialize the VideoWriter object 
	//cv::VideoWriter oVideoWriter("MyVideo.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
	cv::VideoWriter oVideoWriter("MyVideo.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'),
		frames_per_second, frame_size, true);

	//If the VideoWriter object is not initialized successfully, exit the program
	if (oVideoWriter.isOpened() == false)
	{
		std::cout << "Cannot save the video to a file" << std::endl;
		return;
	}

	std::string window_name = "My Camera Feed";
	cv::namedWindow(window_name); //create a window called "My Camera Feed"

	int num_frame = 0;
	bool write_img = true;
	while (true)
	{
		cv::Mat frame;
		bool isSuccess = cap.read(frame); // read a new frame from the video camera

										  //Breaking the while loop if frames cannot be read from the camera
		if (isSuccess == false)
		{
			std::cout << "Video camera is disconnected" << std::endl;
			break;
		}

		/*
		Make changes to the frame as necessary
		e.g.
		1. Change brightness/contrast of the image
		2. Smooth/Blur image
		3. Crop the image
		4. Rotate the image
		5. Draw shapes on the image
		*/

		std::vector< std::string > vmsg, testmsg;
		vmsg.push_back("This is an example");
		vmsg.push_back("of how");
		vmsg.push_back("a data is codified in an image $%&#?!");
		vmsg.push_back("frame: " + std::to_string(num_frame++));

		// Codify the strings
		int x = data_block_offset, y = data_block_offset;
		storedata::codify::CodifyImage::merge_strings2image(vmsg, data_block_size, 
			data_block_offset, m_data_block, x, y);

		// combine the images
		cv::Mat m(frame_size, frame.type());
		frame.copyTo(m(cv::Rect(0, 0, frame.cols, frame.rows)));
		m_data_block.copyTo(m(cv::Rect(0, frame.rows, m_data_block.cols, m_data_block.rows)));

		//write the video frame to the file
		oVideoWriter.write(m);

		//show the frame in the created window
		cv::imshow(window_name, m);

		//Wait for for 10 milliseconds until any key is pressed.  
		//If the 'Esc' key is pressed, break the while loop.
		//If any other key is pressed, continue the loop 
		//If any key is not pressed within 10 milliseconds, continue the loop 
		if (cv::waitKey(10) == 27)
		{
			std::cout << "Esc key is pressed by the user. Stopping the video" << std::endl;
			break;
		}
		if (write_img) {
			write_img = false;
			cv::imwrite("RecordData_sample_codify.jpg", m);
		}

	}

	//Flush and close the video file
	oVideoWriter.release();
}


/** @brief Read recorded video
*/
void read() {
	cv::VideoCapture vc("MyVideo.avi");

	cv::Mat tmp(480, 640, CV_8UC3);
	//cv::Mat m_data_block(30, frame_width, CV_8UC3, cv::Scalar::all(0));
	cv::Mat m_data_block;
	size_t msg_len_max_bytes = 500;
	storedata::codify::CodifyImage::estimate_data_size(tmp, msg_len_max_bytes,
		data_block_size, data_block_offset, m_data_block);


	bool write_img = true;
	if (vc.isOpened()) {
		int num_frame = 0;
		while (true)
		{
			cv::Mat m;
			vc >> m;
			if (m.empty()) {
				break;
			}
			if (write_img) {
				write_img = false;
				cv::imwrite("m.png", m);
			}
			int x = data_block_offset, y = data_block_offset;
			std::string msg_out;
			storedata::codify::CodifyImage::image2string(m(cv::Rect(0, tmp.rows, tmp.cols,
				m.rows - tmp.rows)), x, y, data_block_size,
				data_block_offset, msg_out);
			std::cout << "msg_out: " << msg_out << std::endl;

			cv::imshow("m", m);
			if (cv::waitKey(1) == 27) break;
		}
	}
}


//-----------------------------------------------------------------------------
void main(int argc, char* argv[]) {

	std::cout << "This project shows how some text is codified inside a ";
	std::cout << "frame. All the size information are known for both writing ";
	std::cout << "and reading." << std::endl;
	// read a video saved in the impact field (warning the file may not exist)
	//read2(argc, argv);
	//return;
	write();
	read();
	return;
}