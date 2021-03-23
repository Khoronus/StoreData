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
 * @version 0.1.1.0
 * 
 */


#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

#define BOOST_BUILD
#include <boost/filesystem.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp> 

#include "record/record_headers.hpp"

// https://cheind.wordpress.com/2011/12/06/serialization-of-cvmat-objects-using-boost/
// https://www.patrikhuber.ch/blog/6-serialising-opencv-matrices-using-boost-and-cereal
// https://answers.opencv.org/question/6414/cvmat-serialization-to-binary-file/
BOOST_SERIALIZATION_SPLIT_FREE(::cv::Mat)
namespace boost {
namespace serialization {
 
/** Serialization support for cv::Mat */
template<class Archive>
void save(Archive & ar, const ::cv::Mat& m, const unsigned int version)
{
    size_t elem_size = m.elemSize();
    size_t elem_type = m.type();
 
    ar & m.cols;
    ar & m.rows;
    ar & elem_size;
    ar & elem_type;
 
    const size_t data_size = m.cols * m.rows * elem_size;
    ar & boost::serialization::make_array(m.ptr(), data_size);
}
 
/** Serialization support for cv::Mat */
template<class Archive>
void load(Archive & ar, ::cv::Mat& m, const unsigned int version)
{
    int cols, rows;
    size_t elem_size, elem_type;
 
    ar & cols;
    ar & rows;
    ar & elem_size;
    ar & elem_type;
 
    m.create(rows, cols, elem_type);
 
    size_t data_size = m.cols * m.rows * elem_size;
    ar & boost::serialization::make_array(m.ptr(), data_size);
}
 
}
}

 /** @brief Class object to serialize
 */
class ObjectToSerialize
{
public:

	/** @brief 'ctor
	*/
	ObjectToSerialize() {
	}

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		// invoke serialization of the base class 
		ar << img_;
		ar << msg_;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		// invoke serialization of the base class 
		ar >> img_;
		ar >> msg_;
		//if (version > 0)
		//	ar >> member3;

		//std::cout << "labels_instances_: " <<
		//	labels_instances_.size() << std::endl;
		//for (auto &it : labels_instances_) {
		//	std::cout << it.first << std::endl;
		//}
	}

	template<class Archive>
	void serialize(
		Archive & ar,
		const unsigned int file_version
	) {
		boost::serialization::split_member(ar, *this, file_version);
	}

	void set_img(const cv::Mat &img) {
		img_ = img;
	}
	cv::Mat& img() {
		return img_;
	}
	const cv::Mat& img() const {
		return img_;
	}
	void set_msg(const std::string &msg) {
		msg_ = msg;
	}
	std::string& msg() {
		return msg_;
	}
	const std::string& msg() const {
		return msg_;
	}

private:

	cv::Mat img_;
	std::string msg_;
};

/** @brief Container serialized
*/
class ContainerSerializedIO {
public:

	/** @brief Save the manager container
	*/
	static bool save_container(ObjectToSerialize &s, const std::string &filename) {
		// make an archive
		std::ofstream ofs(filename, std::ofstream::binary);
		if (ofs.is_open()) {
			boost::archive::binary_oarchive oa(ofs);
			oa << s;
			return true;
		}
		return false;
	}

	/** @brief Save the manager container
	*/
	static bool load_container(ObjectToSerialize &s, const std::string &filename)
	{
		// open the archive
		std::ifstream ifs(filename, std::ifstream::binary);
		if (ifs.is_open()) {
			boost::archive::binary_iarchive ia(ifs);
			// restore the schedule from the archive
			ia >> s;
			return true;
		}
		return false;
	}


    // https://stackoverflow.com/questions/3015582/direct-boost-serialization-to-char-array/5604782
	static void to_string(ObjectToSerialize &obj, std::string &serial_str) {
		// serialize obj into an std::string
		boost::iostreams::back_insert_device<std::string> inserter(serial_str);
		boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
		boost::archive::binary_oarchive oa(s);
		oa << obj;
		// don't forget to flush the stream to finish writing into the buffer
		s.flush();
		// now you get to const char* with serial_str.data() or serial_str.c_str()
	}

	static void from_string(std::string &serial_str, ObjectToSerialize &obj) {
		// wrap buffer inside a stream and deserialize serial_str into obj
		boost::iostreams::basic_array_source<char> device(serial_str.data(), serial_str.size());
		boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
		boost::archive::binary_iarchive ia(s);
		ia >> obj;
	}
};



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


/** @brief It records a video with a raw recorder
*/
int record_raw() {
	cv::VideoCapture vc(0);
	if (!vc.isOpened()) {
		std::cout << "Unable to open the camera" << std::endl;
		return 0;
	}

	storedata::RawRecorder pr;
	pr.set_callback_createfile(std::bind(&name_changed,
		std::placeholders::_1));
	pr.setup("data_recordraw\\record_", ".dat", 1000000000, 100);

	while (true) //Show the image captured in the window and repeat
	{
		cv::Mat curr;
		vc >> curr;
		if (curr.empty()) continue;

		//std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 3.03";
		std::string msg = "obj1 4.04 5.05 6.06 7.07|obj2 1.01 2.02 " + std::to_string((float)rand() / RAND_MAX);

		ObjectToSerialize obj;
		obj.set_img(curr);
		obj.set_msg(msg);

		std::string serial_str;
		ContainerSerializedIO::to_string(obj, serial_str);
		pr.record_t<const char*>(serial_str.data(), 
			serial_str.size());

		////////////////////////////////// Elaboration ////////////////////////////////////////
		cv::imshow("curr", curr);
		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}

// ----------------------------------------------------------------------------
void rawrecorder_read_raw(const std::string &filename) {

	std::vector< std::vector<uint8_t> > data_info;
	storedata::RawRecorder rr;
	rr.read(filename, data_info);

	std::cout << "Found: " << data_info.size() << " frames" << std::endl;

	int idx = 0;
//#if _MSC_VER && !__INTEL_COMPILER && (_MSC_VER > 1600)
//	for (auto it = data_info.begin(); it != data_info.end(); it++)
//#else
//	for (std::vector< std::vector<char> >::const_iterator it = data_info.begin(); it != data_info.end(); it++)
//#endif		
	for (auto &&it : data_info)
	{
		std::cout << idx << " => " << it.size() << std::endl;

		ObjectToSerialize obj;
		std::string serial_str(it.begin(), it.end());
		ContainerSerializedIO::from_string(serial_str, obj);
		std::cout << obj.msg() << std::endl;
		cv::imshow("img", obj.img());
		cv::waitKey(1);

		++idx;
	}
}


} // namespace anonymous

/**	 Main code
*/
int main(int argc, char *argv[])
{
	std::cout << "It records a serialized data (boost) into a file" << std::endl;
	// record a video with a raw data saver
	create_folder("data_recordraw");
	create_folder("unpack");

	record_raw();
	std::string fname = "data_recordraw\\record_" + global_fname + ".dat";
	std::cout << "[!] open: " << fname << std::endl;
	rawrecorder_read_raw(fname);
	return 0;

}
